//
// Created by Keren Dong on 2019-06-15.
//

#include <kungfu/common.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/practice/hero.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/os.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu::yijinjing::practice {

hero::hero(yijinjing::io_device_with_reply_ptr io_device)
    : io_device_(std::move(io_device)), now_(0), real_now_(0), begin_time_(time::now_in_nano()), end_time_(INT64_MAX) {
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

  events_ =
      observable<>::create<event_ptr>([&, this](subscriber<event_ptr> sb) { delegate_produce(this, sb); }) | holdon();

  events_ | on_error_resume_next([&](std::exception_ptr e) -> observable<event_ptr> {
    SPDLOG_ERROR("on error resume next");
    try {
      std::rethrow_exception(e);
    } catch (const nn_exception &ex) {
      switch (ex.num()) {
      case EINTR:
      case EAGAIN:
      case ETIMEDOUT: {
        SPDLOG_INFO("kungfu app quit because {}", ex.what());
        return observable<>::empty<event_ptr>();
      }
      default: {
        SPDLOG_ERROR("Unexpected nanomsg error: {}", ex.what());
        return observable<>::error<event_ptr>(ex);
      }
      }
    } catch (const std::exception &ex) {
      SPDLOG_ERROR("Unexpected exception: {}", ex.what());
      return observable<>::error<event_ptr>(ex);
    }
  });

  react();
  live_ = true;
}

void hero::step() {
  continual_ = false;
  events_.connect(cs_);
}

void hero::run() {
  SPDLOG_INFO("{} [{:08x}] running", get_home_uname(), get_home_uid());
  SPDLOG_INFO("from {} until {}", time::strftime(begin_time_),
              end_time_ == INT64_MAX ? "end of world" : time::strftime(end_time_));
  setup();
  continual_ = true;
  events_.connect(cs_);
  on_exit();
  SPDLOG_INFO("{} [{:08x}] finished", get_home_uname(), get_home_uid());
}

bool hero::is_live() const { return live_; }

void hero::signal_stop() { live_ = false; }

int64_t hero::now() const { return now_; }

int64_t hero::real_now() const { return real_now_; }

void hero::set_begin_time(int64_t begin_time) { begin_time_ = begin_time; }

void hero::set_end_time(int64_t end_time) { end_time_ = end_time; }

const locator_ptr &hero::get_locator() const { return io_device_->get_locator(); }

io_device_with_reply_ptr hero::get_io_device() const { return io_device_; }

const location_ptr &hero::get_home() const { return get_io_device()->get_home(); }

uint32_t hero::get_home_uid() const { return get_io_device()->get_home()->uid; }

const std::string &hero::get_home_uname() const { return get_io_device()->get_home()->uname; }

const location_ptr &hero::get_live_home() const { return get_io_device()->get_live_home(); }

uint32_t hero::get_live_home_uid() const { return get_io_device()->get_live_home()->uid; }

reader_ptr hero::get_reader() const { return reader_; }

bool hero::has_writer(uint32_t dest_id) const { return writers_.find(dest_id) != writers_.end(); }

writer_ptr hero::get_writer(uint32_t dest_id) const {
  if (writers_.find(dest_id) == writers_.end()) {
    SPDLOG_ERROR("no writer for {}",
                 has_location(dest_id) ? get_location(dest_id)->uname : fmt::format("{:08x}", dest_id));
  }
  return writers_.at(dest_id);
}

bool hero::has_location(uint32_t uid) const { return locations_.find(uid) != locations_.end(); }

location_ptr hero::get_location(uint32_t uid) const {
  assert(has_location(uid));
  return locations_.at(uid);
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
    SPDLOG_ERROR("[{:08x}] -> [{:08x}] source does not exist", source_id, dest_id);
    return false;
  }
  if (dest_id != 0 and not has_location(dest_id)) {
    SPDLOG_ERROR("[{:08x}] -> [{:08x}] dest does not exist", source_id, dest_id);
    return false;
  }
  return true;
}

bool hero::check_location_live(uint32_t source_id, uint32_t dest_id) const {
  if (not check_location_exists(source_id, dest_id)) {
    return false;
  }
  if (registry_.find(source_id) == registry_.end()) {
    SPDLOG_ERROR("[{:08x}] -> [{:08x}] source is not live", source_id, dest_id);
    return false;
  }
  if (registry_.find(dest_id) == registry_.end()) {
    SPDLOG_ERROR("[{:08x}] -> [{:08x}] dest is not live", source_id, dest_id);
    return false;
  }
  return true;
}

void hero::add_location(int64_t trigger_time, const location_ptr &location) {
  if (not has_location(location->uid)) {
    locations_.emplace(location->uid, location);
    SPDLOG_INFO("added location {} [{:08x}]", location->uname, location->uid);
  }
}

void hero::remove_location(int64_t trigger_time, uint32_t location_uid) {
  if (has_location(location_uid)) {
    SPDLOG_INFO("removed location {} [{:08x}]", locations_.at(location_uid)->uname, location_uid);
    locations_.erase(location_uid);
  }
}

void hero::register_location(int64_t trigger_time, const longfist::types::Register &register_data) {
  if (not is_location_live(register_data.location_uid)) {
    registry_.emplace(register_data.location_uid, register_data);
    SPDLOG_INFO("registered location {} [{:08x}]", register_data.to_string(), register_data.location_uid);
  }
}

void hero::deregister_location(int64_t trigger_time, const uint32_t location_uid) {
  registry_.erase(location_uid);
  SPDLOG_INFO("deregister-ed location {} [{:08x}]", locations_.at(location_uid)->uname, location_uid);
}

void hero::register_channel(int64_t trigger_time, const Channel &channel) {
  uint64_t channel_uid = make_chanel_hash(channel.source_id, channel.dest_id);
  channels_[channel_uid] = channel;
  SPDLOG_INFO("registered channel [{:08x}] from {} [{:08x}] to {} [{:08x}] ", channel_uid,
              has_location(channel.source_id) ? get_location(channel.source_id)->uname : "unknown", channel.source_id,
              has_location(channel.dest_id) ? get_location(channel.dest_id)->uname : "unknown", channel.dest_id);
}

void hero::deregister_channel(uint32_t source_location_uid) {
  auto channel_it = channels_.begin();
  while (channel_it != channels_.end()) {
    if (channel_it->second.source_id == source_location_uid) {
      const auto &channel_uid = channel_it->first;
      const auto &channel = channel_it->second;
      SPDLOG_INFO("deregister-ed channel [{:08x}] from {} [{:08x}] to {} [{:08x}]", channel_uid,
                  has_location(channel.source_id) ? get_location(channel.source_id)->uname : "unknown",
                  channel.source_id, has_location(channel.dest_id) ? get_location(channel.dest_id)->uname : "unknown",
                  channel.dest_id);
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
  SPDLOG_INFO("require {} [{:08x}] write to {} [{:08x}]", get_location(source_id)->uname, source_id,
              dest_id == 0 ? "public" : get_location(dest_id)->uname, dest_id);
}

void hero::produce(const rx::subscriber<event_ptr> &sb) {
  try {
    do {
      on_active();
      live_ = drain(sb) && live_;
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
  if (io_device_->get_home()->mode == mode::LIVE) {
    if (io_device_->get_observer()->wait()) {
      const std::string &notice = io_device_->get_observer()->get_notice();
      now_ = time::now_in_nano();
      real_now_ = time::now_in_nano();
      if (notice.length() > 2) {
        sb.on_next(std::make_shared<nanomsg_json>(notice));
      } else {
        on_notify();
      }
    }
    if (io_device_->get_rep_sock()->recv() > 0) {
      const std::string &msg = io_device_->get_rep_sock()->last_message();
      now_ = time::now_in_nano();
      real_now_ = time::now_in_nano();
      sb.on_next(std::make_shared<nanomsg_json>(msg));
    }
  }
  while (live_ and reader_->data_available()) {
    if (reader_->current_frame()->gen_time() <= end_time_) {
      int64_t frame_time = reader_->current_frame()->gen_time();
      if (frame_time > now_) {
        now_ = frame_time;
      }
      real_now_ = time::now_in_nano();
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

void hero::delegate_produce(hero *instance, const rx::subscriber<event_ptr> &sb) {
#ifdef _WINDOWS
  __try {
    instance->produce(sb);
  } __except (util::print_stack_trace(GetExceptionInformation())) {
  }
#else
  instance->produce(sb);
#endif
}
} // namespace kungfu::yijinjing::practice
