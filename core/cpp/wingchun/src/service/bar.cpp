
#include <kungfu/wingchun/service/bar.h>
#include <regex>
#include <kungfu/wingchun/utils.h>
#include <kungfu/yijinjing/log/setup.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::rx;

namespace kungfu
{
    namespace wingchun
    {
        namespace service
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

            BarGenerator::BarGenerator(locator_ptr locator, mode m, bool low_latency, const std::string &json_config):
                practice::apprentice(location::make(m, category::SYSTEM, "service", "bar", std::move(locator)), low_latency),
                time_interval_(kungfu::yijinjing::time_unit::NANOSECONDS_PER_MINUTE)
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
                              auto symbol_id = get_symbol_id(inst.get_instrument_id(), inst.get_exchange_id());
                              if(bars_.find(symbol_id) ==  bars_.end())
                              {
                                  write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_instrument_id());
                                  auto now_in_nano = now();
                                  auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                                  SPDLOG_INFO("subscribe {}.{}", inst.instrument_id, inst.exchange_id);
                                  Bar bar = {};
                                  strncpy(bar.instrument_id, inst.instrument_id, INSTRUMENT_ID_LEN);
                                  strncpy(bar.exchange_id, inst.exchange_id, EXCHANGE_ID_LEN);
                                  bar.start_time = start_time;
                                  bar.end_time = start_time + time_interval_;
                                  bars_[symbol_id] = bar;
                              }
                          }
                      });
                } else
                {
                    for (const auto& inst: instruments)
                    {
                        auto symbol_id = get_symbol_id(inst.get_instrument_id(), inst.get_exchange_id());
                        if(bars_.find(symbol_id) ==  bars_.end())
                        {
                            write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_instrument_id());
                            auto now_in_nano = now();
                            auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                            SPDLOG_INFO("subscribe {}.{}", inst.instrument_id, inst.exchange_id);
                            Bar bar = {};
                            strncpy(bar.instrument_id, inst.instrument_id, INSTRUMENT_ID_LEN);
                            strncpy(bar.exchange_id, inst.exchange_id, EXCHANGE_ID_LEN);
                            bar.start_time = start_time;
                            bar.end_time = start_time + time_interval_;
                            bars_[symbol_id] = bar;
                        }
                    }
                }
                return true;
            }

            void BarGenerator::register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location)
            {
                if (has_location(location->uid))
                {
                    return;
                }
                apprentice::register_location(trigger_time, location);
                if (location->uid == source_location_->uid)
                {
                    request_read_from(now(), source_location_->uid, true);
                    request_write_to(now(), source_location_->uid);
                    SPDLOG_INFO("added md {} [{:08x}]", source_location_->uname, source_location_->uid);
                }
            }

            void BarGenerator::on_start()
            {
                apprentice::on_start();

                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                {
                    const auto& quote = event->data<Quote>();
                    auto symbol_id = get_symbol_id(quote.get_instrument_id(), quote.get_exchange_id());
                    if (bars_.find(symbol_id) != bars_.end())
                    {
                        SPDLOG_TRACE("{}.{} at {} vol {} price {}", quote.instrument_id, quote.exchange_id, time::strftime(quote.data_time), quote.volume, quote.last_price);
                        auto& bar = bars_[symbol_id];
                        if (quote.data_time >= bar.start_time && quote.data_time <= bar.end_time)
                        {
                            if(bar.tick_count == 0)
                            {
                                bar.high = quote.last_price;
                                bar.low = quote.last_price;
                                bar.open = quote.last_price;
                                bar.close = quote.last_price;
                                bar.start_volume = quote.volume;
                            }
                            bar.tick_count ++;
                            bar.volume = quote.volume - bar.start_volume;
                            bar.high = std::max(bar.high, quote.last_price);
                            bar.low = std::min(bar.low, quote.last_price);
                            bar.close = quote.last_price;
                        }
                        if (quote.data_time >= bar.end_time)
                        {
                            this->get_writer(0)->write(0, msg::type::Bar, bar);
                            SPDLOG_INFO("{}.{} [o:{},c:{},h:{},l:{}] from {} to {}",bar.instrument_id, bar.exchange_id,
                                    bar.open, bar.close, bar.high, bar.low, time::strftime(bar.start_time), time::strftime(bar.end_time));
                            bar.start_time = bar.end_time;
                            while(bar.start_time + time_interval_ < quote.data_time)
                            {
                                bar.start_time += time_interval_;
                            }
                            bar.end_time = bar.start_time + time_interval_;
                            if (bar.start_time <= quote.data_time)
                            {
                                bar.tick_count = 1;
                                bar.start_volume = quote.volume;
                                bar.volume = 0;
                                bar.high = quote.last_price;
                                bar.low = quote.last_price;
                                bar.open = quote.last_price;
                                bar.close = quote.last_price;
                            }
                            else
                            {
                                bar.tick_count = 0;
                                bar.start_volume = 0;
                                bar.volume = 0;
                                bar.high = 0;
                                bar.low = 0;
                                bar.open = 0;
                                bar.close = 0;
                            }
                        }
                    }
                });

                events_ | is(msg::type::Subscribe) |
                $([&](event_ptr event)
                {
                    SPDLOG_INFO("subscribe request");
                    std::vector<Instrument> symbols;
                    const char *buffer = &(event->data<char>());
                    hffix::message_reader reader(buffer, buffer + event->data_length());
                    for (; reader.is_complete(); reader = reader.next_message_reader())
                    {
                        if (reader.is_valid())
                        {
                            auto group_mdentry_begin = std::find_if(reader.begin(), reader.end(), hffix::tag_equal(hffix::tag::MDEntryType));
                            hffix::message_reader::const_iterator group_mdentry_end;
                            for (; group_mdentry_begin != reader.end(); group_mdentry_begin = group_mdentry_end)
                            {
                                group_mdentry_end = std::find_if(group_mdentry_begin + 1, reader.end(), hffix::tag_equal(hffix::tag::MDEntryType));

                                auto group_instrument_begin = std::find_if(group_mdentry_begin, group_mdentry_end,
                                                                             hffix::tag_equal(hffix::tag::Symbol));
                                hffix::message_reader::const_iterator group_instrument_end;

                                for (; group_instrument_begin != group_mdentry_end; group_instrument_begin = group_instrument_end)
                                {
                                    group_instrument_end = std::find_if(group_instrument_begin + 1, group_mdentry_end,
                                                                          hffix::tag_equal(hffix::tag::Symbol));
                                    hffix::message_reader::const_iterator symbol = group_instrument_begin;
                                    hffix::message_reader::const_iterator exchange = group_instrument_begin;
                                    reader.find_with_hint(hffix::tag::SecurityExchange, exchange);

                                    Instrument instrument{};
                                    strcpy(instrument.instrument_id, symbol->value().as_string().c_str());
                                    strcpy(instrument.exchange_id, exchange->value().as_string().c_str());
                                    symbols.push_back(instrument);
                                }
                            }
                        }
                    }
                    subscribe(symbols);
                });
            }

        }
    }
}
