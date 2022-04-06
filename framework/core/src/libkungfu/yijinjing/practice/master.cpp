//
// Created by Keren Dong on 2019-06-15.
//

#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/master.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::cache;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;

namespace kungfu::yijinjing::practice {

master::master(location_ptr home, bool low_latency)
    : hero(std::make_shared<io_device_master>(home, low_latency)), start_time_(time::now_in_nano()), last_check_(0),
      session_builder_(get_io_device()), profile_(get_locator()) {
  profile_.setup();
  for (const auto &app_location : profile_.get_all(Location{})) {
    add_location(start_time_, location::make_shared(app_location, get_locator()));
  }
  for (const auto &config : profile_.get_all(Config{})) {
    try_add_location(start_time_, location::make_shared(config, get_locator()));
  }

  auto io_device = std::dynamic_pointer_cast<io_device_master>(get_io_device());
  writers_.emplace(location::PUBLIC, io_device->open_writer(0));
  get_writer(location::PUBLIC)->mark(start_time_, SessionStart::tag);
}

index::session_builder &master::get_session_builder() { return session_builder_; }

void master::on_exit() {
  auto io_device = std::dynamic_pointer_cast<io_device_master>(get_io_device());
  auto now = time::now_in_nano();
  get_writer(location::PUBLIC)->mark(now, SessionEnd::tag);
}

void master::on_notify() { get_io_device()->get_publisher()->notify(); }

void master::register_app(const event_ptr &event) {
  auto io_device = std::dynamic_pointer_cast<io_device_master>(get_io_device());
  auto home = io_device->get_home();

  auto request_json = event->data<nlohmann::json>();
  auto request_data = event->data_as_string();

  Register register_data(request_data.c_str(), request_data.length());

  auto app_location = location::make_shared(register_data, home->locator);

  if (is_location_live(app_location->uid)) {
    SPDLOG_ERROR("location {} has already been registered live", app_location->uname);
    return;
  }

  auto now = time::now_in_nano();
  auto uid_str = fmt::format("{:08x}", app_location->uid);
  auto master_cmd_location = location::make_shared(mode::LIVE, category::SYSTEM, "master", uid_str, home->locator);
  auto public_writer = get_writer(location::PUBLIC);
  auto app_cmd_writer = get_io_device()->open_writer_at(master_cmd_location, app_location->uid);

  try_add_location(event->gen_time(), app_location);
  try_add_location(event->gen_time(), master_cmd_location);

  register_data.last_active_time = session_builder_.find_last_active_time(app_location);
  register_location(event->gen_time(), register_data);

  writers_.emplace(app_location->uid, app_cmd_writer);
  reader_->join(app_location, location::PUBLIC, now);
  reader_->join(app_location, master_cmd_location->uid, now);

  session_builder_.open_session(app_location, event->gen_time());

  public_writer->write(event->gen_time(), *std::dynamic_pointer_cast<Location>(app_location));
  public_writer->write(event->gen_time(), register_data);

  require_write_to(event->gen_time(), app_location->uid, location::PUBLIC);
  require_write_to(event->gen_time(), app_location->uid, master_cmd_location->uid);

  app_cmd_writer->mark(event->gen_time(), SessionStart::tag);

  write_time_reset(event->gen_time(), app_cmd_writer);
  write_trading_day(event->gen_time(), app_cmd_writer);

  // tell others alive locations
  write_locations(event->gen_time(), app_cmd_writer);
  // tell the registing app the their self and cached process started
  write_registries(event->gen_time(), app_cmd_writer);

  on_register(event, register_data);
}

void master::deregister_app(int64_t trigger_time, uint32_t app_location_uid) {
  auto location = get_location(app_location_uid);
  SPDLOG_INFO("app {} gone", location->uname);
  session_builder_.close_session(location, trigger_time);
  get_writer(app_location_uid)->mark(trigger_time, SessionEnd::tag);
  deregister_channel(app_location_uid);
  deregister_location(trigger_time, app_location_uid);
  registry_.erase(app_location_uid);
  reader_->disjoin(app_location_uid);
  writers_.erase(app_location_uid);
  timer_tasks_.erase(app_location_uid);
  get_writer(location::PUBLIC)->write(trigger_time, location->to<Deregister>());
}

void master::publish_trading_day() { write_trading_day(0, get_writer(location::PUBLIC)); }

void master::react() {
  events_ | is(RequestWriteTo::tag) | $$(on_request_write_to(event));
  events_ | is(RequestReadFrom::tag) | $$(on_request_read_from(event));
  events_ | is(RequestReadFrom::tag) | $$(check_cached_ready_to_read(event));
  events_ | is(RequestReadFromPublic::tag) | $$(on_request_read_from_public(event));
  events_ | is(Channel::tag) | $$(on_channel_request(event));
  events_ | is(TimeRequest::tag) | $$(on_time_request(event));
  events_ | is(Location::tag) | $$(on_new_location(event));
  events_ | is(Register::tag) | $$(register_app(event));
  events_ | is(RequestCachedDone::tag) | $$(on_request_cached_done(event));
  events_ | is(Ping::tag) | $$(pong(event));
  events_ | instanceof <journal::frame>() | $$(feed(event));
}

void master::on_active() {
  auto now = time::now_in_nano();
  if (last_check_ + time_unit::NANOSECONDS_PER_SECOND < now) {
    on_interval_check(now);
    last_check_ = now;
  }
  handle_timer_tasks();
}

void master::handle_timer_tasks() {
  auto now = time::now_in_nano();
  for (auto &app : timer_tasks_) {
    uint32_t app_id = app.first;
    auto &app_tasks = app.second;
    for (auto it = app_tasks.begin(); it != app_tasks.end();) {
      auto &task = it->second;
      if (task.checkpoint <= now) {
        get_writer(app_id)->mark(0, Time::tag);
        task.checkpoint += task.duration;
        task.repeat_count++;
        if (task.repeat_count >= task.repeat_limit) {
          it = app_tasks.erase(it);
          continue;
        }
      }
      it++;
    }
  }
}

void master::try_add_location(int64_t trigger_time, const location_ptr &app_location) {
  if (not has_location(app_location->uid)) {
    add_location(trigger_time, app_location);
  }
}

void master::feed(const event_ptr &event) {
  handle_timer_tasks();

  if (registry_.find(event->source()) == registry_.end()) {
    return;
  }

  session_builder_.update_session(std::dynamic_pointer_cast<journal::frame>(event));
}

void master::pong(const event_ptr &event) { get_io_device()->get_publisher()->publish("{}"); }

void master::on_request_write_to(const event_ptr &event) {
  const RequestWriteTo &request = event->data<RequestWriteTo>();
  auto trigger_time = event->gen_time();
  auto app_uid = event->source();
  if (not is_location_live(app_uid)) {
    return;
  }
  reader_->join(get_location(app_uid), request.dest_id, trigger_time);
  require_write_to(trigger_time, app_uid, request.dest_id);
  if (is_location_live(request.dest_id) and has_writer(request.dest_id)) {
    require_read_from(0, request.dest_id, app_uid, trigger_time);
  }
  Channel channel = {};
  channel.source_id = app_uid;
  channel.dest_id = request.dest_id;
  register_channel(trigger_time, channel);
  get_writer(location::PUBLIC)->write(trigger_time, channel);
}

void master::on_request_read_from(const event_ptr &event) {
  const RequestReadFrom &request = event->data<RequestReadFrom>();
  auto trigger_time = event->gen_time();
  auto app_uid = event->source();
  if (not check_location_live(request.source_id, app_uid)) {
    return;
  }
  reader_->join(get_location(request.source_id), app_uid, trigger_time);
  require_write_to(trigger_time, request.source_id, app_uid);
  require_read_from(trigger_time, app_uid, request.source_id, request.from_time);
  Channel channel = {};
  channel.source_id = request.source_id;
  channel.dest_id = app_uid;
  register_channel(trigger_time, channel);
  get_writer(location::PUBLIC)->write(trigger_time, channel);
}

void master::on_request_read_from_public(const event_ptr &event) {
  const RequestReadFromPublic &request = event->data<RequestReadFromPublic>();
  require_read_from_public(event->gen_time(), event->source(), request.source_id, request.from_time);
}

void master::check_cached_ready_to_read(const event_ptr &event) {
  const RequestReadFrom &request = event->data<RequestReadFrom>();
  auto trigger_time = event->gen_time();
  auto app_uid = event->source();
  auto read_from_source_id = request.source_id;

  if (read_from_source_id == cached_home_location_->uid) {
    auto app_cmd_writer = get_writer(app_uid);
    app_cmd_writer->mark(now(), CachedReadyToRead::tag);
  }
}

void master::on_request_cached_done(const event_ptr &event) {
  auto request_cached_done_data = event->data<RequestCachedDone>();
  auto app_uid = request_cached_done_data.dest_id;
  auto app_cmd_writer = get_writer(app_uid);
  app_cmd_writer->mark(now(), RequestStart::tag);
  write_registries(event->gen_time(), app_cmd_writer);
  write_channels(event->gen_time(), app_cmd_writer);
}

void master::on_channel_request(const event_ptr &event) {
  const Channel &channel = event->data<Channel>();
  auto trigger_time = event->gen_time();
  if (is_location_live(channel.source_id) and not has_channel(channel.source_id, channel.dest_id)) {
    reader_->join(get_location(channel.source_id), channel.dest_id, trigger_time);
    require_write_to(trigger_time, channel.source_id, channel.dest_id);
    register_channel(trigger_time, channel);
    get_writer(location::PUBLIC)->write(trigger_time, channel);
  }
}

void master::on_time_request(const event_ptr &event) {
  const TimeRequest &request = event->data<TimeRequest>();
  timer_tasks_.try_emplace(event->source());
  auto &app_tasks = timer_tasks_.at(event->source());
  app_tasks.try_emplace(request.id);
  auto &task = app_tasks.at(request.id);
  task.checkpoint = time::now_in_nano() + request.duration;
  task.duration = request.duration;
  task.repeat_count = 0;
  task.repeat_limit = request.repeat;
}

void master::on_new_location(const event_ptr &event) {
  const Location &location = event->data<Location>();
  try_add_location(event->gen_time(), data::location::make_shared(location, get_locator()));
  get_writer(location::PUBLIC)->write(event->gen_time(), location);
}

void master::write_time_reset(int64_t trigger_time, const writer_ptr &writer) {
  auto time_base = time::get_base();
  TimeReset &time_reset = writer->open_data<TimeReset>();
  time_reset.system_clock_count = time_base.system_clock_count;
  time_reset.steady_clock_count = time_base.steady_clock_count;
  writer->close_data();
}

void master::write_trading_day(int64_t trigger_time, const writer_ptr &writer) {
  TradingDay &trading_day = writer->open_data<TradingDay>();
  trading_day.timestamp = acquire_trading_day();
  writer->close_data();
}

void master::write_registries(int64_t trigger_time, const writer_ptr &writer) {
  for (const auto &item : registry_) {
    writer->write(trigger_time, item.second);
  }
}

void master::write_locations(int64_t trigger_time, const writer_ptr &writer) {
  for (const auto &item : locations_) {
    writer->write(trigger_time, dynamic_cast<Location &>(*item.second));
  }
}

void master::write_channels(int64_t trigger_time, const writer_ptr &writer) {
  for (const auto &item : channels_) {
    writer->write(trigger_time, item.second);
  }
}

} // namespace kungfu::yijinjing::practice
