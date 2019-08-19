#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>

#include "bar.h"

using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace std::chrono;
using namespace kungfu::practice;

using namespace msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace bar
        {

            BarHandle::BarHandle(yijinjing::data::location_ptr home, const std::string &source, int frequency,
                                 bool low_latency) : apprentice(home, low_latency), source_(source),
                                                     frequency_(frequency)
            {
            }

            void BarHandle::snapshot_updata(const msg::data::Quote &quote, uint32_t source)
            {
                // nlohmann::json _quote;
                // msg::data::to_json(_quote, quote);
                // SPDLOG_INFO("[quote] {}", _quote.dump());

                if (bar_map.find(quote.get_instrument_id()) == bar_map.end())
                {
                    bar_map[quote.get_instrument_id()] = msg::data::Bar();
                    auto &bar = bar_map[quote.get_instrument_id()];
                    new_bar(bar, quote);
                    bar.start_time = quote.data_time;
                    bar.end_time = quote.data_time;
                    bar.next_time = quote.data_time + frequency_ * yijinjing::time_unit::NANOSECONDS_PER_MINUTE;
                    nlohmann::json j;
                    msg::data::to_json(j, bar);
                } else
                {
                    auto &bar = bar_map[quote.get_instrument_id()];
                    nlohmann::json j;
                    msg::data::to_json(j, bar);
                    while (quote.data_time > bar.next_time)
                    {
                        int64_t next_time = bar.next_time;
                        send_bar(bar);
                        new_bar(bar, quote);
                        bar.start_time = next_time;
                        bar.end_time = next_time + frequency_ * yijinjing::time_unit::NANOSECONDS_PER_MINUTE;
                        bar.next_time = next_time + frequency_ * yijinjing::time_unit::NANOSECONDS_PER_MINUTE;
                    }
                    if (bar.volume == 0)
                    {
                        bar.start_time = quote.data_time;
                    }
                    bar.end_time = quote.data_time;
                    bar.close = quote.last_price;
                    bar.low = std::min(bar.low, quote.last_price);
                    bar.high = std::max(bar.high, quote.last_price);
                    bar.volume = quote.volume - bar.start_volume;
                    bar.count += 1;
                }
            }

            void BarHandle::send_bar(const msg::data::Bar &bar)
            {
                nlohmann::json j;
                msg::data::to_json(j, bar);
                SPDLOG_INFO("[BAR] {}", j.dump());
                auto writer = writers_[0];
                int timep = kungfu::yijinjing::time::now_in_nano() / 1e9;
                msg::data::Bar &_bar = writer->open_data<msg::data::Bar>(kungfu::yijinjing::time::now_in_nano(),
                                                                         msg::type::Bar);
                _bar = bar;
                writer->close_data();
            }

            void inline BarHandle::new_bar(msg::data::Bar &bar, const msg::data::Quote &quote)
            {
                strncpy(bar.trading_day, quote.trading_day, DATE_LEN);
                strncpy(bar.instrument_id, quote.instrument_id, INSTRUMENT_ID_LEN);
                bar.pre_close_price = quote.pre_close_price;
                bar.open = quote.last_price;
                bar.close = quote.last_price;
                bar.low = quote.last_price;
                bar.high = quote.last_price;
                bar.volume = 0;
                bar.start_volume = quote.volume;
                bar.start_time = quote.data_time;
                bar.end_time = quote.data_time;
                bar.next_time = 0;
            }

            void BarHandle::react()
            {
                apprentice::react();
                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      snapshot_updata(event->data<msg::data::Quote>(), event->source());
                  });
            }

            void BarHandle::on_start()
            {
                SPDLOG_INFO("try to add md {} ", source_);
                auto md_location = location::make(mode::LIVE, category::MD, source_, source_,
                                                  get_io_device()->get_home()->locator);
                if (not has_location(md_location->uid))
                {
                    throw wingchun_error(fmt::format("invalid md {}", source_));
                }
                request_read_from(now(), md_location->uid, true);
                SPDLOG_INFO("added md {} [{:08x}]", source_, md_location->uid);
                // test();
            }

            void set_quote(msg::data::Quote &quote)
            {
                strncpy(quote.source_id, "1", SOURCE_ID_LEN);
                quote.data_time = 0;
                strncpy(quote.instrument_id, "rb1912", INSTRUMENT_ID_LEN);
                quote.pre_close_price = 10;
                quote.last_price = 11;
                quote.volume = 10000;
            }
        }
    }
}