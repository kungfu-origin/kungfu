//
// Created by qlu on 2019/2/21.
//

#include <cstring>
#include <cstdio>
#include <algorithm>

#include <nanomsg/pubsub.h>
#include <kungfu/yijinjing/nanomsg/socket.h>

#include <kungfu/wingchun/gateway/gateway_impl.h>

#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/serialize.h>

#include <kungfu/wingchun/util/env.h>
#include <kungfu/wingchun/util/filesystem_util.h>
#include <kungfu/wingchun/util/timer_util.h>
#include <kungfu/wingchun/gateway/util.hpp>

#include <kungfu/wingchun/storage/log.h>
#include <kungfu/wingchun/storage/snapshot_storage.h>
#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/oms/def.h>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
#define DUMP_1D_SNAPSHOT(account_id, account_info) kungfu::storage::SnapshotStorage(\
    ACCOUNT_SNAPSHOT_DB_FILE(account_id), ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME, true, true).insert(account_info)

#define DUMP_1M_SNAPSHOT(account_id, account_info) kungfu::storage::SnapshotStorage(\
    ACCOUNT_SNAPSHOT_DB_FILE(account_id), ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME, false, true).insert(account_info)

    GatewayImpl::GatewayImpl(const std::string &source, const std::string &name) 
            : source_(source), name_(name), state_(GatewayState::Idle)
    {
        kungfu::log::LogConfig::setup_log(name);
        logger_ = spdlog::default_logger();
        SPDLOG_INFO("created gateway {} with source {}", name, source);
    }

    GatewayImpl::~GatewayImpl()
    {
        set_state(GatewayState::Idle);
    }

    void GatewayImpl::init()
    {
        calendar_ = CalendarPtr(new Calendar());
        kungfu::calendar_util::set_logger(get_logger());

        loop_ = std::shared_ptr<EventLoop>(new EventLoop(get_name()));
        loop_->set_logger(get_logger());

        if (!create_folder_if_not_exists(GATEWAY_FOLDER(this->get_name())))
        {
            SPDLOG_ERROR("failed to create gateway folder {}", GATEWAY_FOLDER(this->get_name()));
            abort();
        }

        std::string rep_url = GATEWAY_REP_URL(get_name());
        rsp_socket_ = std::shared_ptr<yijinjing::nanomsg::socket>(new yijinjing::nanomsg::socket(AF_SP, NN_REP));
        try
        {
            rsp_socket_->bind(rep_url.c_str());
        }
        catch(std::exception &e)
        {
            SPDLOG_ERROR("failed to bind to rep_url {}, exception: {}", rep_url.c_str(), e.what());
            abort();
        }
        SPDLOG_INFO("gateway bind to rep url {}", rep_url);
        loop_->add_socket(rsp_socket_);

        std::string state_db_file = GATEWAY_STATE_DB_FILE(get_name());
#ifdef  _WINDOWS
        std::replace(state_db_file.begin(), state_db_file.end(), '/', '\\');
#endif
        std::remove(state_db_file.c_str());
        state_storage_ =  std::shared_ptr<GatewayStateStorage>(new GatewayStateStorage(state_db_file));

        std::string url = GATEWAY_PUB_URL(name_);
        nn_publisher_ = std::unique_ptr<NNPublisher>(new NNPublisher(url));
        nn_publisher_->set_logger(get_logger());
        SPDLOG_INFO("gateway bind to pub url {}", url);
    }

    void GatewayImpl::start()
    {
        std::thread loop_thread(std::bind(&kungfu::EventLoop::run, this->loop_.get()));
        loop_thread.join();
    }

    void GatewayImpl::stop()
    {
        loop_->stop();
    }

    void GatewayImpl::set_state(const GatewayState& state, const std::string& message)
    {
        state_ = state;

        GatewayStateRecord record = {};
        strcpy(record.name, this->name_.c_str());
        record.update_time = kungfu::yijinjing::getNanoTime();
        strcpy(record.message, message.c_str());
        record.state = state;

        nlohmann::json data = record;
        get_publisher()->publish(kungfu::MsgType::GatewayState, data);
        state_storage_->set_state(this->name_, record.update_time, state, message);

        if (state == GatewayState::Ready)
        {
            on_started();
        }
    }

    void MdGatewayImpl::init()
    {
        GatewayImpl::init();

        std::string journal_folder = MD_JOURNAL_FOLDER(get_source());
        kungfu::yijinjing::JournalWriterPtr writer = kungfu::yijinjing::JournalWriter::create(journal_folder, MD_JOURNAL_NAME(get_source()), this->get_name());
        std::shared_ptr<kungfu::MarketDataStreamingWriter> feed_handler = std::shared_ptr<kungfu::MarketDataStreamingWriter>(new kungfu::MarketDataStreamingWriter(writer));
        register_feed_handler(feed_handler);

        std::string subscription_db_file = SUBSCRIPTION_DB_FILE(get_name());
#ifdef _WINDOWS
        std::replace(subscription_db_file.begin(), subscription_db_file.end(), '/', '\\');
#endif
        std::shared_ptr<kungfu::SubscriptionStorage> subscription_storage = std::shared_ptr<kungfu::SubscriptionStorage>(new kungfu::SubscriptionStorage(subscription_db_file));
        register_subscription_storage(subscription_storage);

        loop_->register_req_login_callback(std::bind(&MdGatewayImpl::on_login, this, std::placeholders::_1, std::placeholders::_2));
        loop_->register_subscribe_callback(std::bind(&MdGatewayImpl::on_subscribe, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void MdGatewayImpl::on_login(const std::string &recipient, const std::string &client_id)
    {
        SPDLOG_TRACE("(recipient) {} (client) {}", recipient, client_id);
        if (recipient == this->get_name())
        {
            gateway::GatewayLoginRsp rsp = {};
            rsp.state = this->get_state();

            NNMsg msg = {};
            msg.msg_type = kungfu::MsgType::RspLogin;
            msg.data = rsp;
            std::string js = to_string(msg);
            get_rsp_socket()->send(js, 0);
            SPDLOG_TRACE("login sent {} ", js);
        }
    }

    void MdGatewayImpl::on_quote(const Quote &quote)
    {
        QUOTE_TRACE(kungfu::journal::to_string(quote));
        feed_handler_->on_quote(&quote);
    }

    void MdGatewayImpl::on_entrust(const Entrust &entrust)
    {
        ENTRUST_TRACE(kungfu::journal::to_string(entrust));
        feed_handler_->on_entrust(&entrust);
    }

    void MdGatewayImpl::on_transaction(const Transaction &transaction)
    {
        TRANSACTION_TRACE(kungfu::journal::to_string(transaction));
        feed_handler_->on_transaction(&transaction);
    }

    void MdGatewayImpl::on_subscribe(const std::string &recipient, const std::vector<Instrument> &instruments, bool is_level2 = false)
    {
        SPDLOG_TRACE("(recipient) {} (size) {} (is_level2) {}", recipient, instruments.size(), is_level2);
        if (recipient == this->get_name())
        {
            this->subscribe(instruments, is_level2);
            SubscribeRsp rsp = {};
            NNMsg msg = {};
            msg.msg_type = kungfu::MsgType::RspSubscribe;
            msg.data = rsp;
            std::string js = to_string(msg);
            SPDLOG_TRACE("sending {} ", js);
            get_rsp_socket()->send(js, 0);
        }
    }

    TdGatewayImpl::~TdGatewayImpl() {}

    void TdGatewayImpl::init()
    {
        GatewayImpl::init();

        order_manager_ = oms::create_order_manager();

        storage::set_logger(get_logger());
        portfolio_util::set_logger(get_logger());

        if (!create_folder_if_not_exists(ACCOUNT_FOLDER(this->get_account_id())))
        {
            SPDLOG_ERROR("failed to create account folder {}", ACCOUNT_FOLDER(this->get_account_id()));
        }

        int worker_id = UidWorkerStorage::get_instance(UID_WORKER_DB_FILE)->get_uid_worker_id(get_name());
        if (worker_id <= 0)
        {
            UidWorkerStorage::get_instance(UID_WORKER_DB_FILE)->add_uid_worker(get_name());
            worker_id = UidWorkerStorage::get_instance(UID_WORKER_DB_FILE)->get_uid_worker_id(get_name());
        }
        uid_generator_ = std::unique_ptr<UidGenerator>(new UidGenerator(worker_id, UID_EPOCH_SECONDS));

        init_account_manager();

        loop_->subscribe_yjj_journal(MD_JOURNAL_FOLDER(get_source()), MD_JOURNAL_NAME(get_source()), kungfu::yijinjing::getNanoTime());
        std::string journal_folder = TD_JOURNAL_FOLDER(get_source(), get_account_id());
        kungfu::yijinjing::JournalWriterPtr writer = kungfu::yijinjing::JournalWriter::create(journal_folder, TD_JOURNAL_NAME(get_source(), get_account_id()), this->get_name());
        std::shared_ptr<kungfu::TraderDataFeedHandler> feed_handler = std::shared_ptr<kungfu::TraderDataFeedHandler>(new kungfu::TraderDataStreamingWriter(writer));
        register_feed_handler(feed_handler);

        std::shared_ptr<kungfu::storage::OrderStorage> order_storage = std::shared_ptr<kungfu::storage::OrderStorage>(new kungfu::storage::OrderStorage(ORDER_DB_FILE(get_account_id())));
        register_order_storage(order_storage);

        std::shared_ptr<kungfu::storage::TradeStorage> trade_storage = std::shared_ptr<kungfu::storage::TradeStorage>(new kungfu::storage::TradeStorage(TRADE_DB_FILE(get_account_id())));
        register_trade_storage(trade_storage);

        loop_->register_nanotime_callback(nseconds_next_min(yijinjing::getNanoTime()), std::bind(&TdGatewayImpl::on_1min_timer, this, std::placeholders::_1));
        loop_->register_nanotime_callback(nseconds_next_day(yijinjing::getNanoTime()), std::bind(&TdGatewayImpl::on_daily_timer, this, std::placeholders::_1));
        loop_->register_req_login_callback(std::bind(&TdGatewayImpl::on_login, this, std::placeholders::_1, std::placeholders::_2));
        loop_->register_order_input_callback(std::bind(&TdGatewayImpl::on_order_input, this, std::placeholders::_1));
        loop_->register_order_action_callback(std::bind(&TdGatewayImpl::on_order_action, this, std::placeholders::_1));
        loop_->register_quote_callback(std::bind(&TdGatewayImpl::on_quote, this, std::placeholders::_1));
        loop_->register_manual_order_input_callback(std::bind(&TdGatewayImpl::on_manual_order_input, this, std::placeholders::_1));
        loop_->register_manual_order_action_callback(std::bind(&TdGatewayImpl::on_manual_order_action, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        calendar_->register_switch_day_callback(std::bind(&TdGatewayImpl::on_switch_day, this, std::placeholders::_1));
    }

    void TdGatewayImpl::on_started()
    {
        account_manager_->dump_to_db(ACCOUNT_ASSET_DB_FILE(this->get_account_id()).c_str(), true);
        add_market_feed(this->get_source());
        subscribe_holdings();
    }

    void TdGatewayImpl::init_account_manager()
    {
        AccountType account_type = get_account_type();
        std::string asset_db_file = ACCOUNT_ASSET_DB_FILE(this->get_account_id());

        account_manager_ = create_account_manager(this->get_account_id().c_str(), account_type, asset_db_file.c_str());

        int64_t last_update = account_manager_->get_last_update();
        if (last_update > 0)
        {
            std::vector<std::string> folders;
            std::vector<std::string> names;
            folders.push_back(TD_JOURNAL_FOLDER(get_source(), get_account_id()));
            names.emplace_back(TD_JOURNAL_NAME(get_source(), get_account_id()));
            folders.push_back(MD_JOURNAL_FOLDER(get_source()));
            names.emplace_back(MD_JOURNAL_NAME(get_source()));
            kungfu::yijinjing::JournalReaderPtr reader = kungfu::yijinjing::JournalReader::create(folders, names, last_update);
            kungfu::yijinjing::FramePtr frame = reader->getNextFrame();
            while (frame != nullptr)
            {
                int msg_type = frame->getMsgType();
                switch (msg_type)
                {
                    case (int) MsgType::Quote:
                    {
                        auto quote = (const Quote*) frame->getData();
                        if (quote->rcv_time > last_update)
                        {
                            account_manager_->on_quote(quote);
                        }
                        break;
                    }
                    case (int) MsgType::Order:
                    {
                        auto order = (const Order*) frame->getData();
                        if (order->rcv_time > last_update)
                        {
                            account_manager_->on_order(order);
                        }
                        break;
                    }
                    case (int) MsgType::Trade:
                    {
                        auto trade = (const Trade*) frame->getData();
                        if (trade->rcv_time > last_update)
                        {
                            account_manager_->on_trade(trade);
                        }
                        break;
                    }
                    case (int) MsgType::AccountInfo:
                    {
                        auto account = (const AccountInfo*)frame->getData();
                        if (account->rcv_time > last_update)
                        {
                            account_manager_->on_account(*account);
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                frame = reader->getNextFrame();
            }
            SPDLOG_INFO("forward account manager from {}|{} to {}|{}", last_update, kungfu::yijinjing::parseNano(last_update, "%Y%m%d-%H:%M:%S"), account_manager_->get_last_update(), kungfu::yijinjing::parseNano(account_manager_->get_last_update(), "%Y%m%d-%H:%M:%S"));
        }

        account_manager_->set_current_trading_day(get_calendar()->get_current_trading_day());
        account_manager_->dump_to_db(asset_db_file.c_str(), true);
        SPDLOG_INFO("account_manager inited and set to {}", account_manager_->get_current_trading_day());

        account_manager_->register_pos_callback(std::bind(&NNPublisher::publish_pos, (NNPublisher*)get_publisher(), std::placeholders::_1));
        account_manager_->register_acc_callback(std::bind(&NNPublisher::publish_account_info, (NNPublisher*)get_publisher(), std::placeholders::_1, MsgType::AccountInfo));

    }

    uint64_t TdGatewayImpl::next_id()
    {
        int64_t seconds = kungfu::yijinjing::getNanoTime() / kungfu::yijinjing::NANOSECONDS_PER_SECOND;
        return uid_generator_->next_id(seconds);
    }

    void TdGatewayImpl::subscribe_holdings() const
    {
        std::vector<Instrument> instruments = account_manager_->get_all_pos_instruments();
        SPDLOG_TRACE(fmt::format("holding {} insts, sending subscribe request", instruments.size()));
        gateway::subscribe(get_source(), instruments, false, this->get_name());
    }

    void TdGatewayImpl::on_login(const std::string &recipient, const std::string &client_id)
    {
        SPDLOG_TRACE("(recipient) {} (client) {}", recipient, client_id);
        if (recipient == get_name())
        {
            loop_->subscribe_yjj_journal(STRATEGY_JOURNAL_FOLDER, client_id, kungfu::yijinjing::getNanoTime());

            gateway::GatewayLoginRsp rsp = {};
            rsp.state = this->get_state();
            rsp.type =  this->get_account_type();
            NNMsg msg = {};
            msg.msg_type = kungfu::MsgType::RspLogin;
            msg.data = rsp;
            std::string js = to_string(msg);
            SPDLOG_TRACE("sending {} ", js);
            rsp_socket_->send(js, 0);
        }
    }

    void TdGatewayImpl::on_quote(const Quote &quote)
    {
        QUOTE_TRACE(to_string(quote));
        account_manager_->on_quote(&quote);
    }

    std::vector<uint64_t> TdGatewayImpl::get_pending_orders(const string &client_id) const
    {
        std::vector<uint64_t> order_ids;
        auto orders = order_manager_->get_pending_orders();
        for (const auto& order : orders)
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
        gateway::add_market_feed(source_name, this->get_name());
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

    void TdGatewayImpl::on_manual_order_input(kungfu::journal::OrderInput &order_input)
    {
        int error_id = 0;
        std::string error_text = "";
        uint64_t order_id = 0;

        if (strcmp(get_account_id().c_str(), order_input.account_id) == 0)
        {
            order_id = next_id();
            auto type = get_account_type();

            order_input.order_id = order_id;
            strcpy(order_input.exchange_id, (type == AccountTypeFuture ? get_exchange_id_from_future_instrument_id(order_input.instrument_id) : get_exchange_id_from_stock_instrument_id(order_input.instrument_id)).c_str());
            order_input.instrument_type = get_instrument_type(order_input.instrument_id, order_input.exchange_id);
            order_input.frozen_price = order_input.limit_price;
            order_input.volume_condition = VolumeConditionAny;
            order_input.time_condition = TimeConditionGFD;

            insert_order(order_input);
        }
        else
        {
            error_id = -1;
            error_text = "account id not matched";
        }

        NNMsg msg = {MsgType::RspOrderInput, {}};
        msg.data["error_id"] = error_id;
        msg.data["error_text"] = error_text;
        msg.data["order_id"] = std::to_string(order_id);
        std::string js = to_string(msg);
        rsp_socket_->send(js, 0);
    }

    void TdGatewayImpl::on_manual_order_action(const std::string &account_id, const std::string &client_id, const std::vector<uint64_t> &order_ids)
    {
        int error_id = 0;
        std::string error_text = "";
        int cancel_count = 0;
        if (!order_ids.empty())
        {
            for (const auto& order_id : order_ids)
            {
                OrderAction order_action{order_id, 0, OrderActionFlagCancel, 0, 0};
                cancel_order(order_action);
            }
            cancel_count = order_ids.size();
        }
        else
        {
            auto pending_orders = get_pending_orders(client_id);
            for (const auto& order_id : pending_orders)
            {
                OrderAction order_action{order_id, 0, OrderActionFlagCancel, 0, 0};
                cancel_order(order_action);
            }
            cancel_count = pending_orders.size();
        }

        NNMsg msg = {MsgType::RspOrderAction, {}};
        msg.data["error_id"] = error_id;
        msg.data["error_text"] = error_text;
        msg.data["cancel_count"] = cancel_count;
        std::string js = to_string(msg);
        rsp_socket_->send(js, 0);
    }

    void TdGatewayImpl::on_order(Order &order)
    {
        ORDER_TRACE(kungfu::journal::to_string(order));
        feed_handler_->on_order(&order);
        account_manager_->on_order(&order);
        order_manager_->on_order(&order);
        get_publisher()->publish_order(order);
        order_storage_->add_order(order.order_id, order);
    }

    void TdGatewayImpl::on_trade(Trade &trade)
    {
        TRADE_TRACE(kungfu::journal::to_string(trade));
        double commission = account_manager_->calc_commission(&trade);
        double tax = account_manager_->calc_tax(&trade);
        trade.commission = commission;
        trade.tax = tax;

        feed_handler_->on_trade(&trade);
        account_manager_->on_trade(&trade);
        account_manager_->dump_to_db(ACCOUNT_ASSET_DB_FILE(this->get_account_id()).c_str(), true);

        auto trade_id = trade_storage_->add_trade(trade);
        trade.id = trade_id;
        get_publisher()->publish_trade(trade);
    }

    void TdGatewayImpl::on_position(const Position &pos, bool is_last)
    {
        POSITION_TRACE(kungfu::flying::to_string(pos));
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
        POSITION_DETAIL_TRACE(kungfu::flying::to_string(pos_detail));
        feed_handler_->on_position_detail(&pos_detail);
        rsp_pos_detail_.push_back(pos_detail);
        if (is_last)
        {
            account_manager_->on_position_details(rsp_pos_detail_);
            rsp_pos_detail_.clear();
        }
    }

    void TdGatewayImpl::on_account(AccountInfo& account)
    {
        strcpy(account.trading_day, get_calendar()->get_current_trading_day().c_str());
        ACCOUNT_TRACE( kungfu::flying::to_string(account));
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
            account_info.update_time = (int64_t)std::round((double)yijinjing::getNanoTime() / 1000000000) * 1000000000;
            get_publisher()->publish_account_info(account_info, MsgType::AccountInfoByMin);
            DUMP_1M_SNAPSHOT(this->get_account_id(), account_info);
        }
        loop_->register_nanotime_callback(nano + kungfu::yijinjing::NANOSECONDS_PER_MINUTE, std::bind(&TdGatewayImpl::on_1min_timer, this, std::placeholders::_1));
    }

    void TdGatewayImpl::on_daily_timer(int64_t nano)
    {
        TIMER_TRACE(fmt::format("[on_daily_timer] (nano) {}", nano));
        auto account_info = account_manager_->get_account_info();
        account_info.update_time = (int64_t)std::round((double)yijinjing::getNanoTime() / 1000000000) * 1000000000;
        DUMP_1D_SNAPSHOT(this->get_account_id(), account_info);
        loop_->register_nanotime_callback(nano + yijinjing::NANOSECONDS_PER_DAY, std::bind(&TdGatewayImpl::on_daily_timer, this, std::placeholders::_1));
    }

    void TdGatewayImpl::on_switch_day(const std::string &trading_day)
    {
        if (nullptr != account_manager_)
        {
            account_manager_->on_switch_day(trading_day);
            account_manager_->dump_to_db(ACCOUNT_ASSET_DB_FILE(this->get_account_id()).c_str(), true);
        }
    }
}
