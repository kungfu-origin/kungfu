#ifndef KUNGFU_CACHED_H
#define KUNGFU_CACHED_H

#include <kungfu/yijinjing/cache/runtime.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/yijinjing/practice/profile.h>

namespace kungfu::yijinjing::cache {

using ProfileDataTypesType = decltype(longfist::ProfileDataTypes);
using ProfileStateMapType = decltype(longfist::build_state_map(longfist::ProfileDataTypes));
typedef yijinjing::cache::typed_bank<ProfileDataTypesType, ProfileStateMapType> ProfileStateBank;

class cached : public yijinjing::practice::apprentice {
public:
  explicit cached(yijinjing::data::locator_ptr locator, longfist::enums::mode m, bool low_latency = false);

protected:
  void on_start() override;

  void on_react() override;

  void on_active() override;

  static constexpr auto profile_get_all = [](auto &profile, auto &receiver) {
    boost::hana::for_each(longfist::ProfileDataTypes, [&](auto it) {
      auto type = boost::hana::second(it);
      using DataType = typename decltype(+type)::type;
      int get_all_count = 0;
      while (get_all_count++ < 10) {
        try {
          for (const auto &data : profile.get_all(DataType{})) {
            auto s = state(0, 0, 0, data);
            receiver << s;
          }
          break;
        } catch (const std::exception &e) {
          SPDLOG_ERROR("Unexpected exception by profile_get_all {}", e.what());
        }
      }
    });
  };

private:
  std::unordered_map<uint32_t, yijinjing::cache::shift> app_cache_shift_ = {};
  yijinjing::cache::bank feed_bank_;
  yijinjing::practice::profile profile_;
  ProfileStateBank profile_bank_ = ProfileStateBank(longfist::ProfileDataTypes);

  void on_location(const event_ptr &event);

  void handle_cached_feeds();

  void handle_profile_feeds();

  void mark_request_cached_done(uint32_t dest_id);

  void inspect_channel(int64_t trigger_time, const longfist::types::Channel &channel);

  void channel_trigger_make_cache_shift(const longfist::types::Channel &channel);

  void register_triggger_clear_cache_shift(const longfist::types::Register &deregister_data);

  void on_cache_reset(const event_ptr &event);

  void ensure_cached_storage(uint32_t source_id, uint32_t dest_id);

  void feed(const event_ptr &event);
};

} // namespace kungfu::yijinjing::cache

#endif // KUNGFU_CACHED_H
