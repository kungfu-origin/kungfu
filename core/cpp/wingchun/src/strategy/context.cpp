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
            inline uint32_t get_symbol_id(const std::string &symbol, const std::string &exchange)
            {
                return yijinjing::util::hash_str_32(symbol) ^ yijinjing::util::hash_str_32(exchange);
            }

            Context::Context(practice::apprentice &app) :
                    app_(app), now_(0),
                    calendar_(app_.get_config_db_file("holidays"))
            {}

            void Context::react(const rx::observable<yijinjing::event_ptr> &events)
            {
                events |
                $([&](event_ptr event)
                  {
                      now_ = event->gen_time();
                  });

                events | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      auto quote = event->data<Quote>();
                      auto id = get_symbol_id(quote.get_instrument_id(), quote.get_exchange_id());
                      quotes_[id].last_price = quote.last_price;
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
                return now_;
            }

            void Context::add_md(const std::string &source)
            {
                auto home = app_.get_io_device()->get_home();
                app_.observe(location::make(home->mode, category::MD, source, source, home->locator));
            }

            void Context::add_account(const std::string &source, const std::string &account, double cash_limit)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                if (accounts_.find(account_id) != accounts_.end())
                {
                    throw wingchun_error("duplicated account " + account);
                }

                auto home = app_.get_io_device()->get_home();
                auto account_location = location::make(mode::LIVE, category::TD, source, account, home->locator);
                account_location_ids_[account_id] = account_location->uid;

                auto commission_db_file = home->locator->layout_file(account_location, yijinjing::data::layout::SQLITE, "commission");
                account_managers_[account_id] = std::make_shared<AccountManager>(account, calendar_, commission_db_file);
                accounts_[account_id] = msg::data::AccountInfo{};

                auto info = accounts_[account_id];
                strcpy(info.account_id, account.c_str());
                strcpy(info.source_id, source.c_str());
                strcpy(info.trading_day, std::to_string(calendar_.get_current_trading_day()).c_str());
                info.initial_equity = cash_limit;
                info.static_equity = cash_limit;
                info.dynamic_equity = cash_limit;
                info.avail = cash_limit;

                app_.observe(account_location);
            }

            void Context::subscribe(const std::string &source, const std::vector<std::string> &symbols, const std::string &exchange, bool is_level2)
            {
                location md_loc(mode::LIVE, category::MD, source, source, app_.get_io_device()->get_home()->locator);
                auto writer = app_.get_writer(md_loc.uid);
                char *buffer = reinterpret_cast<char *>(writer->open_frame(0, msg::type::Subscribe)->address());
                hffix::message_writer sub_msg(buffer, buffer + 1024);
                sub_msg.push_back_string(hffix::tag::ExDestination, exchange);
                for (const auto &symbol : symbols)
                {
                    sub_msg.push_back_string(hffix::tag::Symbol, symbol);
                    quotes_[get_symbol_id(symbol, exchange)] = msg::data::Quote{};
                }

                writer->close_frame(sub_msg.message_end() - buffer);
            }

            uint64_t Context::insert_limit_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                                 double limit_price, int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                auto input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceTypeLimit;
                input.time_condition = TimeConditionGFD;
                input.volume_condition = VolumeConditionAny;

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_fak_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                               double limit_price, int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                auto input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceTypeLimit;
                input.time_condition = TimeConditionIOC;
                input.volume_condition = VolumeConditionAny;

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_fok_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                               double limit_price, int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                auto input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceTypeLimit;
                input.time_condition = TimeConditionIOC;
                input.volume_condition = VolumeConditionAll;

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_market_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                                  int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                auto input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                auto iter = quotes_.find(get_symbol_id(symbol, exchange));
                if (iter != quotes_.end())
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

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::cancel_order(uint64_t order_id)
            {
                uint32_t account_id = order_id >> 32;
                if (account_location_ids_.find(account_id) == account_location_ids_.end())
                {
                    throw wingchun_error(fmt::format("invalid order id {}", order_id));
                }

                auto writer = app_.get_writer(account_location_ids_[account_id]);
                auto action = writer->open_data<msg::data::OrderAction>(0, msg::type::OrderAction);

                action.order_action_id = writer->current_frame_uid();
                action.order_id = order_id;
                action.action_flag = OrderActionFlagCancel;

                writer->close_data();
                return action.order_action_id;
            }

            uint32_t Context::lookup_account_location_id(const std::string &account)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                if (account_location_ids_.find(account_id) == account_location_ids_.end())
                {
                    throw wingchun_error("invalid account " + account);
                }

                return account_location_ids_[account_id];
            }
        }
    }
}
