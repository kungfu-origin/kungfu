
#include "bar.h"
#include <regex>
#include <kungfu/wingchun/utils.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::rx;

namespace kungfu
{
    namespace wingchun
    {
        namespace bar
        {
            static int64_t parse_time_interval(const std::string& s)
            {
                std::regex r("[0-9]+");
                std::smatch m;
                std::regex_search(s, m, r);
                if(m.empty())
                {
                    throw std::runtime_error("invalid time interval: " + s);
                }
                int n =  std::stoi(m[0]);
                if(endswith(s, "s"))
                {
                    return n * time_unit::NANOSECONDS_PER_SECOND;
                }
                else if(endswith(s, "m"))
                {
                    return n * time_unit::NANOSECONDS_PER_MINUTE;
                }
                else if(endswith(s, "h"))
                {
                    return n * time_unit::NANOSECONDS_PER_HOUR;
                }
                else if(endswith(s, "d"))
                {
                    return n * time_unit::NANOSECONDS_PER_DAY;
                }
                else
                {
                    throw std::runtime_error("invalid time_interval: " + s);
                }
            }

            BarGenerator::BarGenerator(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config):
                broker::MarketData(low_latency, std::move(locator), "bar"), time_interval_(kungfu::yijinjing::time_unit::NANOSECONDS_PER_MINUTE)
            {
                log::copy_log_settings(get_io_device()->get_home(), "bar");
                auto config = nlohmann::json::parse(json_config);
                std::string source = config["source"];
                auto home = get_io_device()->get_home();
                source_location_ = location::make(mode::LIVE, category::MD, source, source, home->locator);
                if (config.find("time_interval") != config.end())
                {
                    time_interval_ = parse_time_interval(config["time_interval"]);
                }
            }

            bool BarGenerator::subscribe(const std::vector<Instrument> &instruments)
            {
                if (not has_writer(source_location_->uid))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == source_location_->uid;
                           }) | first() |
                    $([=](event_ptr e)
                      {
                          for (const auto& inst: instruments)
                          {
                              write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_instrument_id());
                              auto now_in_nano = now();
                              auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                              SPDLOG_INFO("subscribe {}.{}", inst.instrument_id, inst.exchange_id);
                              request_subscribe(start_time, start_time + time_interval_, inst);
                          }
                      });
                } else
                {
                    for (const auto& inst: instruments)
                    {
                        write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_instrument_id());
                        auto now_in_nano = now();
                        auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                        SPDLOG_INFO("subscribe {}.{}", inst.instrument_id, inst.exchange_id);
                        request_subscribe(start_time, start_time + time_interval_, inst);
                    }
                }
                return true;
            }

            void BarGenerator::request_subscribe(int64_t start_time, int64_t end_time, const Instrument& instrument)
            {
                if (not is_live())
                {
                    return;
                }
                auto quotes = events_ | from(source_location_->uid) | is(msg::type::Quote) | filter([instrument](event_ptr event)
                        {
                            const auto& quote = event->data<Quote>();
                            bool res = strcmp(quote.instrument_id, instrument.instrument_id) == 0 && strcmp(quote.exchange_id, instrument.exchange_id) == 0;
                            return res;
                        }) | skip_while([start_time](event_ptr event)
                                {
                                    return event->data<Quote>().data_time < start_time;
                                }) | take_while([end_time](event_ptr event)
                                        {
                                            return event->data<Quote>().data_time < end_time;
                                        });

                auto bar = quotes.reduce(Bar(), [](Bar res, event_ptr event)
                {
                    const auto& quote = event->data<Quote>();
                    strcpy(res.instrument_id, quote.instrument_id);
                    strcpy(res.exchange_id, quote.exchange_id);
                    strcpy(res.trading_day, quote.trading_day);
                    if(res.tick_count == 0)
                    {
                        res.high = quote.last_price;
                        res.low = quote.last_price;
                        res.open = quote.last_price;
                        res.close = quote.last_price;
                        res.start_volume = quote.volume;
                    }
                    res.tick_count ++;
                    res.volume = quote.volume - res.start_volume;
                    res.high = std::max(res.high, quote.last_price);
                    res.low = std::min(res.low, quote.last_price);
                    res.close = quote.last_price;
                    return res;
                }).subscribe(
                        [&, start_time, end_time, instrument](Bar res)
                            {
                                res.start_time = start_time;
                                res.end_time = end_time;
                                SPDLOG_INFO("{}.{} [o:{},c:{},h:{},l:{}] from {} to {}",res.instrument_id, res.exchange_id,
                                        res.open, res.close, res.high, res.low, time::strftime(res.start_time), time::strftime(res.end_time));
                                this->get_writer(0)->write(0, msg::type::Bar, res);
                                request_subscribe(end_time, end_time + time_interval_, instrument);
                            });
            }

            void BarGenerator::on_start()
            {
                broker::MarketData::on_start();
                if (not has_location(source_location_->uid))
                {
                    throw wingchun_error(fmt::format("invalid md {}", source_location_->name));
                }
                request_read_from(now(), source_location_->uid, true);
                request_write_to(now(), source_location_->uid);
                SPDLOG_INFO("added md {} [{:08x}]", source_location_->name, source_location_->uid);
            }
        }
    }
}