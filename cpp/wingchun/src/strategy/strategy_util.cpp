//
// Created by qlu on 2019/1/29.
//

#include <fmt/format.h>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/time/timer.h>
#include <kungfu/yijinjing/journal/journal.h>

#include <kungfu/wingchun/strategy/strategy_util.h>
#include <kungfu/wingchun/portfolio/portfolio_manager.h>
#include <kungfu/wingchun/util/nn_publisher.h>

#include <kungfu/wingchun/serialize.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/oms/def.h>

#include <kungfu/wingchun/util/env.h>
#include <kungfu/wingchun/util/filesystem_util.h>
#include <kungfu/wingchun/util/instrument.h>
#include <kungfu/wingchun/storage/log.h>
#include <kungfu/wingchun/storage/account_list_storage.h>
#include <kungfu/wingchun/storage/snapshot_storage.h>


using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
#define DUMP_1D_SNAPSHOT(name, pnl) storage::SnapshotStorage(\
    STRATEGY_SNAPSHOT_DB_FILE(name), PORTFOLIO_ONE_DAY_SNAPSHOT_TABLE_NAME, true, false).insert(pnl)

#define DUMP_1M_SNAPSHOT(name, pnl) storage::SnapshotStorage(\
    STRATEGY_SNAPSHOT_DB_FILE(name), PORTFOLIO_ONE_MIN_SNAPSHOT_TABLE_NAME, false, false).insert(pnl)

    StrategyUtil::StrategyUtil(const std::string& name): name_(name), calendar_(new Calendar()), has_stock_account_(false), has_future_account_(false)
    {
        kungfu::log::LogConfig::setup_log(name);
        kungfu::log::LogConfig::set_log_level(spdlog::level::info);
        auto logger = spdlog::default_logger();
        kungfu::calendar_util::set_logger(logger);

        create_folder_if_not_exists(STRATEGY_FOLDER(name));

        std::string pub_url = STRATEGY_PUB_URL(name);
        publisher_ = std::shared_ptr<NNPublisher>(new NNPublisher(pub_url));
        publisher_->set_logger(logger);

        calendar_->register_switch_day_callback(std::bind(&StrategyUtil::on_switch_day, this, std::placeholders::_1));

        int worker_id = UidWorkerStorage::get_instance(fmt::format(UID_WORKER_DB_FILE_FORMAT, get_base_dir()))->get_uid_worker_id(name);
        if (worker_id <= 0)
        {
            UidWorkerStorage::get_instance(fmt::format(UID_WORKER_DB_FILE_FORMAT, get_base_dir()))->add_uid_worker(name);
            worker_id = UidWorkerStorage::get_instance(fmt::format(UID_WORKER_DB_FILE_FORMAT, get_base_dir()))->get_uid_worker_id(name);
        }
        uid_generator_ = std::unique_ptr<UidGenerator>(new UidGenerator(worker_id, UID_EPOCH_SECONDS));

        writer_ = kungfu::yijinjing::JournalWriter::create(fmt::format(STRATEGY_JOURNAL_FOLDER_FORMAT, get_base_dir()), this->name_, this->name_);

        kungfu::storage::set_logger(logger);
        kungfu::portfolio_util::set_logger(logger);

        init_portfolio_manager();

        order_manager_ = oms::create_order_manager();
    }

    StrategyUtil::~StrategyUtil()
    {
        portfolio_manager_->dump_to_db(STRATEGY_ASSET_DB_FILE(name_).c_str());
    }

    bool StrategyUtil::add_md(const std::string &source_id)
    {
        storage::SourceListStorage(STRATEGY_MD_FEED_DB_FILE(name_)).add_source(source_id);
        auto rsp = gateway::add_market_feed(source_id, name_);
        if (rsp.state != GatewayState::Ready)
        {
            SPDLOG_WARN("market feed {} is not ready yet, current state: {}.", source_id, (int)rsp.state);
            return false;
        }
        else
        {
            return true;
        }
    }

    bool StrategyUtil::add_account(const std::string &source_id, const std::string &account_id, double cash_limit)
    {
        SubPortfolioInfo info = {};
        strcpy(info.source_id, source_id.c_str());
        strcpy(info.account_id, account_id.c_str());

        storage::AccountListStorage(STRATEGY_ACCOUNT_LIST_DB_FILE(name_)).add_account(this->name_, account_id, source_id);
        publisher_->publish_sub_portfolio_info(info);

        auto rsp = gateway::register_trade_account(source_id, account_id, this->name_);
		if (rsp.error_msg != "")
		{
			return false;
		}
        info.account_type = rsp.type;

        has_stock_account_ = has_stock_account_ || (info.account_type == AccountTypeStock || info.account_type == AccountTypeCredit);
        has_future_account_ = has_future_account_ || info.account_type == AccountTypeFuture;

        if (portfolio_manager_->get_account(account_id.c_str()) == nullptr)
        {
            AccountInfo acc = {};
            int64_t nano = yijinjing::getNanoTime();
            acc.rcv_time = nano;
            acc.update_time = nano;
            strcpy(acc.account_id, account_id.c_str());
            strcpy(acc.source_id, source_id.c_str());
            strcpy(acc.trading_day, calendar_->get_current_trading_day().c_str());
            acc.account_type = rsp.type;
            acc.initial_equity = cash_limit;
            acc.static_equity = cash_limit;
            acc.dynamic_equity = cash_limit;
            acc.avail = cash_limit;
            portfolio_manager_->on_account(acc);
            portfolio_manager_->dump_to_db(STRATEGY_ASSET_DB_FILE(name_).c_str());
        }

        if (rsp.state != GatewayState::Ready)
        {
            SPDLOG_WARN("trade gateway {} is not ready yet, current state: {}.", TD_GATEWAY_NAME(source_id, account_id), (int)rsp.state);
            return false;
        }
        else
        {
            return true;
        }
    }

    void StrategyUtil::on_quote(const kungfu::journal::Quote& quote)
    {
        SPDLOG_TRACE("instrument_id: {}, last_price: {}", quote.instrument_id, quote.last_price);
        quote_map_[get_symbol(quote.instrument_id, quote.exchange_id)] = quote;
        portfolio_manager_->on_quote(&quote);
    }

    void StrategyUtil::on_quote_py(uintptr_t quote)
    {
        on_quote(*(const kungfu::journal::Quote*)quote);
    }

    void StrategyUtil::on_order(const kungfu::journal::Order& order)
    {
        portfolio_manager_->on_order(&order);
        order_manager_->on_order(&order);
    }

    void StrategyUtil::on_order_py(uintptr_t order)
    {
        on_order(*(const kungfu::journal::Order*)order);
    }

    void StrategyUtil::on_trade(const kungfu::journal::Trade& trade)
    {
        portfolio_manager_->on_trade(&trade);
        portfolio_manager_->dump_to_db(STRATEGY_ASSET_DB_FILE(name_).c_str());
    }

    void StrategyUtil::on_trade_py(uintptr_t trade)
    {
        on_trade(*(const kungfu::journal::Trade*)trade);
    }

    void StrategyUtil::on_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& status_msg)
    {
        order_manager_->on_algo_order_status(order_id, algo_type, status_msg);
    }

    void StrategyUtil::register_switch_day_callback(std::function<void(const std::string &)> cb)
    {
        cbs_.emplace_back(cb);
    }

    bool StrategyUtil::register_algo_service()
    {
        //TODO
        return true;
    }

    void StrategyUtil::subscribe(const std::string &source, const std::vector<std::string> &instruments, const string &exchange_id, bool is_level2)
    {
        std::vector<journal::Instrument> inst_vec;
        for (const auto& ins : instruments)
        {
            journal::Instrument inst = {};
            strcpy(inst.instrument_id, ins.c_str());
            strcpy(inst.exchange_id, exchange_id.c_str());
            inst_vec.emplace_back(inst);
            subscribed_[source].insert(get_symbol(inst.instrument_id, inst.exchange_id));
        }
        kungfu::gateway::subscribe(source, inst_vec, is_level2, this->name_);
    }

    bool StrategyUtil::is_subscribed(const std::string &source, const std::string &instrument,
                                     const string &exchange_id) const
    {
        return subscribed_.find(source) != subscribed_.end() &&
            subscribed_.at(source).find(get_symbol(instrument, exchange_id)) != subscribed_.at(source).end();
    }

    uint64_t StrategyUtil::insert_limit_order(const std::string &instrument_id, const std::string &exchange_id,
                                              const std::string &account_id, double limit_price, int64_t volume,
                                              Side side, Offset offset)
    {
        OrderInput input = {};
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

        return insert_order(input);
    }

    uint64_t StrategyUtil::insert_fak_order(const std::string &instrument_id, const std::string &exchange_id,
                                            const std::string &account_id, double limit_price, int64_t volume,
                                            Side side, Offset offset)
    {
        OrderInput input = {};

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

        return insert_order(input);
    }

    uint64_t StrategyUtil::insert_fok_order(const std::string &instrument_id, const std::string &exchange_id,
                                            const std::string &account_id, double limit_price, int64_t volume,
                                            Side side, Offset offset)
    {
        OrderInput input = {};

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

        return insert_order(input);
    }

    uint64_t StrategyUtil::insert_market_order(const std::string &instrument_id, const std::string &exchange_id,
                                               const std::string &account_id, int64_t volume, Side side, Offset offset)
    {
        OrderInput input = {};

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
        }
        else
        {
            input.price_type = PriceTypeAny;
            input.time_condition = TimeConditionIOC;
            input.volume_condition = VolumeConditionAny;
        }
        return insert_order(input);
    }

    uint64_t StrategyUtil::insert_order(OrderInput& order_input)
    {
        if (nullptr == portfolio_manager_->get_account(order_input.account_id))
        {
            SPDLOG_ERROR("account {} has to be added in init before use", order_input.account_id);
            return 0;
        }

        uint64_t uid = next_id();
        order_input.order_id = uid;
        strcpy(order_input.client_id, this->name_.c_str());
        writer_->write_frame(&order_input, sizeof(OrderInput), -1, (int)MsgType::OrderInput, 1, -1);
        return uid;
    }

    uint64_t StrategyUtil::cancel_order(uint64_t order_id)
    {
        struct OrderAction action = {};

        uint64_t uid = next_id();
        action.order_action_id = uid;
        action.order_id = order_id;
        action.action_flag = OrderActionFlagCancel;

        writer_->write_frame(&action, sizeof(OrderAction), -1, (int)MsgType::OrderAction, true, -1);

        return uid;
    }

    uint64_t StrategyUtil::insert_algo_order(const std::string& algo_type, const std::string& order_input_msg)
    {
        uint64_t uid = next_id();
        AlgoOrderInput input = {};

        input.algo_type = algo_type;
        input.order_id = uid;
        input.client_id = name_;
        input.input = order_input_msg;
        std::string js = to_string(input);
        
        writer_->write_frame(js.c_str(), js.length() + 1, -1, (int)MsgType::AlgoOrderInput, true, -1 );
        return uid;
    }

    uint64_t StrategyUtil::modify_algo_order(uint64_t order_id, const std::string &cmd)
    {
        uint64_t uid = next_id();

        AlgoOrderAction action = {};
        action.order_action_id = uid;
        action.order_id = order_id;
        action.action = cmd;

        std::string js = flying::to_string(action);

        writer_->write_frame(js.c_str(), js.length() + 1, -1, (int)MsgType::AlgoOrderAction, true, -1);

        return uid;
    }

    const Quote* const StrategyUtil::get_last_md(const std::string& instrument_id, const std::string& exchange_id) const
    {
        auto iter = quote_map_.find(get_symbol(instrument_id, exchange_id));
        if (iter == quote_map_.end())
        {
            return nullptr;
        }
        else
        {
            return & iter->second;
        }
    }

    Position StrategyUtil::get_position(const std::string& instrument_id, const std::string& exchange_id, Direction direction, const std::string& account_id) const
    {
        if (direction == DirectionLong)
        {
            auto pos = portfolio_manager_->get_long_pos(account_id.c_str(), instrument_id.c_str(), exchange_id.c_str());
            return pos;
        }
        else
        {
            return portfolio_manager_->get_short_pos(account_id.c_str(), instrument_id.c_str(), exchange_id.c_str());
        }
    }

    PortfolioInfo StrategyUtil::get_portfolio_info() const
    {
        return portfolio_manager_->get_portfolio();
    }

    SubPortfolioInfo StrategyUtil::get_sub_portfolio_info(const std::string& account_id) const
    {
        return portfolio_manager_->get_sub_portfolio(account_id.c_str());
    }

    std::vector<uint64_t> StrategyUtil::get_pending_orders(const string &account_id) const
    {
        std::vector<uint64_t> order_ids;
        auto orders = order_manager_->get_pending_orders();
        for (const auto& order : orders)
        {
            auto simple_order = std::dynamic_pointer_cast<oms::SimpleOrder>(order);
            if (nullptr != simple_order)
            {
                if (account_id.empty() || account_id == simple_order->get_account_id())
                {
                    order_ids.emplace_back(simple_order->get_order_id());
                }
            }
        }
        return order_ids;
    }

    void StrategyUtil::on_push_by_min()
    {
        int64_t nano = yijinjing::getNanoTime();
        bool is_open = false;
        if (has_stock_account_)
        {
            is_open = is_open || calendar_->is_open(nano, EXCHANGE_SSE);
        }
        if (has_future_account_)
        {
            is_open = is_open || calendar_->is_open(nano, EXCHANGE_SHFE);
        }
        SPDLOG_TRACE("before push, is_open {} nano {}", is_open ? "true" : "false", nano);

        if (is_open)
        {
            auto pnl = portfolio_manager_->get_portfolio();
            pnl.update_time = (int64_t)std::round((double)nano / 1000000000)* 1000000000;
            publisher_->publish_portfolio_info(pnl, kungfu::MsgType::PortfolioByMin);
            DUMP_1M_SNAPSHOT(name_, pnl);
        }
    }

    void StrategyUtil::on_push_by_day()
    {
        auto pnl = portfolio_manager_->get_portfolio();
        pnl.update_time = (int64_t)std::round((double)yijinjing::getNanoTime() / 1000000000) * 1000000000;
        DUMP_1D_SNAPSHOT(name_, pnl);
    }

    void StrategyUtil::reload_instruments()
    {
        InstrumentManager::get_instrument_manager()->reload_from_db();
    }

    uint64_t StrategyUtil::next_id()
    {
        int64_t seconds = kungfu::yijinjing::getNanoTime() / kungfu::yijinjing::NANOSECONDS_PER_SECOND;
        return uid_generator_->next_id(seconds);
    }

    void StrategyUtil::on_switch_day(const std::string &trading_day)
    {
        if (nullptr != portfolio_manager_)
        {
            portfolio_manager_->on_switch_day(trading_day);
            portfolio_manager_->dump_to_db(STRATEGY_ASSET_DB_FILE(name_).c_str());
        }

        for (const auto& cb : cbs_)
        {
            cb(trading_day);
        }
    }

    std::vector<std::string> StrategyUtil::get_md_sources()
    {
        return storage::SourceListStorage(STRATEGY_MD_FEED_DB_FILE(name_)).get_sources();
    }

    std::vector<SubPortfolioInfo> StrategyUtil::get_accounts()
    {
        return storage::AccountListStorage(STRATEGY_ACCOUNT_LIST_DB_FILE(name_)).get_accounts();
    }

    void StrategyUtil::init_portfolio_manager()
    {
        std::string asset_db_file = STRATEGY_ASSET_DB_FILE(name_);
        portfolio_manager_ = create_portfolio_manager(this->name_.c_str(), asset_db_file.c_str());
        portfolio_manager_->register_pos_callback(std::bind(&NNPublisher::publish_pos, publisher_.get(), std::placeholders::_1));
        portfolio_manager_->register_pnl_callback(std::bind(&NNPublisher::publish_portfolio_info, publisher_.get(), std::placeholders::_1,MsgType::Portfolio));

        int64_t last_update = portfolio_manager_->get_last_update();
        SPDLOG_TRACE("{} last update {}", name_, last_update);
        if (last_update > 0)
        {
            std::vector<std::string> folders;
            std::vector<std::string> names;
            for (const auto& source: get_md_sources())
            {
                folders.emplace_back(MD_JOURNAL_FOLDER(source));
                names.emplace_back(MD_JOURNAL_NAME(source));
            }
            for (const auto& account: get_accounts())
            {
                folders.emplace_back(TD_JOURNAL_FOLDER(account.source_id, account.account_id));
                names.emplace_back(TD_JOURNAL_NAME(account.source_id, account.account_id));
            }
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
                            portfolio_manager_->on_quote(quote);
                        }
                        break;
                    }
                    case (int) MsgType::Order:
                    {
                        auto* order = (const Order*) frame->getData();
                        if (strcmp(order->client_id, this->name_.c_str()) == 0 && order->rcv_time > last_update)
                        {
                            portfolio_manager_->on_order(order);
                        }
                        break;
                    }
                    case (int) MsgType::Trade:
                    {
                        auto* trade = (const Trade* ) frame->getData();
                        if (strcmp(trade->client_id, this->name_.c_str()) == 0 && trade->rcv_time > last_update)
                        {
                            portfolio_manager_->on_trade(trade);
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
            SPDLOG_INFO("forward portfolio manager from {}|{} to {}|{}", last_update, kungfu::yijinjing::parseNano(last_update, "%Y%m%d-%H:%M:%S"), portfolio_manager_->get_last_update(), kungfu::yijinjing::parseNano(portfolio_manager_->get_last_update(), "%Y%m%d-%H:%M:%S"));
        }
        portfolio_manager_->set_current_trading_day(calendar_->get_current_trading_day());
        portfolio_manager_->dump_to_db(asset_db_file.c_str());
        SPDLOG_INFO("portfolio_manager inited and set trading_day to {}", portfolio_manager_->get_current_trading_day());
    }
}
