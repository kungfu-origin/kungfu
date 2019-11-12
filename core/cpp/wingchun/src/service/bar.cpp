
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
                              if(subscribed_insts_.find(symbol_id) ==  subscribed_insts_.end())
                              {
                                  write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_instrument_id());
                                  auto now_in_nano = now();
                                  auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                                  SPDLOG_INFO("subscribe {}.{}", inst.instrument_id, inst.exchange_id);
                                  request_subscribe(start_time, start_time + time_interval_, inst);
                                  subscribed_insts_.insert(symbol_id);
                              }
                          }
                      });
                } else
                {
                    for (const auto& inst: instruments)
                    {
                        auto symbol_id = get_symbol_id(inst.get_instrument_id(), inst.get_exchange_id());
                        if(subscribed_insts_.find(symbol_id) ==  subscribed_insts_.end())
                        {
                            write_subscribe_msg(get_writer(source_location_->uid), 0, inst.get_exchange_id(), inst.get_instrument_id());
                            auto now_in_nano = now();
                            auto start_time = now_in_nano - now_in_nano % time_interval_ + time_interval_;
                            SPDLOG_INFO("subscribe {}.{}", inst.instrument_id, inst.exchange_id);
                            request_subscribe(start_time, start_time + time_interval_, inst);
                            subscribed_insts_.insert(symbol_id);
                        }
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
                        }) | skip_while([start_time](event_ptr event) { return event->data<Quote>().data_time < start_time; })
                                | take_while([end_time](event_ptr event)
                                        {
                                            return event->data<Quote>().data_time < end_time;
                                        });

                auto bar = quotes.reduce(Bar(), [](Bar res, event_ptr event)
                {
                    const auto& quote = event->data<Quote>();
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
                                strcpy(res.instrument_id, instrument.instrument_id);
                                strcpy(res.exchange_id, instrument.exchange_id);
                                SPDLOG_INFO("{}.{} [o:{},c:{},h:{},l:{}] from {} to {}",res.instrument_id, res.exchange_id,
                                        res.open, res.close, res.high, res.low, time::strftime(res.start_time), time::strftime(res.end_time));
                                this->get_writer(0)->write(0, msg::type::Bar, res);
                                request_subscribe(end_time, end_time + time_interval_, instrument);
                            });
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
