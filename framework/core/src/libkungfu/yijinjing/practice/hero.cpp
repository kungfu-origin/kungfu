//
// Created by Keren Dong on 2019-06-15.
//

#include <kungfu/common.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/practice/hero.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/os.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::cache;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu::yijinjing::practice {

hero::hero(io_device_ptr io_device)
    : io_device_(std::move(io_device)), now_(0), begin_time_(time::now_in_nano()), end_time_(INT64_MAX)
       {

  os::handle_os_signals(this);
  add_location(0, get_io_device()->get_home());
  reader_ = io_device_->open_reader_to_subscribe();
}

hero::~hero() {
  writers_.clear();
  reader_.reset();
  io_device_.reset();
  ensure_sqlite_shutdown();
}

bool hero::is_usable() { return io_device_->is_usable(); }

void hero::setup() {
  io_device_->setup();
  events_ = observable<>::create<event_ptr>([this](auto &s) { delegate_produce(this, s); }) | holdon();
  react();
  live_ = true;
}

void hero::step() {
  continual_ = false;
  events_.connect(cs_);
}

void hero::run() {
  SPDLOG_INFO("[{:08x}] {} running", get_home_uid(), get_home_uname());
  SPDLOG_TRACE("from {} until {}", time::strftime(begin_time_), time::strftime(end_time_));
  setup();
  continual_ = true;
  events_.connect(cs_);
  on_exit();
  SPDLOG_INFO("[{:08x}] {} done", get_home_uid(), get_home_uname());
}

bool hero::is_live() const { return live_; }

void hero::signal_stop() { live_ = false; }

int64_t hero::now() const { return now_; }

void hero::set_begin_time(int64_t begin_time) { begin_time_ = begin_time; }

void hero::set_end_time(int64_t end_time) { end_time_ = end_time; }

const locator_ptr &hero::get_locator() const { return io_device_->get_locator(); }

io_device_ptr hero::get_io_device() const { return io_device_; }

const location_ptr &hero::get_home() const { return get_io_device()->get_home(); }

uint32_t hero::get_home_uid() const { return get_io_device()->get_home()->uid; }

const std::string &hero::get_home_uname() const { return get_io_device()->get_home()->uname; }

const location_ptr &hero::get_live_home() const { return get_io_device()->get_live_home(); }

uint32_t hero::get_live_home_uid() const { return get_io_device()->get_live_home()->uid; }

reader_ptr hero::get_reader() const { return reader_; }

bool hero::has_writer(uint32_t dest_id) const { return writers_.find(dest_id) != writers_.end(); }

writer_ptr hero::get_writer(uint32_t dest_id) const {
  if (writers_.find(dest_id) == writers_.end()) {
    SPDLOG_ERROR("no writer for {}", get_location_uname(dest_id));
  }
  return writers_.at(dest_id);
}

bool hero::has_location(uint32_t uid) const { return locations_.find(uid) != locations_.end(); }

location_ptr hero::get_location(uint32_t uid) const {
  if (not has_location(uid)) {
    SPDLOG_ERROR("no location {} in locations_", get_location_uname(uid));
  }

  assert(has_location(uid));
  return locations_.at(uid);
}

std::string hero::get_location_uname(uint32_t uid) const {
  if (uid == location::PUBLIC) {
    return "public";
  }
  if (not has_location(uid)) {
    return fmt::format("{:08x}", uid);
  }
  return get_location(uid)->uname;
}

bool hero::is_location_live(uint32_t uid) const { return registry_.find(uid) != registry_.end(); }

bool hero::has_channel(uint32_t source, uint32_t dest) const { return has_channel(make_chanel_hash(source, dest)); }

bool hero::has_channel(uint64_t hash) const { return channels_.find(hash) != channels_.end(); }

const Channel &hero::get_channel(uint64_t hash) const {
  assert(has_channel(hash));
  return channels_.at(hash);
}

const std::unordered_map<uint64_t, longfist::types::Channel> &hero::get_channels() const { return channels_; }

void hero::on_notify() {}

void hero::on_exit() {}

uint64_t hero::make_chanel_hash(uint32_t source_id, uint32_t dest_id) const {
  return uint64_t(source_id) << 32u | uint64_t(dest_id);
}

bool hero::check_location_exists(uint32_t source_id, uint32_t dest_id) const {
  if (not has_location(source_id)) {
    SPDLOG_ERROR("source_id {}, {} does not exist", source_id, get_location_uname(source_id));
    return false;
  }
  if (dest_id != 0 and not has_location(dest_id)) {
    SPDLOG_ERROR("dest_id {}, {} does not exist", dest_id, get_location_uname(dest_id));
    return false;
  }
  return true;
}

bool hero::check_location_live(uint32_t source_id, uint32_t dest_id) const {
  if (not check_location_exists(source_id, dest_id)) {
    return false;
  }
  if (registry_.find(source_id) == registry_.end()) {
    SPDLOG_ERROR("{} is not live", get_location_uname(source_id));
    return false;
  }
  if (registry_.find(dest_id) == registry_.end()) {
    SPDLOG_ERROR("{} is not live", get_location_uname(dest_id));
    return false;
  }
  return true;
}

void hero::add_location(int64_t trigger_time, const location_ptr &location) {
  locations_.try_emplace(location->uid, location);
}

void hero::add_location(int64_t trigger_time, const Location &location) {
  add_location(trigger_time, data::location::make_shared(location, get_locator()));
}

void hero::remove_location(int64_t trigger_time, uint32_t location_uid) { locations_.erase(location_uid); }

void hero::register_location(int64_t trigger_time, const Register &register_data) {
  uint32_t location_uid = register_data.location_uid;
  auto result = registry_.try_emplace(location_uid, register_data);
  if (result.second) {
    SPDLOG_TRACE("location [{:08x}] {} up", location_uid, get_location_uname(location_uid));
  }
}

void hero::deregister_location(int64_t trigger_time, const uint32_t location_uid) {
  auto result = registry_.erase(location_uid);
  if (result) {
    SPDLOG_TRACE("location [{:08x}] {} down", location_uid, get_location_uname(location_uid));
  }
}

void hero::register_channel(int64_t trigger_time, const Channel &channel) {
  uint64_t channel_uid = make_chanel_hash(channel.source_id, channel.dest_id);
  auto result = channels_.try_emplace(channel_uid, channel);
  if (result.second) {
    auto source_uname = get_location_uname(channel.source_id);
    auto dest_uname = get_location_uname(channel.dest_id);
    SPDLOG_TRACE("channel [{:08x}] {} -> {} up", channel_uid, source_uname, dest_uname);
  }
}

void hero::deregister_channel(uint32_t source_location_uid) {
  auto channel_it = channels_.begin();
  while (channel_it != channels_.end()) {
    if (channel_it->second.source_id == source_location_uid) {
      const auto &channel_uid = channel_it->first;
      const auto &channel = channel_it->second;
      auto source_uname = get_location_uname(channel.source_id);
      auto dest_uname = get_location_uname(channel.dest_id);
      SPDLOG_TRACE("channel [{:08x}] {} -> {} down", channel_uid, source_uname, dest_uname);
      channel_it = channels_.erase(channel_it);
      continue;
    }
    channel_it++;
  }
}

void hero::require_read_from(int64_t trigger_time, uint32_t dest_id, uint32_t source_id, int64_t from_time) {
  do_require_read_from<RequestReadFrom>(get_writer(dest_id), trigger_time, dest_id, source_id, from_time);
}

void hero::require_read_from_public(int64_t trigger_time, uint32_t dest_id, uint32_t source_id, int64_t from_time) {
  do_require_read_from<RequestReadFromPublic>(get_writer(dest_id), trigger_time, dest_id, source_id, from_time);
}

void hero::require_write_to(int64_t trigger_time, uint32_t source_id, uint32_t dest_id) {
  if (not check_location_exists(source_id, dest_id)) {
    return;
  }
  auto writer = get_writer(source_id);
  RequestWriteTo &msg = writer->open_data<RequestWriteTo>(trigger_time);
  msg.dest_id = dest_id;
  writer->close_data();
}

void hero::produce(const rx::subscriber<event_ptr> &sb) {
  try {
    do {
      live_ = drain(sb) && live_;
      on_active();
    } while (continual_ and live_);
  } catch (...) {
    live_ = false;
    sb.on_error(std::current_exception());
  }
  if (not live_) {
    sb.on_completed();
  }
}

bool hero::drain(const rx::subscriber<event_ptr> &sb) {
  if (io_device_->get_home()->mode == mode::LIVE and io_device_->get_observer()->wait()) {
    const std::string &notice = io_device_->get_observer()->get_notice();
    now_ = time::now_in_nano();
    if (notice.length() > 2) {
      sb.on_next(std::make_shared<nanomsg_json>(notice));
    } else {
      on_notify();
    }
  }
  while (live_ and reader_->data_available()) {
    if (reader_->current_frame()->gen_time() <= end_time_) {
      int64_t frame_time = reader_->current_frame()->gen_time();
      if (frame_time > now_) {
        now_ = frame_time;
      }
      sb.on_next(reader_->current_frame());
      reader_->next();
    } else {
      SPDLOG_INFO("reached journal end {}", time::strftime(reader_->current_frame()->gen_time()));
      return false;
    }
  }
  if (get_io_device()->get_home()->mode != mode::LIVE and not reader_->data_available()) {
    SPDLOG_INFO("reached journal end {}", time::strftime(reader_->current_frame()->gen_time()));
    return false;
  }
  return true;
}

void hero::delegate_produce(hero *instance, const rx::subscriber<event_ptr> &subscriber) {
#ifdef _WINDOWS
  __try {
    instance->produce(subscriber);
  } __except (util::print_stack_trace(GetExceptionInformation())) {
  }
#else
  instance->produce(subscriber);
#endif
}
} // namespace kungfu::yijinjing::practice
