//
// Created by Keren Dong on 2019-06-20.
//

#include <fmt/format.h>
#include <hffix.hpp>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/msg.h>
#include <kungfu/wingchun/strategy/context.h>

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
                    app_(app), events_(events)
            {}

            void Context::react()
            {
                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      const Quote &quote = event->data<Quote>();
                      auto id = get_symbol_id(quote.get_instrument_id(), quote.get_exchange_id());
                      quotes_[id].last_price = quote.last_price;
                  });

                events_ | is(msg::type::Order) |
                $([&](event_ptr event)
                  {
                      auto order = event->data<Order>();
                  });

                events_ | is(msg::type::Trade) |
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
                if (not app_.has_location(account_location->uid))
                {
                    throw wingchun_error(fmt::format("invalid account {}@{}", account, source));
                }
                account_location_ids_[account_id] = account_location->uid;

                app_.request_write_to(app_.now(), account_location->uid);
                app_.request_read_from(app_.now(), account_location->uid, true);
                app_.request_read_from(app_.now(), account_location->uid);
                SPDLOG_INFO("added account {}@{} [{:08x}]", account, source, account_id);
            }

            void Context::subscribe(const std::string &source, const std::vector<std::string> &symbols, const std::string &exchange)
            {
                if (market_data_.find(source) == market_data_.end())
                {
                    auto home = app_.get_io_device()->get_home();
                    auto md_location = location::make(mode::LIVE, category::MD, source, source, home->locator);
                    if (not app_.has_location(md_location->uid))
                    {
                        throw wingchun_error(fmt::format("invalid md {}", source));
                    }
                    app_.request_read_from(app_.now(), md_location->uid, true);
                    app_.request_write_to(app_.now(), md_location->uid);
                    market_data_[source] = md_location->uid;
                    SPDLOG_INFO("added md {} [{:08x}]", source, md_location->uid);
                }
                uint32_t md_source = market_data_[source];
                SPDLOG_INFO("strategy subscribe from {} [{:08x}]", source, md_source);
                if (app_.get_writer(market_data_[source]).get() == nullptr)
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
                auto writer = app_.get_writer(source);
                char *buffer = const_cast<char *>(&(writer->open_frame(app_.now(), msg::type::Subscribe, 4096)->data<char>()));
                hffix::message_writer sub_msg(buffer, buffer + 4096);
                sub_msg.push_back_header("FIX.4.2");
                sub_msg.push_back_string(hffix::tag::MsgType, "V");
                sub_msg.push_back_int(hffix::tag::MDReqID, 1);
                sub_msg.push_back_int(hffix::tag::SubscriptionRequestType, 1); // Snapshot + Updates (Subscribe)
                sub_msg.push_back_int(hffix::tag::MarketDepth, 1);

                sub_msg.push_back_int(hffix::tag::NoMDEntryTypes, 1);
                sub_msg.push_back_int(hffix::tag::MDEntryType, 2);
                sub_msg.push_back_int(hffix::tag::NoRelatedSym, symbols.size());
                for (const auto &symbol : symbols)
                {
                    sub_msg.push_back_string(hffix::tag::Symbol, symbol);
                    sub_msg.push_back_string(hffix::tag::SecurityExchange, exchange);
                    quotes_[get_symbol_id(symbol, exchange)] = msg::data::Quote{};
                    SPDLOG_INFO("request subscribe {}", symbol);
                }
                sub_msg.push_back_trailer();
                writer->close_frame(sub_msg.message_end() - buffer);
            }

            uint64_t Context::insert_order(const OrderInput &input)
            {
                auto writer = app_.get_writer(lookup_account_location_id(std::string(input.account_id)));
                msg::data::OrderInput &data = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);
                memcpy(&data, &input, sizeof(OrderInput));
                data.order_id = writer->current_frame_uid();
                writer->close_data();
                return data.order_id;
            }

            uint64_t Context::insert_limit_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                                 double limit_price, int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                msg::data::OrderInput &input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceType::Limit;
                input.time_condition = TimeCondition::GFD;
                input.volume_condition = VolumeCondition::Any;

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_fak_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                               double limit_price, int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                msg::data::OrderInput &input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceType::Limit;
                input.time_condition = TimeCondition::IOC;
                input.volume_condition = VolumeCondition::Any;

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_fok_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                               double limit_price, int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                msg::data::OrderInput &input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

                input.order_id = writer->current_frame_uid();
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceType::Limit;
                input.time_condition = TimeCondition::IOC;
                input.volume_condition = VolumeCondition::All;

                writer->close_data();
                return input.order_id;
            }

            uint64_t Context::insert_market_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                                  int64_t volume, Side side, Offset offset)
            {
                auto writer = app_.get_writer(lookup_account_location_id(account));
                msg::data::OrderInput &input = writer->open_data<msg::data::OrderInput>(0, msg::type::OrderInput);

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
                    input.price_type = PriceType::Best5;
                    input.time_condition = TimeCondition::IOC;
                    input.volume_condition = VolumeCondition::Any;
                } else
                {
                    input.price_type = PriceType::Any;
                    input.time_condition = TimeCondition::IOC;
                    input.volume_condition = VolumeCondition::Any;
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
                msg::data::OrderAction &action = writer->open_data<msg::data::OrderAction>(0, msg::type::OrderAction);

                action.order_action_id = writer->current_frame_uid();
                action.order_id = order_id;
                action.action_flag = OrderActionFlag::Cancel;

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


            // context for backtest
            ContextBackTest::ContextBackTest(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events, std::vector<Strategy_ptr> strategies)
                : Context(app, events), strategies_(strategies), operation_count_(0)
                {}

            void ContextBackTest::react()
            {
                events_ | is(msg::type::Quote) |
                $([&](event_ptr event)
                  {
                      const Quote &quote = event->data<Quote>();
                      auto id = get_symbol_id(quote.get_instrument_id(), quote.get_exchange_id());
                      quotes_[id] = quote;
                      all_quotes_.push_back(quote);
                      //SPDLOG_INFO("quote recieved, instrument_id: {}, volume: {}, last_price: {} in backtest mode", quote.get_instrument_id(), quote.volume, quote.last_price);
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

                events_ | is(yijinjing::msg::type::SessionEnd) |
                $([&](event_ptr event)
                  {
                      SPDLOG_INFO("SessionEnd time: {} {}",event->gen_time(), all_quotes_.back().last_price);
                      for(auto strategy: strategies_)
                      {
                          strategy->pre_stop(shared_from_this());
                      }
                  });
            }

            void ContextBackTest::add_timer(int64_t nanotime, const std::function<void(yijinjing::event_ptr)> &callback)
            {
                SPDLOG_INFO("add_timer in BackTest Mode");
            }

            void ContextBackTest::add_time_interval(int64_t duration, const std::function<void(yijinjing::event_ptr)> &callback)
            {
                SPDLOG_INFO("add_time_interval in BackTest Mode");
            }

            void ContextBackTest::add_account(const std::string &source, const std::string &account, double cash_limit)
            {
                uint32_t account_id = yijinjing::util::hash_str_32(account);
                SPDLOG_INFO("added account {}@{} [{:08x}] in BackTest Mode", account, source, account_id);
            }

            void ContextBackTest::subscribe(const std::string &source, const std::vector<std::string> &symbols, const std::string &exchange)
            {
                for(auto symbol : symbols)
                {
                    quotes_[get_symbol_id(symbol, exchange)] = msg::data::Quote{};
                    SPDLOG_INFO("request subscribe {} in BackTest Mode", symbol);
                }
            }
            
            uint64_t ContextBackTest::insert_order(const msg::data::OrderInput &input)
            {
                //下单时需要维护一个order_id, TODO
                //由于没有TD，需要在此处就可调用strategy的on_order和on_trade
                SPDLOG_INFO("insert_order in BackTest Mode");
                return 0;
            }

            uint64_t ContextBackTest::insert_limit_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                        double limit_price, int64_t volume, Side side, Offset offset)
            {
                //SPDLOG_INFO("insert_limit_order in BackTest Mode");
                operation_count_ += 1;
                auto input = OrderInput();
                input.order_id = operation_count_;
                strcpy(input.instrument_id, symbol.c_str());
                strcpy(input.exchange_id, exchange.c_str());
                strcpy(input.account_id, account.c_str());
                input.limit_price = limit_price;
                input.frozen_price = limit_price;
                input.volume = volume;
                input.side = side;
                input.offset = offset;
                input.price_type = PriceType::Limit;
                input.time_condition = TimeCondition::GFD;
                input.volume_condition = VolumeCondition::Any;

                order_inputs_[operation_count_] = input;
                process_order(input);
                return input.order_id;
            }

            uint64_t ContextBackTest::insert_fok_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                        double limit_price, int64_t volume, Side side, Offset offset)
            {
                SPDLOG_INFO("insert_fok_order in BackTest Mode");
                return 2;
            }

            uint64_t ContextBackTest::insert_fak_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                        double limit_price, int64_t volume, Side side, Offset offset)
            {
                SPDLOG_INFO("insert_fak_order in BackTest Mode");
                return 3;
            }

            uint64_t ContextBackTest::insert_market_order(const std::string &symbol, const std::string &exchange, const std::string &account,
                                            int64_t volume, Side side, Offset offset)
            {
                //SPDLOG_INFO("insert_market_order in BackTest Mode, operation_count: {}", operation_count_);
                auto input = OrderInput();
                input.order_id = operation_count_;
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
                input.time_condition = TimeCondition::IOC;
                input.volume_condition = VolumeCondition::Any;
                if (strcmp(input.exchange_id, EXCHANGE_SSE) == 0 || strcmp(input.exchange_id, EXCHANGE_SZE) == 0) //沪深市，最优五档转撤销
                {
                    input.price_type = PriceType::Best5;
                } else
                {
                    input.price_type = PriceType::Any;
                }

                order_inputs_[input.order_id] = input;
                process_order(input);
                operation_count_ += 1;
                return input.order_id;
            }

            uint64_t ContextBackTest::cancel_order(uint64_t order_id)
            {
                //SPDLOG_INFO("cancel_order in BackTest Mode");
                return 5;
            }

            void ContextBackTest::process_order(const kungfu::wingchun::msg::data::OrderInput & input)
            {
                auto time_stamp = all_quotes_.back().data_time;
                auto order = Order();
                kungfu::wingchun::msg::data::order_from_input(input, order);
                order.insert_time = time_stamp;
                orders_[order.order_id] = order;
                //SPDLOG_INFO("order record recieved: volumn {}, price {}", order.volume, order.frozen_price);

            // inline void to_xtp(XTPOrderInsertInfo &des, const OrderInput &ori)
            // {
            //     strcpy(des.ticker, ori.instrument_id);
            //     to_xtp(des.market, ori.exchange_id);
            //     des.price = ori.limit_price;
            //     des.quantity = ori.volume;
            //     to_xtp(des.side, ori.side);
            //     to_xtp(des.price_type, ori.price_type, ori.time_condition, ori.volume_condition);
            //     des.business_type = XTP_BUSINESS_TYPE_CASH;
            // }

                auto trade = Trade();
                trade_from_input(input, trade);
                trade.trade_time = time_stamp;
                trades_[trade.order_id] = trade;

                //order update after trade
                orders_[trade.order_id].volume_traded = trade.volume;
                orders_[trade.order_id].volume_left = orders_[trade.order_id].volume - orders_[trade.order_id].volume_traded;
            // inline void from_xtp(const XTPTradeReport &ori, Trade &des)
            // {
            //     strcpy(des.instrument_id, ori.ticker);
            //     des.volume = ori.quantity;
            //     des.price = ori.price;
            //     from_xtp(ori.market, des.exchange_id);
            //     from_xtp(ori.side, des.side);
            //     des.offset = Offset::Open;
            //     if (ori.business_type == XTP_BUSINESS_TYPE_CASH)
            //     {
            //         des.instrument_type = InstrumentType::Stock;
            //     }
            //     from_xtp_timestamp(ori.trade_time, des.trade_time);
            // }

                for(auto strategy: strategies_)
                {
                    strategy->on_order(shared_from_this(), order);
                    strategy->on_trade(shared_from_this(), trade);
                }
            }

            void ContextBackTest::trade_from_input(const kungfu::wingchun::msg::data::OrderInput & input, kungfu::wingchun::msg::data::Trade &trade)
            {
                auto price_type = OrderType::UNKNOWN;

                if (input.price_type == PriceType::Limit && input.time_condition == TimeCondition::GFD) // 限价
                {
                    price_type = OrderType::LIMIT; // XTP_PRICE_LIMIT
                } else if (input.price_type == PriceType::Best && input.time_condition == TimeCondition::IOC) //成交剩余转撤销
                {
                    price_type = OrderType::MARKET; // XTP_PRICE_BEST_OR_CANCEL
                } else if (input.price_type == PriceType::Any && input.time_condition == TimeCondition::IOC) //成交剩余转撤销
                {
                    price_type = OrderType::MARKET; // XTP_PRICE_BEST_OR_CANCEL
                } else if (input.price_type == PriceType::Best5 && input.time_condition == TimeCondition::IOC) // 最优五档成交剩余转撤销
                {
                    price_type = OrderType::MARKET; // XTP_PRICE_BEST5_OR_CANCEL
                } else if (input.price_type == PriceType::Best5 && input.time_condition == TimeCondition::GFD) // 最优五档成交剩余转限价
                {
                    price_type = OrderType::MARKET; // XTP_PRICE_BEST5_OR_LIMIT
                } else if (input.price_type == PriceType::ForwardBest) //本方最优价
                {
                    price_type =  OrderType::MARKET; // XTP_PRICE_FORWARD_BEST
                } else if (input.price_type == PriceType::ReverseBest && input.time_condition == TimeCondition::GFD) //对方最优价
                {
                    price_type =  OrderType::MARKET; // XTP_PRICE_REVERSE_BEST_LIMIT
                }

                trade.order_id = input.order_id;
                strcpy(trade.instrument_id, input.instrument_id);
                strcpy(trade.exchange_id,input.exchange_id);
                trade.instrument_type = kungfu::wingchun::get_instrument_type(trade.instrument_id, trade.exchange_id);
                trade.side = input.side;
                trade.offset = input.offset; //Offset::Open
                trade.volume = input.volume;
                trade.price = input.limit_price;

                auto iter = quotes_.find(get_symbol_id(trade.instrument_id, trade.exchange_id));
                if (iter != quotes_.end())
                {
                    auto quote = iter->second;
                    volume_price_generator(trade, quote, price_type);
                    //SPDLOG_INFO("trade record recieved: volumn {}, price {}, trade type {}", trade.volume, trade.price, price_type);
                }
                else
                {
                    trade.volume = 0;
                    trade.price = 0;
                    SPDLOG_INFO("trade failed, invalid combination of instrument_id and exchange_id");
                }
            }
            
            void volume_price_generator(kungfu::wingchun::msg::data::Trade & trade, const kungfu::wingchun::msg::data::Quote & quote, const int price_type)
            {
                //SPDLOG_INFO("trade volume and price generating");
                auto volume_vec = trade.side == Side::Buy ? quote.get_ask_volume() : quote.get_bid_volume();
                auto price_vec = trade.side == Side::Buy ? quote.get_ask_price() : quote.get_bid_price();
                //auto level = price_type == OrderType::MARKET ? 5 : volume_vec.size(); //TODO
                //SPDLOG_INFO("volume_vec {} {} {} {} {} {} {} {} {} {}", volume_vec[0], volume_vec[1], volume_vec[2], volume_vec[3], volume_vec[4], volume_vec[5], volume_vec[6], volume_vec[7], volume_vec[8], volume_vec[9]);
                //SPDLOG_INFO("price_vec {} {} {} {} {} {} {} {} {} {}", price_vec[0], price_vec[1], price_vec[2], price_vec[3], price_vec[4], price_vec[5], price_vec[6], price_vec[7], price_vec[8], price_vec[9]);
                double value=0;
                int vol_sum=0;
                for(int i=0; i<volume_vec.size(); ++i)
                {
                    if(vol_sum + volume_vec[i] > trade.volume)
                    {
                        value += (trade.volume - vol_sum) * price_vec[i];
                        vol_sum = trade.volume;
                        break;
                    } else
                    {
                        value += volume_vec[i] * price_vec[i];
                        vol_sum += volume_vec[i];
                    }
                }
                trade.volume = vol_sum;
                trade.price = vol_sum == 0 ? 0 : value / vol_sum;
            }
        }
    }
}