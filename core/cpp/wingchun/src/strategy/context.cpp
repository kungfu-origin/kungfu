//
// Created by Keren Dong on 2019-06-20.
//

#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/msg.h>
#include <kungfu/wingchun/strategy/context.h>
#include <kungfu/wingchun/utils.h>

using namespace kungfu::practice;
using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::msg::data;

namespace kungfu
{
    namespace wingchun
    {
        namespace strategy
        {
            Context::Context(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events) :
                app_(app), events_(events), subscribe_all_(false)
            {
                auto home = app.get_io_device()->get_home();
                log::copy_log_settings(home, home->name);
                book_context_ = std::make_shared<book::BookContext>(app, events);
                algo_context_ = std::make_shared<algo::AlgoContext>(app, events);
            }

            void Context::react()
            {
                algo_context_->react();

                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      const Quote &quote = event->data<Quote>();
                      auto id = get_symbol_id(quote.get_instrument_id(), quote.get_exchange_id());
                      quotes_[id].last_price = quote.last_price;
                  });

                events_ | is(msg::type::Order) | to(app_.get_home_uid()) |
                $([&](event_ptr event)
                  {
                      auto order = event->data<Order>();
                  });

                events_ | is(msg::type::Trade) | to(app_.get_home_uid()) |
                $([&](event_ptr event)
                  {
                      auto trade = event->data<Trade>();
                  });

                events_ | is(msg::type::Entrust) |
                $([&](event_ptr event)
                  {
                      auto entrust = event->data<Entrust>();
                  });

                events_ | is(msg::type::Transaction) |
                $([&](event_ptr event)
                  {
                      auto transaction = event->data<Transaction>();
                  });

                subscribe_instruments();

                auto home = app_.get_io_device()->get_home();
                auto ledger_location = location::make(mode::LIVE, category::SYSTEM, "service", "ledger", home->locator);
                app_.request_write_to(app_.now(), ledger_location->uid);
                app_.request_read_from(app_.now(), ledger_location->uid);
            }

            int64_t Context::now() const
            {
                return app_.now();
            }

            void Context::add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)>& callback)
            {
                app_.add_timer(nanotime, callback);
            }

            void Context::add_time_interval(int64_t duration, const std::function<void(yijinjing::event_ptr)>& callback)
            {
                app_.add_time_interval(duration, callback);
            }

            void Context::add_account(const std::string &source, const std::string &account, double cash_limit)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                if (accounts_.find(account_id) != accounts_.end())
                {
                    throw wingchun_error(fmt::format("duplicated account {}@{}", account, source));
                }

                auto home = app_.get_io_device()->get_home();
                auto account_location = location::make(mode::LIVE, category::TD, source, account, home->locator);
                if (home->mode == mode::LIVE and not app_.has_location(account_location->uid))
                {
                    throw wingchun_error(fmt::format("invalid account {}@{}", account, source));
                }

                accounts_[account_id] = account_location;
                account_cash_limits_[account_id] = cash_limit;
                account_location_ids_[account_id] = account_location->uid;

                app_.request_write_to(app_.now(), account_location->uid);
                app_.request_read_from(app_.now(), account_location->uid);

                SPDLOG_INFO("added account {}@{} [{:08x}]", account, source, account_id);
            }

            std::vector<yijinjing::data::location_ptr> Context::list_accounts()
            {
                std::vector<yijinjing::data::location_ptr> acc_locations;
                for (auto item : accounts_)
                {
                    acc_locations.push_back(item.second);
                }
                return acc_locations;
            }

            double Context::get_account_cash_limit(const std::string &account)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                if (account_cash_limits_.find(account_id) == account_cash_limits_.end())
                {
                    throw wingchun_error(fmt::format("invalid account {}", account));
                }
                return account_cash_limits_[account_id];
            }

            void Context::subscribe_instruments()
            {
                auto home = app_.get_io_device()->get_home();
                auto ledger_location = location::make(mode::LIVE, category::SYSTEM, "service", "ledger", home->locator);
                if (home->mode == mode::LIVE and not app_.has_location(ledger_location->uid))
                {
                    throw wingchun_error("has no location for ledger service");
                }
                if (not app_.has_writer(ledger_location->uid))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == ledger_location->uid;
                           }) | first() |
                           $([=](event_ptr e)
                           {
                               auto writer = app_.get_writer(ledger_location->uid);
                               writer->mark(0, msg::type::InstrumentRequest);
                               SPDLOG_INFO("instrument requested");
                           });
                } else
                {
                    auto writer = app_.get_writer(ledger_location->uid);
                    writer->mark(0, msg::type::InstrumentRequest);
                    SPDLOG_INFO("instrument requested");
                }
            }

            uint32_t Context::add_marketdata(const std::string &source)
            {
                if (market_data_.find(source) == market_data_.end())
                {
                    auto home = app_.get_io_device()->get_home();
                    auto md_location = source == "bar" ? location::make(mode::LIVE, category::SYSTEM, "service", source, home->locator) :
                                       location::make(mode::LIVE, category::MD, source, source, home->locator);
                    if (not app_.has_location(md_location->uid))
                    {
                        throw wingchun_error(fmt::format("invalid md {}", source));
                    }
                    app_.request_read_from(app_.now(), md_location->uid, true);
                    app_.request_write_to(app_.now(), md_location->uid);
                    market_data_[source] = md_location->uid;
                    SPDLOG_INFO("added md {} [{:08x}]", source, md_location->uid);
                }
                return market_data_[source];
            }

            void Context::subscribe_all(const std::string &source)
            {
                subscribe_all_ = true;
                auto md_source = add_marketdata(source);
                SPDLOG_INFO("strategy subscribe all from {} [{:08x}]", source, md_source);
                if (not app_.has_writer(md_source))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == md_source;
                           }) | first() |
                    $([=](event_ptr e)
                      {
                            auto writer = app_.get_writer(md_source);
                            writer->mark(0, msg::type::SubscribeAll);
                      });
                } else
                {
                    auto writer = app_.get_writer(md_source);
                    writer->mark(0, msg::type::SubscribeAll);
                }
            }

            void Context::subscribe(const std::string &source, const std::vector<std::string> &symbols, const std::string &exchange)
            {
                for (const auto& symbol: symbols)
                {
                    auto symbol_id = get_symbol_id(symbol, exchange);
                    subscribed_symbols_[symbol_id] = symbol_id;
                }

                auto md_source = add_marketdata(source);
                SPDLOG_INFO("strategy subscribe from {} [{:08x}]", source, md_source);
                if (not app_.has_writer(md_source))
                {
                    events_ | is(yijinjing::msg::type::RequestWriteTo) |
                    filter([=](yijinjing::event_ptr e)
                           {
                               const yijinjing::msg::data::RequestWriteTo &data = e->data<yijinjing::msg::data::RequestWriteTo>();
                               return data.dest_id == md_source;
                           }) | first() |
                    $([=](event_ptr e)
                      {
                          request_subscribe(md_source, symbols, exchange);
                      });
                } else
                {
                    request_subscribe(md_source, symbols, exchange);
                }
            }

            void Context::request_subscribe(uint32_t source, const std::vector<std::string> &symbols, const std::string &exchange)
            {
                for (const auto &symbol : symbols)
                {
                    write_subscribe_msg(app_.get_writer(source), app_.now(), exchange, symbol);
                }
            }

            uint64_t Context::insert_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                                 double limit_price, int64_t volume, PriceType type, Side side, Offset offset, HedgeFlag hedge_flag)
            {
                auto inst_type = get_instrument_type(symbol, exchange);
                if (inst_type == InstrumentType::Unknown || inst_type == InstrumentType::Repo)
                {
                    SPDLOG_ERROR("unsupported instrument type {} of {}.{}", str_from_instrument_type(inst_type), symbol, exchange);
                    return 0;
                }
                auto writer = app_.get_writer(lookup_account_location_id(account));
                msg::data::OrderInput &input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);
                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.price_type = type;
                input.side = side;
                input.offset = offset;
                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::cancel_order(uint64_t order_id)
            {
                uint32_t account_location_id = (order_id >> 32) ^ app_.get_home_uid();
                SPDLOG_INFO("{:08x} cancel order {:016x} with account location {:08x}", app_.get_home_uid(), order_id, account_location_id);
                if (app_.has_writer(account_location_id))
                {
                    auto writer = app_.get_writer(account_location_id);
                    msg::data::OrderAction &action = writer->open_data<msg::data::OrderAction>(0, msg::type::OrderAction);

                    action.order_action_id = writer->current_frame_uid();
                    action.order_id = order_id;
                    action.action_flag = OrderActionFlag::Cancel;

                    writer->close_data();
                    return action.order_action_id;
                }
                else
                {
                    SPDLOG_ERROR("has no writer for {}", account_location_id);
                    return 0;
                }
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
