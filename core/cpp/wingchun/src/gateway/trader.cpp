//
// Created by Keren Dong on 2019-06-20.
//

#include <kungfu/yijinjing/time.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/trader.h>
#include <kungfu/wingchun/util/business_helper.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

typedef rxcpp::schedulers::scheduler::clock_type::time_point::duration duration_type;

namespace kungfu
{
    namespace wingchun
    {
        namespace gateway
        {
            Trader::Trader(bool low_latency, locator_ptr locator, const std::string &source, const std::string &account_id)
                    :
                    apprentice(location::make(mode::LIVE, category::TD, source, account_id, locator), low_latency),
                    source_(source), account_id_(account_id),
                    order_storage_(get_app_db_file("orders")),
                    trade_storage_(get_app_db_file("trades")),
                    calendar_(get_config_db_file("holidays")),
                    account_manager_(account_id, calendar_, get_app_db_file("commission"))
            {
                log::copy_log_settings(get_io_device()->get_home(), account_id);
            }

            std::vector<uint64_t> Trader::get_pending_orders(const std::string &client_id) const
            {
                std::vector<uint64_t> order_ids;
//                auto orders = order_manager_->get_pending_orders();
//                for (const auto &order : orders)
//                {
//                    auto simple_order = std::dynamic_pointer_cast<oms::SimpleOrder>(order);
//                    if (nullptr != simple_order)
//                    {
//                        if (client_id.empty() || client_id == simple_order->get_client_id())
//                        {
//                            order_ids.emplace_back(simple_order->get_order_id());
//                        }
//                    }
//                }
                return order_ids;
            }

            void Trader::react(const rx::observable<yijinjing::event_ptr> &events)
            {
                apprentice::react(events);

                events | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      account_manager_.on_quote(&event->data<Quote>());
                  });

                events | is(msg::type::OrderInput) |
                $([&](event_ptr event)
                  {
                      insert_order(event);
                  });

                events | is(msg::type::OrderAction) |
                $([&](event_ptr event)
                  {
                      cancel_order(event);
                  });

                events | is(msg::type::Order) |
                $([&](event_ptr event)
                  {
                      auto order = event->data<Order>();
                      account_manager_.on_order(&order);
                      order_storage_.add_order(order.order_id, order);
                  });

                events | is(msg::type::Trade) |
                $([&](event_ptr event)
                  {
                      auto trade = event->data<Trade>();

                      double commission = account_manager_.calc_commission(&trade);
                      double tax = account_manager_.calc_tax(&trade);
                      trade.commission = commission;
                      trade.tax = tax;

                      account_manager_.on_trade(&trade);
                      account_manager_.dump_to_db(get_app_db_file("asset"), true);

                      auto trade_id = trade_storage_.add_trade(trade);
                      trade.id = trade_id;
                  });

                events | is(msg::type::Position) |
                $([&](event_ptr event)
                  {
                      auto position = event->data<Position>();
                      rsp_pos_.emplace_back(position);
                  });

                events | is(msg::type::PositionEnd) |
                $([&](event_ptr event)
                  {
                      account_manager_.on_positions(rsp_pos_);
                      rsp_pos_.clear();
                  });

                events | is(msg::type::PositionDetail) |
                $([&](event_ptr event)
                  {
                      auto position = event->data<Position>();
                      rsp_pos_detail_.emplace_back(position);
                  });

                events | is(msg::type::PositionDetailEnd) |
                $([&](event_ptr event)
                  {
                      account_manager_.on_positions(rsp_pos_detail_);
                      rsp_pos_detail_.clear();
                  });

                events | is(msg::type::AccountInfo) |
                $([&](event_ptr event)
                  {
                      auto account = event->data<AccountInfo>();
                      strcpy(account.trading_day, std::to_string(calendar_.get_current_trading_day()).c_str());
                      account_manager_.on_account(account);
                  });

                events | is(msg::type::SwitchDay) |
                $([&](event_ptr event)
                  {
                      auto trading_day = event->data<char *>();
                      account_manager_.on_switch_day(std::string(trading_day));
                      account_manager_.dump_to_db(get_app_db_file("asset"), true);
                  });

//                events | time_interval(std::chrono::minutes(1), observe_on_new_thread()) |
//                $([&](duration_type v)
//                  {
//                      auto account_info = account_manager_.get_account_info();
//                      bool is_open = calendar_.is_open(time::now_in_nano(), EXCHANGE_SSE);
//                      if (is_open)
//                      {
//                          account_info.update_time = (int64_t) std::round((double) yijinjing::time::now_in_nano() / 1000000000) * 1000000000;
//
//                          storage::SnapshotStorage(get_app_db_file("snapshots"), ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME, false, true).insert(
//                                  account_info);
//                      }
//                  });
//
//                events | time_interval(std::chrono::hours(24), observe_on_new_thread()) |
//                $([&](duration_type v)
//                  {
//                      auto account_info = account_manager_.get_account_info();
//                      account_info.update_time = (int64_t) std::round((double) yijinjing::time::now_in_nano() / 1000000000) * 1000000000;
//                      storage::SnapshotStorage(get_app_db_file("snapshots"), ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME, true, true).insert(account_info);
//                  });
            }

            void Trader::start()
            {
                apprentice::start();
                auto home = get_io_device()->get_home();
                observe(location::make(home->mode, category::MD, source_, source_, home->locator), time::now_in_nano());
            }

            void Trader::init_account_manager()
            {
                int64_t last_update = account_manager_.get_last_update();
                if (last_update > 0)
                {
                    auto io = get_io_device();
                    auto reader = io->open_reader_to_subscribe();
                    reader->join(
                            std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::MD, source_,
                                                                        source_, io->get_home()->locator), 0, last_update);
                    reader->join(
                            std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::TD, source_,
                                                                        account_id_, io->get_home()->locator), 0, last_update);
                    while (reader->data_available())
                    {
                        auto frame = reader->current_frame();
                        switch (frame->msg_type())
                        {
                            case msg::type::Quote:
                            {
                                auto quote = frame->data<msg::data::Quote>();
                                if (quote.rcv_time > last_update)
                                {
                                    account_manager_.on_quote(&quote);
                                }
                                break;
                            }
                            case msg::type::Order:
                            {
                                auto order = frame->data<msg::data::Order>();
                                if (order.rcv_time > last_update)
                                {
                                    account_manager_.on_order(&order);
                                }
                                break;
                            }
                            case msg::type::Trade:
                            {
                                auto trade = frame->data<msg::data::Trade>();
                                if (trade.rcv_time > last_update)
                                {
                                    account_manager_.on_trade(&trade);
                                }
                                break;
                            }
                            case msg::type::AccountInfo:
                            {
                                auto account = frame->data<msg::data::AccountInfo>();
                                if (account.rcv_time > last_update)
                                {
                                    account_manager_.on_account(account);
                                }
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                        reader->next();
                    }
                    SPDLOG_INFO("forwarded account manager from {} to {}", yijinjing::time::strftime(last_update, "%Y%m%d-%H:%M:%S"),
                                yijinjing::time::strftime(account_manager_.get_last_update(), "%Y%m%d-%H:%M:%S"));
                }

                account_manager_.set_current_trading_day(std::to_string(calendar_.get_current_trading_day()));
                account_manager_.dump_to_db(get_app_db_file("assets"), true);
                SPDLOG_INFO("account_manager inited and set to {}", account_manager_.get_current_trading_day());
            }
        }
    }
}
