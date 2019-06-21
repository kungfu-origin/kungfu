//
// Created by qlu on 2019/2/21.
//

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <hffix.hpp>
#include <nanomsg/pubsub.h>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log/setup.h>

#include <kungfu/wingchun/gateway/gateway_impl.h>

#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/serialize.h>

#include <kungfu/wingchun/util/filesystem_util.h>
#include <kungfu/wingchun/util/timer_util.h>
#include <kungfu/wingchun/gateway/util.hpp>

#include <kungfu/wingchun/storage/snapshot_storage.h>
#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/oms/def.h>

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
        GatewayImpl::GatewayImpl(bool low_latency, category c, const std::string &source, const std::string &name) :
                apprentice(std::make_shared<location>(mode::LIVE, c, source, name), low_latency),
                source_(source), name_(name), state_(GatewayState::Idle)
        {
            log::copy_log_settings(get_io_device()->get_home(), get_name());

            SPDLOG_DEBUG("created gateway {} with source {}", get_name(), get_source());
            calendar_ = Calendar_ptr(new Calendar(get_io_device()));
            std::string state_db_file = get_app_db_file("state");
            std::remove(state_db_file.c_str());
            state_storage_ = std::shared_ptr<GatewayStateStorage>(new GatewayStateStorage(state_db_file));
            nn_publisher_ = std::make_unique<NNPublisher>(get_io_device());
        }

        GatewayImpl::~GatewayImpl()
        {
            set_state(GatewayState::Idle);
        }

        void GatewayImpl::set_state(const enum GatewayState &state, const std::string &message)
        {
            state_ = state;

            GatewayStateRecord record = {};
            strcpy(record.name, this->name_.c_str());
            record.update_time = yijinjing::time::now_in_nano();
            strcpy(record.message, message.c_str());
            record.state = state;

            nlohmann::json data = record;
            get_publisher()->publish(msg::type::GatewayState, data);
            state_storage_->set_state(this->name_, record.update_time, state, message);

            if (state == GatewayState::Ready)
            {
                on_started();
            }
        }

        void GatewayImpl::react(observable<event_ptr> events)
        {
            apprentice::react(events);
        }

        MdGatewayImpl::MdGatewayImpl(bool low_latency, const std::string &source) :
                GatewayImpl(low_latency, category::MD, source, source)
        {

            std::string subscription_db_file = get_app_db_file("subscription");
            auto subscription_storage = std::make_shared<SubscriptionStorage>(subscription_db_file);
            register_subscription_storage(subscription_storage);
        }

        void MdGatewayImpl::react(observable<event_ptr> events)
        {
            GatewayImpl::react(events);

            events | is(msg::type::ReqLogin) |
            $([&](event_ptr event)
              {
//            auto data = event->data<journal::Quote>();
//            get_writer(event->source())->write(event->gen_time(), msg::type::RspLogin, &data);
              });

            events | is(msg::type::Subscribe) |
            $([&](event_ptr event)
              {
                  std::vector<std::string &> symbols{};
                  auto buffer = event->data<char *>();
                  hffix::message_reader reader(buffer, buffer + event->data_length());
                  for (; reader.is_complete(); reader = reader.next_message_reader())
                  {
                      if (reader.is_valid())
                      {
                          hffix::message_reader::const_iterator i = reader.begin();
                          if (reader.find_with_hint(hffix::tag::Symbol, i))
                          {
                              symbols.push_back(i++->value());
                          }
                      }
                  }
                  subscribe_market_data(symbols);
              });

            init();
        }

        TdGatewayImpl::TdGatewayImpl(bool low_latency, const std::string &source, const std::string &name) :
                GatewayImpl(low_latency, category::TD, source, name)
        {}

        void TdGatewayImpl::react(observable<event_ptr> events)
        {
            GatewayImpl::react(events);
            init();
        }

        void TdGatewayImpl::configure_event_source(yijinjing::event_source_ptr event_source)
        {
            GatewayImpl::configure_event_source(event_source);

            order_manager_ = oms::create_order_manager();

            init_account_manager();

            int worker_id = UidWorkerStorage::get_instance(UID_WORKER_DB_FILE)->get_uid_worker_id(get_name());
            if (worker_id <= 0)
            {
                UidWorkerStorage::get_instance(UID_WORKER_DB_FILE)->add_uid_worker(get_name());
                worker_id = UidWorkerStorage::get_instance(UID_WORKER_DB_FILE)->get_uid_worker_id(get_name());
            }
            uid_generator_ = std::unique_ptr<UidGenerator>(new UidGenerator(worker_id, UID_EPOCH_SECONDS));

            event_source_->observe(
                    std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::MD, get_source(),
                                                                get_source(),
                                                                event_source_->get_io_device()->get_home()->locator));


            auto order_storage = std::make_shared<storage::OrderStorage>(get_app_db_file("orders"));
            register_order_storage(order_storage);

            auto trade_storage = std::make_shared<storage::TradeStorage>(get_app_db_file("trades"));
            register_trade_storage(trade_storage);

            register_nanotime_callback(yijinjing::time::next_minute_nano(yijinjing::time::now_in_nano()),
                                       std::bind(&TdGatewayImpl::on_1min_timer, this, std::placeholders::_1));
            register_nanotime_callback(yijinjing::time::next_day_nano(yijinjing::time::now_in_nano()),
                                       std::bind(&TdGatewayImpl::on_daily_timer, this, std::placeholders::_1));
            register_req_login_callback(
                    std::bind(&TdGatewayImpl::on_login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            register_order_input_callback(std::bind(&TdGatewayImpl::on_order_input, this, std::placeholders::_1));
            register_order_action_callback(std::bind(&TdGatewayImpl::on_order_action, this, std::placeholders::_1));
            register_quote_callback(std::bind(&TdGatewayImpl::on_quote, this, std::placeholders::_1));
            register_manual_order_input_callback(std::bind(&TdGatewayImpl::on_manual_order_input, this, std::placeholders::_1));
            register_manual_order_action_callback(
                    std::bind(&TdGatewayImpl::on_manual_order_action, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            register_switch_day_callback(std::bind(&TdGatewayImpl::on_switch_day, this, std::placeholders::_1));

            nn_publisher_ = std::make_unique<NNPublisher>(event_source_);

            account_manager_->register_pos_callback(std::bind(&NNPublisher::publish_pos, (NNPublisher *) get_publisher(), std::placeholders::_1));
            account_manager_->register_acc_callback(
                    std::bind(&NNPublisher::publish_account_info, (NNPublisher *) get_publisher(), std::placeholders::_1, msg::type::AccountInfo));

            init();
        }

        void TdGatewayImpl::on_started()
        {
            account_manager_->dump_to_db(get_app_db_file("asset"), true);
            add_market_feed(this->get_source());
            subscribe_holdings();
        }

        void TdGatewayImpl::init_account_manager()
        {
            AccountType account_type = get_account_type();
            std::string asset_db_file = get_app_db_file("asset");

            account_manager_ = create_account_manager(event_source_, this->get_account_id().c_str(), account_type, asset_db_file);

            int64_t last_update = account_manager_->get_last_update();
            if (last_update > 0)
            {
                auto io = event_source_->get_io_device();
                auto reader = io->open_reader_to_subscribe();
                reader->join(
                        std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::MD, get_source(),
                                                                    get_source(), io->get_home()->locator), 0, last_update);
                reader->join(
                        std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::TD, get_source(),
                                                                    get_account_id(), io->get_home()->locator), 0, last_update);
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
                                account_manager_->on_quote(&quote);
                            }
                            break;
                        }
                        case msg::type::Order:
                        {
                            auto order = frame->data<msg::data::Order>();
                            if (order.rcv_time > last_update)
                            {
                                account_manager_->on_order(&order);
                            }
                            break;
                        }
                        case msg::type::Trade:
                        {
                            auto trade = frame->data<msg::data::Trade>();
                            if (trade.rcv_time > last_update)
                            {
                                account_manager_->on_trade(&trade);
                            }
                            break;
                        }
                        case msg::type::AccountInfo:
                        {
                            auto account = frame->data<msg::data::AccountInfo>();
                            if (account.rcv_time > last_update)
                            {
                                account_manager_->on_account(account);
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
                            yijinjing::time::strftime(account_manager_->get_last_update(), "%Y%m%d-%H:%M:%S"));
            }

            account_manager_->set_current_trading_day(get_calendar()->get_current_trading_day());
            account_manager_->dump_to_db(asset_db_file, true);
            SPDLOG_INFO("account_manager inited and set to {}", account_manager_->get_current_trading_day());

        }

        uint64_t TdGatewayImpl::next_id()
        {
            int64_t seconds = yijinjing::time::now_in_nano() / yijinjing::time_unit::NANOSECONDS_PER_SECOND;
            return uid_generator_->next_id(seconds);
        }

        void TdGatewayImpl::subscribe_holdings() const
        {
            std::vector<Instrument> instruments = account_manager_->get_all_pos_instruments();
            SPDLOG_TRACE(fmt::format("holding {} insts, sending subscribe request", instruments.size()));
            gateway::subscribe(event_source_->get_io_device(), get_source(), instruments, false, this->get_name());
        }

        void TdGatewayImpl::on_login(const std::string &source, const std::string &name, const std::string &client_id)
        {
            SPDLOG_TRACE("login from client {} source {} name {}, this source {} account_id {}", client_id, source, name, get_source(),
                         get_account_id());
            if (source == get_source() && name == get_account_id())
            {
                auto locator = event_source_->get_io_device()->get_home()->locator;
                event_source_->observe(
                        std::make_shared<yijinjing::data::location>(yijinjing::data::mode::LIVE, yijinjing::data::category::STRATEGY, client_id,
                                                                    client_id, locator));

                gateway::GatewayLoginRsp rsp = {};
                rsp.state = this->get_state();
                rsp.type = this->get_account_type();
                NNMsg msg = {};
                msg.msg_type = msg::type::RspLogin;
                msg.data = rsp;
                std::string js = to_string(msg);
                SPDLOG_TRACE("sending {} ", js);
//            event_source_->get_socket_reply()->send(js, 0);
            } else
            {
                SPDLOG_TRACE("wrong login from client {} source {} name {}, this source {} account_id {}", client_id, source, name, get_source(),
                             get_account_id());
            }
        }

        void TdGatewayImpl::on_quote(const Quote &quote)
        {
            QUOTE_TRACE(to_string(quote));
            account_manager_->on_quote(&quote);
        }

        std::vector<uint64_t> TdGatewayImpl::get_pending_orders(const std::string &client_id) const
        {
            std::vector<uint64_t> order_ids;
            auto orders = order_manager_->get_pending_orders();
            for (const auto &order : orders)
            {
                auto simple_order = std::dynamic_pointer_cast<oms::SimpleOrder>(order);
                if (nullptr != simple_order)
                {
                    if (client_id.empty() || client_id == simple_order->get_client_id())
                    {
                        order_ids.emplace_back(simple_order->get_order_id());
                    }
                }
            }
            return order_ids;
        }

        bool TdGatewayImpl::add_market_feed(const std::string &source_name)
        {
            gateway::add_market_feed(event_source_->get_io_device(), source_name, this->get_name());
            return true;
        }

        void TdGatewayImpl::on_order_input(const OrderInput &order_input)
        {
            if (order_input.account_id == this->get_account_id())
            {
                insert_order(order_input);
            }
        }

        void TdGatewayImpl::on_order_action(const OrderAction &order_action)
        {
            cancel_order(order_action);
        }

        void TdGatewayImpl::on_manual_order_input(journal::OrderInput &order_input)
        {
            int error_id = 0;
            std::string error_text = "";
            uint64_t order_id = 0;

            if (strcmp(get_account_id().c_str(), order_input.account_id) == 0)
            {
                order_id = next_id();
                auto type = get_account_type();

                order_input.order_id = order_id;
                strcpy(order_input.exchange_id, (type == AccountTypeFuture ? get_exchange_id_from_future_instrument_id(order_input.instrument_id)
                                                                           : get_exchange_id_from_stock_instrument_id(
                                order_input.instrument_id)).c_str());
                order_input.instrument_type = get_instrument_type(order_input.instrument_id, order_input.exchange_id);
                order_input.frozen_price = order_input.limit_price;
                order_input.volume_condition = VolumeConditionAny;
                order_input.time_condition = TimeConditionGFD;

                insert_order(order_input);
            } else
            {
                error_id = -1;
                error_text = "account id not matched";
            }

            NNMsg msg = {msg::type::RspOrderInput, {}};
            msg.data["error_id"] = error_id;
            msg.data["error_text"] = error_text;
            msg.data["order_id"] = std::to_string(order_id);
            std::string js = to_string(msg);
//        event_source_->get_socket_reply()->send(js, 0);
        }

        void
        TdGatewayImpl::on_manual_order_action(const std::string &account_id, const std::string &client_id, const std::vector<uint64_t> &order_ids)
        {
            int error_id = 0;
            std::string error_text = "";
            int cancel_count = 0;
            if (!order_ids.empty())
            {
                for (const auto &order_id : order_ids)
                {
                    OrderAction order_action{order_id, 0, OrderActionFlagCancel, 0, 0};
                    cancel_order(order_action);
                }
                cancel_count = order_ids.size();
            } else
            {
                auto pending_orders = get_pending_orders(client_id);
                for (const auto &order_id : pending_orders)
                {
                    OrderAction order_action{order_id, 0, OrderActionFlagCancel, 0, 0};
                    cancel_order(order_action);
                }
                cancel_count = pending_orders.size();
            }

            NNMsg msg = {msg::type::RspOrderAction, {}};
            msg.data["error_id"] = error_id;
            msg.data["error_text"] = error_text;
            msg.data["cancel_count"] = cancel_count;
            std::string js = to_string(msg);
//        event_source_->get_socket_reply()->send(js, 0);
        }

        void TdGatewayImpl::on_order(Order & order)
        {
            ORDER_TRACE(journal::to_string(order));
            feed_handler_->on_order(&order);
            account_manager_->on_order(&order);
            order_manager_->on_order(&order);
            get_publisher()->publish_order(order);
            order_storage_->add_order(order.order_id, order);
        }

        void TdGatewayImpl::on_trade(Trade & trade)
        {
            TRADE_TRACE(journal::to_string(trade));
            double commission = account_manager_->calc_commission(&trade);
            double tax = account_manager_->calc_tax(&trade);
            trade.commission = commission;
            trade.tax = tax;

            feed_handler_->on_trade(&trade);
            account_manager_->on_trade(&trade);
            account_manager_->dump_to_db(get_app_db_file("asset"), true);

            auto trade_id = trade_storage_->add_trade(trade);
            trade.id = trade_id;
            get_publisher()->publish_trade(trade);
        }

        void TdGatewayImpl::on_position(const Position &pos, bool is_last)
        {
            POSITION_TRACE(flying::to_string(pos));
            feed_handler_->on_position(&pos);
            get_publisher()->publish_pos(pos);

            rsp_pos_.emplace_back(pos);
            if (is_last)
            {
                account_manager_->on_positions(rsp_pos_);
                rsp_pos_.clear();
            }
        }

        void TdGatewayImpl::on_position_detail(const Position &pos_detail, bool is_last)
        {
            POSITION_DETAIL_TRACE(flying::to_string(pos_detail));
            feed_handler_->on_position_detail(&pos_detail);
            rsp_pos_detail_.push_back(pos_detail);
            if (is_last)
            {
                account_manager_->on_position_details(rsp_pos_detail_);
                rsp_pos_detail_.clear();
            }
        }

        void TdGatewayImpl::on_account(AccountInfo & account)
        {
            strcpy(account.trading_day, get_calendar()->get_current_trading_day().c_str());
            ACCOUNT_TRACE(flying::to_string(account));
            feed_handler_->on_account(&account);
            account_manager_->on_account(account);
        }

        void TdGatewayImpl::on_1min_timer(int64_t nano)
        {
            auto account_info = account_manager_->get_account_info();
            bool is_open = calendar_->is_open(nano, account_info.account_type == AccountTypeFuture ? EXCHANGE_SHFE : EXCHANGE_SSE);
            TIMER_TRACE(fmt::format("[on_1min_timer] (nano) {} (is_open) {}", nano, is_open));
            if (is_open)
            {
                account_info.update_time = (int64_t) std::round((double) yijinjing::time::now_in_nano() / 1000000000) * 1000000000;
                get_publisher()->publish_account_info(account_info, msg::type::AccountInfoByMin);
                DUMP_1M_SNAPSHOT(this->get_account_id(), account_info);
                storage::SnapshotStorage(get_app_db_file("snapshots"), ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME, false, true).insert(account_info)
            }
            register_nanotime_callback(nano + yijinjing::time_unit::NANOSECONDS_PER_MINUTE,
                                       std::bind(&TdGatewayImpl::on_1min_timer, this, std::placeholders::_1));
        }

        void TdGatewayImpl::on_daily_timer(int64_t nano)
        {
            TIMER_TRACE(fmt::format("[on_daily_timer] (nano) {}", nano));
            auto account_info = account_manager_->get_account_info();
            account_info.update_time = (int64_t) std::round((double) yijinjing::time::now_in_nano() / 1000000000) * 1000000000;
            DUMP_1D_SNAPSHOT(this->get_account_id(), account_info);
            storage::SnapshotStorage(get_app_db_file("snapshots"), ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME, true, true).insert(account_info)
            register_nanotime_callback(nano + yijinjing::time_unit::NANOSECONDS_PER_DAY,
                                       std::bind(&TdGatewayImpl::on_daily_timer, this, std::placeholders::_1));
        }

        void TdGatewayImpl::on_switch_day(const std::string &trading_day)
        {
            if (nullptr != account_manager_)
            {
                account_manager_->on_switch_day(trading_day);
                account_manager_->dump_to_db(get_app_db_file("asset"), true);
            }
        }
    }
}
