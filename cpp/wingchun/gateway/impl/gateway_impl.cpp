//
// Created by qlu on 2019/2/21.
//

#include "gateway_impl.h"

#include "config.h"
#include "serialize.h"

#include "util/include/env.h"
#include "util/include/filesystem_util.h"
#include "util/include/timer_util.h"
#include "gateway/include/util.hpp"

#include <cstring>
#include <cstdio>
#include <algorithm>

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <nn.hpp>

#include "storage/snapshot_storage.h"

namespace kungfu
{
#define DUMP_1D_SNAPSHOT(account_id, account_info) kungfu::storage::SnapshotStorage(\
    ACCOUNT_SNAPSHOT_DB_FILE(account_id), ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME, true, true).insert(account_info)

#define DUMP_1M_SNAPSHOT(account_id, account_info) kungfu::storage::SnapshotStorage(\
    ACCOUNT_SNAPSHOT_DB_FILE(account_id), ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME, false, true).insert(account_info)

    GatewayImpl::GatewayImpl(const std::string &source, const std::string &name, int log_level) 
            : source_(source), name_(name), state_(GatewayState::Idle)
    {
        kungfu::yijinjing::KungfuLog::setup_log(name);
        kungfu::yijinjing::KungfuLog::set_log_level(log_level);
    }

    GatewayImpl::~GatewayImpl()
    {
        set_state(GatewayState::Idle);
    }

    void GatewayImpl::init()
    {

        calendar_ = CalendarPtr(new Calendar());

        loop_ = std::shared_ptr<EventLoop>(new EventLoop(get_name()));

        if (!create_folder_if_not_exists(GATEWAY_FOLDER(this->get_name())))
        {
            SPDLOG_ERROR("failed to create gateway folder {}", GATEWAY_FOLDER(this->get_name()));
            abort();
        }

        std::string rep_url = GATEWAY_REP_URL(get_name());
        rsp_socket_ = std::shared_ptr<nn::socket>(new nn::socket(AF_SP, NN_REP));
        try
        {
            rsp_socket_->bind(rep_url.c_str());
        }
        catch(std::exception &e)
        {
            SPDLOG_ERROR("failed to bind to rep_url {}, exception: {}", rep_url.c_str(), e.what());
            abort();
        }

        loop_->add_socket(rsp_socket_);

        std::string state_db_file = GATEWAY_STATE_DB_FILE(get_name());
#ifdef  _WINDOWS
        std::replace(state_db_file.begin(), state_db_file.end(), '/', '\\');
#endif
        std::remove(state_db_file.c_str());
        state_storage_ =  std::shared_ptr<GatewayStateStorage>(new GatewayStateStorage(state_db_file));

        std::string url = GATEWAY_PUB_URL(name_);
        nn_publisher_ = std::unique_ptr<NNPublisher>(new NNPublisher(url));
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
            SPDLOG_TRACE("sending {} ", js);
            get_rsp_socket()->send(js.c_str(), js.length() + 1, 0);
        }
    }

    void MdGatewayImpl::on_quote(const Quote &quote)
    {
        QUOTE_TRACE(kungfu::to_string(quote));
        feed_handler_->on_quote(&quote);
    }

    void MdGatewayImpl::on_entrust(const Entrust &entrust)
    {
        ENTRUST_TRACE(kungfu::to_string(entrust));
        feed_handler_->on_entrust(&entrust);
    }

    void MdGatewayImpl::on_transaction(const Transaction &transaction)
    {
        TRANSACTION_TRACE(kungfu::to_string(transaction));
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
            get_rsp_socket()->send(js.c_str(), js.length() + 1, 0);
        }
    }

    void TdGatewayImpl::init()
    {
        GatewayImpl::init();
        if (!create_folder_if_not_exists(ACCOUNT_FOLDER(this->get_account_id())))
        {
            SPDLOG_ERROR("failed to create account folder {}", ACCOUNT_FOLDER(this->get_account_id()));
        }

        std::string rep_url = ACCOUNT_REP_URL(get_account_id());
        acc_rsp_socket_ = std::shared_ptr<nn::socket>(new nn::socket(AF_SP, NN_REP));
        try
        {
            acc_rsp_socket_->bind(rep_url.c_str());
        }
        catch(std::exception &e)
        {
            SPDLOG_ERROR("failed to bind to acc_rep_url {}, exception: {}", rep_url.c_str(), e.what());
        }

        kungfu::storage::SnapshotStorage s1(ACCOUNT_SNAPSHOT_DB_FILE(get_account_id()), ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME, true, true);
        kungfu::storage::SnapshotStorage s2(ACCOUNT_SNAPSHOT_DB_FILE(get_account_id()), ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME, false, true);

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

        calendar_->register_switch_day_callback(std::bind(&TdGatewayImpl::on_switch_day, this, std::placeholders::_1));

        loop_->add_socket(acc_rsp_socket_);
    }

    void TdGatewayImpl::on_started()
    {
        add_market_feed(this->get_source());
        subscribe_holdings();
    }

    void TdGatewayImpl::init_account_manager()
    {
        AccountType account_type = get_account_type();
        std::string asset_db_file = ACCOUNT_ASSET_DB_FILE(this->get_account_id());

        account_manager_ = std::shared_ptr<AccountManager>(new AccountManager(this->get_account_id().c_str(), account_type, asset_db_file.c_str()));

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
            rsp_socket_->send(js.c_str(), js.length() + 1, 0);
        }
    }

    void TdGatewayImpl::on_quote(const Quote &quote)
    {
        QUOTE_TRACE(to_string(quote));
        account_manager_->on_quote(&quote);
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
            if (order_input.order_id == 0)
            {
                auto order_input_ptr = (OrderInput*)(&order_input);
                auto order_id = next_id();
                order_input_ptr->order_id = order_id;

                OrderInputRsp rsp = {};
                rsp.order_id = order_id;
                NNMsg msg = {};
                msg.msg_type = MsgType::RspOrderInput;
                msg.data = rsp;
                std::string js = to_string(msg);
                SPDLOG_TRACE("sending {} ", js);
                acc_rsp_socket_->send(js.c_str(), js.length() + 1, 0);
            }

            insert_order(order_input);
        }
    }

    void TdGatewayImpl::on_order_action(const OrderAction &order_action)
    {
        if (order_action.order_action_id == 0)
        {
            auto order_action_ptr = (OrderAction*)(&order_action);
            auto order_action_id = next_id();
            order_action_ptr->order_action_id = order_action_id;

            OrderActionRsp rsp = {};
            rsp.order_id = order_action.order_id;
            rsp.order_action_id = order_action_id;
            NNMsg msg = {};
            msg.msg_type = MsgType::RspOrderAction;
            msg.data = rsp;
            std::string js = to_string(msg);
            SPDLOG_TRACE("sending {} ", js);
            acc_rsp_socket_->send(js.c_str(), js.length() + 1, 0);
        }

        cancel_order(order_action);
    }

    void TdGatewayImpl::on_order(Order &order)
    {
        ORDER_TRACE(kungfu::to_string(order));
        feed_handler_->on_order(&order);
        account_manager_->on_order(&order);
        get_publisher()->publish_order(order);
        order_storage_->add_order(order.order_id, order);
    }

    void TdGatewayImpl::on_trade(Trade &trade)
    {
        TRADE_TRACE(kungfu::to_string(trade));
        double commission = account_manager_->calc_commission(&trade);
        double tax = account_manager_->calc_tax(&trade);
        trade.commission = commission;
        trade.tax = tax;

        feed_handler_->on_trade(&trade);
        account_manager_->on_trade(&trade);

        auto trade_id = trade_storage_->add_trade(trade);
        trade.id = trade_id;
        get_publisher()->publish_trade(trade);
    }

    void TdGatewayImpl::on_position(const Position &pos, bool is_last)
    {
        POSITION_TRACE(kungfu::to_string(pos));
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
        POSITION_DETAIL_TRACE(kungfu::to_string(pos_detail));
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
        ACCOUNT_TRACE( kungfu::to_string(account));
        feed_handler_->on_account(&account);
        account_manager_->on_account(account);
    }

    void TdGatewayImpl::on_1min_timer(int64_t nano)
    {
        auto account_info = account_manager_->get_account_info();
        bool is_open = calendar_->is_open(nano, account_info.type == AccountTypeFuture ? EXCHANGE_SHFE : EXCHANGE_SSE);
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
        loop_->register_nanotime_callback_at_next(DAILY_STORAGE_TIME, std::bind(&TdGatewayImpl::on_daily_timer, this, std::placeholders::_1));
    }

    void TdGatewayImpl::on_switch_day(const std::string &trading_day)
    {
        if (nullptr != account_manager_)
        {
            account_manager_->switch_day(trading_day);
        }
    }
}
