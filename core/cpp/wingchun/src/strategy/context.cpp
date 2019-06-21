//
// Created by Keren Dong on 2019-06-20.
//

#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>

#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/storage/storage.h>
#include <kungfu/wingchun/strategy/context.h>

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
        namespace strategy
        {
            Context::Context(practice::apprentice &app) :
                    app_(app),
                    uid_worker_storage_(app_.get_config_db_file("uid_worker")),
                    order_storage_(app_.get_app_db_file("orders")),
                    trade_storage_(app_.get_app_db_file("trades")),
                    calendar_(app_.get_config_db_file("holidays")),
                    uid_generator_(uid_worker_storage_.get_uid_worker_id(app_.get_io_device()->get_home()->name), UID_EPOCH_SECONDS)
            {}

            void Context::react(rx::observable<yijinjing::event_ptr> events)
            {
                events | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      auto quote = event->data<Quote>();
                  });

                events | is(msg::type::Order) |
                $([&](event_ptr event)
                  {
                      auto order = event->data<Order>();
                  });

                events | is(msg::type::Trade) |
                $([&](event_ptr event)
                  {
                      auto trade = event->data<Trade>();
                  });

                events | is(msg::type::Entrust) |
                $([&](event_ptr event)
                  {
                      auto entrust = event->data<Entrust>();
                  });

                events | is(msg::type::Transaction) |
                $([&](event_ptr event)
                  {
                      auto transaction = event->data<Transaction>();
                  });
            }

            int64_t Context::now() const
            {
                return 0;
            }

            bool Context::add_md(const std::string &source_id)
            {
                auto home = app_.get_io_device()->get_home();
                app_.observe(location::make(home->mode, category::MD, source_id, source_id, home->locator));
                return true;
            }

            bool Context::add_account(const std::string &source_id, const std::string &account_id, double cash_limit)
            {
                if (accounts_.find(account_id) != accounts_.end())
                {
                    throw wingchun_error("duplicated account " + account_id);
                }

                auto home = app_.get_io_device()->get_home();
                auto account_location = location::make(mode::LIVE, category::TD, source_id, account_id, home->locator);
                auto commission_db_file = home->locator->layout_file(account_location, yijinjing::data::layout::SQLITE, "commission");
                account_managers_[account_id] = std::make_shared<AccountManager>(account_id, calendar_, commission_db_file);
                accounts_[account_id] = msg::data::AccountInfo{};

                auto account = accounts_[account_id];
                strcpy(account.account_id, account_id.c_str());
                strcpy(account.source_id, source_id.c_str());
                strcpy(account.trading_day, std::to_string(calendar_.get_current_trading_day()).c_str());
                account.initial_equity = cash_limit;
                account.static_equity = cash_limit;
                account.dynamic_equity = cash_limit;
                account.avail = cash_limit;
            }

            void Context::subscribe(const std::string &source, const std::vector<std::string> &symbols, const std::string &exchange_id,
                                    bool is_level2)
            {
                location md_loc(mode::LIVE, category::MD, source, source, app_.get_io_device()->get_home()->locator);
                auto writer = app_.get_writer(md_loc.uid);
                char *buffer = reinterpret_cast<char *>(writer->open_frame(0, msg::type::Subscribe)->address());
                hffix::message_writer sub_msg(buffer, buffer + 1024);
                sub_msg.push_back_string(hffix::tag::ExDestination, exchange_id);
                for (const auto &symbol : symbols)
                {
                    sub_msg.push_back_string(hffix::tag::Symbol, symbol);
                }

                writer->close_frame(sub_msg.message_end() - buffer);
            }

            uint64_t Context::insert_limit_order(const std::string &instrument_id, const std::string &exchange_id,
                                                 const std::string &account_id, double limit_price, int64_t volume,
                                                 Side side, Offset offset)
            {
                auto input = app_.get_writer(0)->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = next_id();
                strcpy(input.instrument_id, instrument_id.c_str());
                strcpy(input.exchange_id, exchange_id.c_str());
                strcpy(input.account_id, account_id.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;

                input.price_type = PriceTypeLimit;
                input.time_condition = TimeConditionGFD;
                input.volume_condition = VolumeConditionAny;

                app_.get_writer(0)->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_fak_order(const std::string &instrument_id, const std::string &exchange_id,
                                               const std::string &account_id, double limit_price, int64_t volume,
                                               Side side, Offset offset)
            {
                auto input = app_.get_writer(0)->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                strcpy(input.instrument_id, instrument_id.c_str());
                strcpy(input.exchange_id, exchange_id.c_str());
                strcpy(input.account_id, account_id.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;

                input.price_type = PriceTypeLimit;
                input.time_condition = TimeConditionIOC;
                input.volume_condition = VolumeConditionAny;

                app_.get_writer(0)->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_fok_order(const std::string &instrument_id, const std::string &exchange_id,
                                               const std::string &account_id, double limit_price, int64_t volume,
                                               Side side, Offset offset)
            {
                auto input = app_.get_writer(0)->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                strcpy(input.instrument_id, instrument_id.c_str());
                strcpy(input.exchange_id, exchange_id.c_str());
                strcpy(input.account_id, account_id.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;

                input.price_type = PriceTypeLimit;
                input.time_condition = TimeConditionIOC;
                input.volume_condition = VolumeConditionAll;

                app_.get_writer(0)->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_market_order(const std::string &instrument_id, const std::string &exchange_id,
                                                  const std::string &account_id, int64_t volume, Side side, Offset offset)
            {
                auto input = app_.get_writer(0)->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                strcpy(input.instrument_id, instrument_id.c_str());
                strcpy(input.exchange_id, exchange_id.c_str());
                strcpy(input.account_id, account_id.c_str());
                auto iter = quote_map_.find(get_symbol(instrument_id, exchange_id));
                if (iter != quote_map_.end())
                {
                    input.frozen_price = iter->second.last_price;
                }
                input.volume = volume;
                input.side = side;
                input.offset = offset;

                if (strcmp(input.exchange_id, EXCHANGE_SSE) == 0 || strcmp(input.exchange_id, EXCHANGE_SZE) == 0) //沪深市，最优五档转撤销
                {
                    input.price_type = PriceTypeBest5;
                    input.time_condition = TimeConditionIOC;
                    input.volume_condition = VolumeConditionAny;
                } else
                {
                    input.price_type = PriceTypeAny;
                    input.time_condition = TimeConditionIOC;
                    input.volume_condition = VolumeConditionAny;
                }

                app_.get_writer(0)->close_data();
                return input.order_id;
            }

            uint64_t Context::cancel_order(uint64_t order_id)
            {
                auto action = app_.get_writer(0)->open_data<msg::data::OrderAction>(0, msg::type::OrderAction);

                uint64_t uid = next_id();
                action.order_action_id = uid;
                action.order_id = order_id;
                action.action_flag = OrderActionFlagCancel;

                app_.get_writer(0)->close_data();
                return uid;
            }

            uint64_t Context::next_id()
            {
                int64_t seconds = yijinjing::time::now_in_nano() / yijinjing::time_unit::NANOSECONDS_PER_SECOND;
                return uid_generator_.next_id(seconds);
            }
        }
    }
}
