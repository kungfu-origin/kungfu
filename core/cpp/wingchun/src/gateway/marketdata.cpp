//
// Created by Keren Dong on 2019-06-20.
//

#include <hffix.hpp>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/marketdata.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            MarketData::MarketData(bool low_latency, locator_ptr locator, const std::string &source) :
                apprentice(location::make(mode::LIVE, category::MD, source, source, locator), low_latency)
            {
                log::copy_log_settings(get_io_device()->get_home(), source);
            }

            void MarketData::react(rx::observable<yijinjing::event_ptr> events)
            {
                apprentice::react(events);

                events | is(msg::type::Subscribe) |
                $([&](event_ptr event)
                  {
                      std::vector<Instrument> symbols;
                      auto buffer = event->data<char *>();
                      hffix::message_reader reader(buffer, buffer + event->data_length());
                      for (; reader.is_complete(); reader = reader.next_message_reader())
                      {
                          if (reader.is_valid())
                          {
                              hffix::message_reader::const_iterator i = reader.begin();
                              if (reader.find_with_hint(hffix::tag::Symbol, i))
                              {
                                  auto symbol = i++->value();
                                  Instrument instrument{};
                                  strcpy(instrument.instrument_id, symbol.as_string().c_str());
                                  symbols.push_back(instrument);
                              }
                          }
                      }
//                      MarketData::subscribe(symbols);
                  });
            }

            void MarketData::start()
            {
                apprentice::start();
            }
        }
    }
}
