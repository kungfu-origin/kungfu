#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>

#include "bar_handle.h"

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

        BarHandle::BarHandle(yijinjing::data::location_ptr home, const std::string &source, int frequency, bool low_latency): apprentice(home, low_latency), _source(source),_frequency(frequency)
        {
        }

        void BarHandle::snapshot_updata(const msg::data::Quote &quote,int source)
        {
            // nlohmann::json _quote;
            // msg::data::to_json(_quote, quote);
            // SPDLOG_INFO("[quote] {}", _quote.dump());
            if (bar_map.find(quote.instrument_id) == bar_map.end())
            {
                auto& bar = bar_map[quote.instrument_id];
                new_bar(bar, quote);
                bar.start_time = quote.data_time;
                bar.end_time = quote.data_time;
                bar.next_time = quote.data_time + _frequency*yijinjing::time_unit::NANOSECONDS_PER_MINUTE;
            }
            else
            {
                auto& bar = bar_map[quote.instrument_id];
                while (quote.data_time > bar.next_time)
                {
                    int64_t next_time = bar.next_time;
                    send_bar(bar);
                    new_bar(bar, quote);
                    bar.start_time = next_time;
                    bar.end_time = next_time + _frequency*yijinjing::time_unit::NANOSECONDS_PER_MINUTE;
                    bar.next_time = next_time + _frequency*yijinjing::time_unit::NANOSECONDS_PER_MINUTE;
                }
                if (bar.Volume  == 0)
                {
                    bar.start_time = quote.data_time;
                }
                bar.end_time = quote.data_time;
                bar.Close = quote.last_price;
                bar.Low = std::min(bar.Low, quote.last_price);
                bar.High = std::max(bar.Low, quote.last_price);
                bar.Volume = quote.volume - bar.StartVolume;
            }
        }

        void BarHandle::send_bar(const msg::data::Bar &bar)
        {
            nlohmann::json j;
            msg::data::to_json(j, bar);
            SPDLOG_INFO("[BAR] {}",j.dump());
            auto writer = writers_[0];
            int timep = kungfu::yijinjing::time::now_in_nano()/1e9;
            msg::data::Bar &_bar = writer->open_data<msg::data::Bar>(kungfu::yijinjing::time::now_in_nano(), msg::type::Bar);
            _bar = bar;
            writer->close_data();
        }

        void inline BarHandle::new_bar(msg::data::Bar &bar, const msg::data::Quote &quote)
        {
            strncpy(bar.trading_day, quote.trading_day, DATE_LEN);
            strncpy(bar.trading_day, quote.trading_day, INSTRUMENT_ID_LEN);
            bar.PreClosePrice = quote.pre_close_price;
            bar.Open = quote.last_price;
            bar.Close = quote.last_price;
            bar.Low = quote.last_price;
            bar.High = quote.last_price;
            bar.Volume = 0;
            bar.StartVolume = quote.volume;
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
            SPDLOG_INFO("try to add md {} ", _source);
            auto md_location = location::make(mode::LIVE, category::MD, _source, _source, get_io_device()->get_home()->locator);
            if (not has_location(md_location->uid))
            {
                throw wingchun_error(fmt::format("invalid md {}", _source));
            }
            request_read_from(now(), md_location->uid, true);
            SPDLOG_INFO("added md {} [{:08x}]", _source, md_location->uid);
            // test();
        }

        void set_quote(kungfu::wingchun::msg::data::Quote &quote)
        {
            strncpy(quote.source_id , "1", SOURCE_ID_LEN);
            quote.data_time = 0;
            strncpy(quote.instrument_id, "rb1912", INSTRUMENT_ID_LEN);
            quote.pre_close_price = 10;
            quote.last_price = 11;
            quote.volume = 10000;
        }


        void BarHandle::test()
        {
            SPDLOG_WARN("test start");
            auto home = get_io_device()->get_home();
            auto location = location::make(home->mode, category::MD, _source, _source, home->locator);
            request_write_to(time::now_in_nano(), location->uid);
            request_read_from(time::now_in_nano(), location->uid, true);
            int64_t data_time = 0;
            int volume = 0;
            for (int i = 0; i < 1000; i++)
            {
                data_time += 3*kungfu::yijinjing::time_unit::NANOSECONDS_PER_SECOND;
                volume += 1*100;
                auto writer = get_writer(0);
                kungfu::wingchun::msg::data::Quote &quote = writer->open_data<kungfu::wingchun::msg::data::Quote>(time::now_in_nano(), msg::type::Quote);
                quote.data_time = data_time;
                quote.volume = volume;
                writer->close_data();
            }
            SPDLOG_WARN("test finish");
        }
    }
}