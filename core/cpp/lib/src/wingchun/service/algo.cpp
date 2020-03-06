//
// Created by qlu on 2019-10-31.
//

#include <utility>

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/utils.h>
#include <kungfu/wingchun/service/algo.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;


namespace kungfu::wingchun::service
{
    Algo::Algo(locator_ptr locator, mode m, bool low_latency) :
            apprentice(location::make_shared(m, category::SYSTEM, "service", "algo", std::move(locator)), low_latency)
    {
        log::copy_log_settings(get_io_device()->get_home(), "algo");
    }

    void Algo::on_start()
    {
        events_ | is(Register::tag) |
        $([&](const event_ptr &event)
          {
              auto register_data = event->data<Register>();
              auto source_location = location::make_shared(register_data, get_locator());
              auto trigger_time = event->gen_time();

              switch (source_location->category)
              {
                  case category::MD:
                  {
                      request_read_from_public(trigger_time, source_location->uid, trigger_time);
                      request_write_to(trigger_time, source_location->uid);
                      break;
                  }
                  case category::TD:
                  {
                      request_read_from(trigger_time, source_location->uid, trigger_time);
                      request_write_to(trigger_time, source_location->uid);
                      break;
                  }
                  case category::STRATEGY:
                  {
                      request_read_from(trigger_time, source_location->uid, trigger_time);
                      request_write_to(trigger_time, source_location->uid);
                      break;
                  }
                  default:
                  {
                      break;
                  }
              }
          }
        );

        events_ | is(AlgoOrderInput::tag) |
        $([&](const event_ptr &event)
          {
              insert_order(event, event->data_as_string());
          });

        events_ | is(OrderAction::tag) |
        $([&](const event_ptr &event)
          {
              cancel_order(event, event->data<OrderAction>());
          });

        events_ | is(AlgoOrderModify::tag) |
        $([&](const event_ptr &event)
          {
              modify_order(event, event->data_as_string());
          });

        context_ = std::make_shared<algo::AlgoContext>(*this, events_);
        context_->react();

        apprentice::on_start();
    }
}
