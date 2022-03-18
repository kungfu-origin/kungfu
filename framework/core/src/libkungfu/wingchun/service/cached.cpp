#include <kungfu/common.h>
#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/service/cached.h>
#include <kungfu/yijinjing/time.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::cache;

namespace kungfu::wingchun::service {

CacheD::CacheD(locator_ptr locator, mode m, bool low_latency)
    : apprentice(location::make_shared(m, category::SYSTEM, "service", "cached", std::move(locator))) {}

void CacheD::on_react() {
  events_ | is(RequestCached::tag) | $([&](const event_ptr &event) {
    auto source_id = event->source();
    if (locations_.find(source_id) == locations_.end()) {
      SPDLOG_ERROR("no location {} in locations_", get_location_uname(source_id));
      return;
    }

    app_cache_shift_.try_emplace(source_id, locations_.at(source_id));
    auto cached_writer = get_writer(source_id);
    app_cache_shift_.at(source_id) >> cached_writer;
    mark_request_cached_done(source_id);
  });
}

void CacheD::mark_request_cached_done(uint32_t dest_id) {
  auto writer = get_writer(master_cmd_location_->uid);
  RequestCachedDone &rcd = writer->open_data<RequestCachedDone>();
  rcd.dest_id = dest_id;
  writer->close_data();
}

void CacheD::on_start() {
  events_ | is(Channel::tag) | $$(inspect_channel(event->gen_time(), event->data<Channel>()));
  events_ | is(Deregister::tag) | $$(deregister_cache_shift(event->data<Deregister>()));
  events_ | is(CacheReset::tag) | $$(on_cache_reset(event));
  events_ | instanceof <journal::frame>() | filter([&](const event_ptr &event) {
                         auto source_id = event->source();
                         return source_id != master_home_location_->location_uid and
                                source_id != master_cmd_location_->location_uid;
                       }) | $$(feed(event));
}

void CacheD::on_active() { handle_cached_feeds(); }

void CacheD::handle_cached_feeds() {
  bool stored_controller = false;
  boost::hana::for_each(StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    auto hana_type = boost::hana::type_c<DataType>;

    using FeedMap = std::unordered_map<uint64_t, state<DataType>>;
    auto &feed_map = const_cast<FeedMap &>(feed_bank_[hana_type]);

    if (feed_map.size() != 0) {
      auto iter = feed_map.begin();
      while (iter != feed_map.end() and !stored_controller) {
        auto s = iter->second;
        auto source_id = s.source;

        if (app_cache_shift_.find(source_id) != app_cache_shift_.end()) {
          try {
            app_cache_shift_.at(source_id) << s;
          } catch (const std::exception &e) {
            SPDLOG_ERROR("Unexpected exception by storage << {}", e.what());
            continue;
          }

          iter = feed_map.erase(iter);
          stored_controller = true;
        } else {
          iter++;
        }
      }
    }
  });
}

void CacheD::inspect_channel(int64_t trigger_time, const Channel &channel) {
  if (channel.source_id != get_live_home_uid() and channel.dest_id != get_live_home_uid()) {
    reader_->join(get_location(channel.source_id), channel.dest_id, trigger_time);
    register_cache_shift(channel);
  }
}

void CacheD::register_cache_shift(const Channel &channel) {
  uint32_t source_id = channel.source_id;
  uint32_t dest_id = channel.dest_id;

  if (locations_.find(source_id) == locations_.end()) {
    SPDLOG_ERROR("no source {} in locations_", get_location_uname(source_id));
    return;
  }

  if (not is_location_live(source_id)) {
    SPDLOG_ERROR("no source {} in registry_", get_location_uname(source_id));
    return;
  }

  const location_ptr &location = locations_.at(source_id);
  app_cache_shift_.emplace(source_id, location);
  ensure_cached_storage(source_id, dest_id);
  ensure_cached_storage(source_id, location::PUBLIC);
}

void CacheD::deregister_cache_shift(const Deregister &deregister_data) {
  uint32_t location_uid = deregister_data.location_uid;
  if (app_cache_shift_.find(location_uid) == app_cache_shift_.end()) {
    SPDLOG_ERROR("no location_uid {} in app_cache_shift_", get_location_uname(location_uid));
    return;
  }
  // maybe some data not cached done, so pass the deregister
  // app_cache_shift_.erase(location_uid);
}

void CacheD::on_cache_reset(const event_ptr &event) {
  auto msg_type = event->data<CacheReset>().msg_type;
  boost::hana::for_each(StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    if (DataType::tag == msg_type) {
      app_cache_shift_[event->source()] -= typed_event_ptr<DataType>(event);
      app_cache_shift_[event->dest()] /= typed_event_ptr<DataType>(event);
    }
  });
}

void CacheD::ensure_cached_storage(uint32_t source_id, uint32_t dest_id) {
  if (app_cache_shift_.find(source_id) == app_cache_shift_.end()) {
    SPDLOG_ERROR("no source {} in app_cache_shift_", get_location_uname(source_id));
    return;
  }

  app_cache_shift_.at(source_id).ensure_storage(dest_id);
}

void CacheD::feed(const event_ptr &event) {

  uint32_t source_id = event->source();
  uint32_t dest_id = event->dest();

  if (get_location(source_id)->category == category::MD) {
    return;
  }

  if (app_cache_shift_.find(source_id) == app_cache_shift_.end()) {
    SPDLOG_ERROR("no source {} in app_cache_shift_", get_location_uname(source_id));
    return;
  }

  boost::hana::for_each(StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    if (DataType::tag == event->msg_type()) {
      auto &data = event->data<DataType>();
      SPDLOG_INFO("===========");
      SPDLOG_INFO("soure {}, dest {}", get_location_uname(source_id), get_location_uname(dest_id));
      auto type_name = DataType::type_name.c_str();
      SPDLOG_INFO("{}, {}", type_name, data.to_string());
      SPDLOG_INFO("===========");
    }
  });

  feed_state_data(event, feed_bank_);
}

} // namespace kungfu::wingchun::service