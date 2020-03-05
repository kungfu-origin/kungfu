//
// Created by qlu on 2019-10-26.
//

#include <kungfu/wingchun/algo/algo.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/wingchun/utils.h>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::algo
{
    AlgoContext::AlgoContext(yijinjing::practice::apprentice &app, const rx::connectable_observable<event_ptr> &events) :
            app_(app), events_(events)
    {
        auto home = app.get_io_device()->get_home();
        service_location_ = yijinjing::data::location::make_shared(mode::LIVE, category::SYSTEM, "service", "algo", home->locator);
    }

    void AlgoContext::react()
    {
        events_ | is(Quote::tag) |
        $([&](const event_ptr &event)
          {
              for (const auto &item: orders_)
              {
                  item.second->on_quote(shared_from_this(), event->data<Quote>());
              }
          });

        events_ | is(Order::tag) |
        $([&](const event_ptr &event)
          {
              const auto &order = event->data<Order>();
              auto order_id = order.parent_id;
              if (has_order(order_id))
              {
                  orders_[order_id]->on_child_order(shared_from_this(), order);
              }
          });

        events_ | is(Trade::tag) |
        $([&](const event_ptr &event)
          {
              const auto &trade = event->data<Trade>();
              auto order_id = trade.parent_order_id;
              if (has_order(order_id))
              {
                  orders_[order_id]->on_child_trade(shared_from_this(), trade);
              }
          });

        events_ | is(AlgoOrderReport::tag) |
        $([&](const event_ptr &event)
          {
              try
              {
                  auto json_str = event->data_as_string();
                  SPDLOG_TRACE("rcv algo order report {}", json_str);
                  nlohmann::json report_json = nlohmann::json::parse(json_str);
                  uint64_t order_id = report_json["order_id"];
                  if (has_order(order_id))
                  {
                      orders_[order_id]->on_order_report(shared_from_this(), json_str);
                  } else
                  {
                      SPDLOG_WARN("has no order {}", order_id);
                  }
              }
              catch (const std::exception &ex)
              {
                  SPDLOG_ERROR("Unexpected exception: {}", ex.what());
              }
          });
    }

    uint64_t AlgoContext::add_order(const AlgoOrder_ptr &order)
    {
        if (app_.get_io_device()->get_home()->uid != service_location_->uid)
        {
            try
            {
                std::string msg = order->dumps();
                auto writer = app_.get_writer(service_location_->uid);
                auto &&frame = writer->open_frame(0, AlgoOrderInput::tag, msg.length());
                order->order_id_ = writer->current_frame_uid();
                msg = order->dumps();
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writer->close_frame(msg.length());
            }
            catch (const std::exception &ex)
            {
                SPDLOG_ERROR("Unexpected exception: {}", ex.what());
                return 0;
            }
        }
        orders_[order->order_id_] = order;
        return order->order_id_;
    }

    uint64_t AlgoContext::cancel_order(const AlgoOrder_ptr &order)
    {
        if (app_.get_io_device()->get_home()->uid != service_location_->uid)
        {
            auto writer = app_.get_writer(service_location_->uid);
            OrderAction &action = writer->open_data<OrderAction>(0);
            action.order_action_id = writer->current_frame_uid();
            action.order_id = order->order_id_;
            action.action_flag = OrderActionFlag::Cancel;
            writer->close_data();
            return action.order_action_id;
        }
        //TODO process algo order cancel logic
        return 0;
    }

    void AlgoContext::subscribe(const std::string &source, const std::vector<std::string> &instruments, const std::string &exchange)
    {
        auto md_location = location::make_shared(mode::LIVE, category::MD, source, source, app_.get_locator());
        auto writer = app_.get_writer(md_location->uid);
        for (const auto &inst : instruments)
        {
            write_subscribe_msg(writer, app_.now(), exchange, inst);
        }
    }

    uint64_t AlgoContext::insert_order(const OrderInput &order)
    {
        auto home = app_.get_io_device()->get_home();
        if (home->uid != service_location_->uid)
        {
            return 0;
        }
        auto account_location = location::make_shared(mode::LIVE, category::TD, order.source_id, order.account_id, home->locator);
        auto writer = app_.get_writer(account_location->uid);
        OrderInput &input = writer->open_data<OrderInput>(0);
        memcpy(&input, &order, sizeof(OrderInput));
        input.order_id = writer->current_frame_uid();
        writer->close_data();
        return input.order_id;
    }

    uint64_t AlgoContext::cancel_order(uint64_t order_id)
    {
        auto home = app_.get_io_device()->get_home();
        if (home->uid != service_location_->uid)
        {
            return 0;
        }
        uint32_t account_location_id = (order_id >> 32) xor app_.get_home_uid();
        auto writer = app_.get_writer(account_location_id);
        OrderAction &action = writer->open_data<OrderAction>(0);
        action.order_action_id = writer->current_frame_uid();
        action.order_id = order_id;
        action.action_flag = OrderActionFlag::Cancel;
        writer->close_data();
        return action.order_action_id;
    }
}
