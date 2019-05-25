//
// Created by qlu on 2019/2/21.
//

#ifndef KUNGFU_GATEWAY_IMP_H
#define KUNGFU_GATEWAY_IMP_H

#include <kungfu/log/spdlog_config.h>
#include <kungfu/yijinjing/journal/journal.h>

#include <kungfu/wingchun/gateway/gateway.h>
#include "subscription_storage.hpp"
#include "state_storage.hpp"

#include <kungfu/wingchun/feed_handler.h>
#include <kungfu/wingchun/msg.h>

#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/util/uid_generator.h>
#include <kungfu/wingchun/storage/order_storage.h>
#include <kungfu/wingchun/storage/trade_storage.h>
#include <kungfu/wingchun/portfolio/account_manager.h>
#include <kungfu/wingchun/util/nn_publisher.h>
#include <kungfu/wingchun/event_loop/event_loop.h>

namespace kungfu
{
    namespace oms
    {
        class OrderManager;
    }


    class MarketDataStreamingWriter: public MarketDataFeedHandler
    {
    public:
        MarketDataStreamingWriter(kungfu::yijinjing::JournalWriterPtr writer): writer_(writer) {};
        virtual ~MarketDataStreamingWriter() {}
        void on_quote(const Quote* quote) override
        {
            writer_->write_frame(quote, sizeof(Quote), -1, (int16_t)MsgType::Quote, true, -1);
        }
        void on_entrust(const Entrust* entrust) override
        {
            writer_->write_frame(entrust, sizeof(Entrust), -1, (int16_t)MsgType::Entrust, true, -1);
        }
        void on_transaction(const Transaction* transaction) override
        {
            writer_->write_frame(transaction, sizeof(Transaction), -1, (int16_t)MsgType::Transaction, true, -1);
        }

    private:
        kungfu::yijinjing::JournalWriterPtr writer_;
    };

    class TraderDataStreamingWriter: public TraderDataFeedHandler
    {
    public:
        TraderDataStreamingWriter(kungfu::yijinjing::JournalWriterPtr writer): writer_(writer) {};
        virtual ~TraderDataStreamingWriter() {}
        virtual void on_order(const Order* order)
        {
            writer_->write_frame(order, sizeof(Order), -1, (int16_t)MsgType::Order, true, -1);
        }
        virtual void on_trade(const Trade* trade)
        {
            writer_->write_frame(trade, sizeof(Trade), -1, (int16_t)MsgType::Trade, true, -1);
        }

    private:
        kungfu::yijinjing::JournalWriterPtr writer_;
    };

    class GatewayImpl: virtual public Gateway
    {
    public:
        GatewayImpl(const std::string& source, const std::string& name);
        virtual ~GatewayImpl();

        virtual const std::string& get_name() const { return name_; }
        virtual const std::string& get_source() const { return source_; }

        virtual GatewayState get_state() const  { return state_; };
        virtual void set_state(const GatewayState& state, const std::string& message = "");

        virtual void init();
        virtual void start();
        virtual void stop();

        kungfu::CalendarPtr get_calendar() const { return calendar_; }
        const NNPublisher* get_publisher() const { return nn_publisher_.get(); }
        std::shared_ptr<GatewayStateStorage> get_state_storage() const { return state_storage_; }
        std::shared_ptr<yijinjing::nanomsg::socket> get_rsp_socket() const { return rsp_socket_; }
        std::shared_ptr<spdlog::logger> get_logger() const { return logger_; }

    protected:
        std::string source_;
        std::string name_;
        GatewayState state_;

        std::shared_ptr<spdlog::logger> logger_;

        std::shared_ptr<EventLoop> loop_;
        std::shared_ptr<yijinjing::nanomsg::socket> rsp_socket_;

        kungfu::CalendarPtr calendar_;
        std::shared_ptr<GatewayStateStorage> state_storage_;
        std::unique_ptr<NNPublisher> nn_publisher_;
    };

    class MdGatewayImpl: virtual public MdGateway, public GatewayImpl
    {
    public:
        MdGatewayImpl(const std::string& source): GatewayImpl(source, MD_GATEWAY_NAME(source)) {}
        virtual ~MdGatewayImpl() {}

        virtual void init() override;
        virtual void on_started() override {};
        virtual void on_login(const std::string& recipient, const std::string& client_id) override ;

        void register_subscription_storage(std::shared_ptr<SubscriptionStorage> subscription_storage) { subscription_storage_ = subscription_storage; }
        std::shared_ptr<SubscriptionStorage> get_subscription_storage() {return subscription_storage_;}

        void register_feed_handler(std::shared_ptr<MarketDataFeedHandler> feed_handler) { feed_handler_ = feed_handler; };
        std::shared_ptr<MarketDataFeedHandler> get_feed_handler() {return feed_handler_;};

        std::vector<Instrument> get_subscriptions() { return subscription_storage_->get_subscriptions(); }

        void on_quote(const Quote& quote);
        void on_entrust(const Entrust& entrust);
        void on_transaction(const Transaction& transaction);
        void on_subscribe(const std::string &recipient, const std::vector<Instrument> &instruments, bool is_level2);

    private:
        std::shared_ptr<SubscriptionStorage> subscription_storage_;
        std::shared_ptr<MarketDataFeedHandler> feed_handler_;
    };

    class TdGatewayImpl: virtual public TdGateway, public GatewayImpl
    {
    public:
        TdGatewayImpl(const std::string& source, const std::string& name): GatewayImpl(source, name) {}
        virtual ~TdGatewayImpl();

        virtual void init() override;
        virtual void on_started() override;
        virtual void on_login(const std::string& recipient, const std::string& client_id) override;

        virtual bool req_position_detail() override { return false;}
        virtual bool req_position() override = 0;
        virtual bool req_account() override = 0;

        void init_account_manager();
        void register_order_storage(std::shared_ptr<kungfu::storage::OrderStorage> order_storage) { order_storage_ = order_storage; }
        void register_trade_storage(std::shared_ptr<kungfu::storage::TradeStorage> trade_storage) { trade_storage_ = trade_storage; }
        void register_feed_handler(std::shared_ptr<TraderDataFeedHandler> feed_handler) { feed_handler_ = feed_handler; };

        std::shared_ptr<kungfu::storage::OrderStorage> get_order_storage(){return order_storage_;};
        std::shared_ptr<kungfu::storage::TradeStorage> get_trade_storage(){return trade_storage_;};
        std::shared_ptr<kungfu::AccountManager> get_account_manager() {return account_manager_; }

        std::vector<uint64_t> get_pending_orders(const std::string& client_id = "") const;

        uint64_t next_id();

        bool add_market_feed(const std::string& source_name);
        void subscribe_holdings() const;

        void on_order_input(const OrderInput& order_input);
        void on_order_action(const OrderAction& order_action);

        void on_manual_order_input(OrderInput& order_input);
        void on_manual_order_action(const std::string& account_id, const std::string& client_id, const std::vector<uint64_t>& order_ids);

        void on_order(Order& order);
        void on_trade(Trade& trade);
        void on_position(const Position& pos, bool is_last);
        void on_position_detail(const Position& pos_detail, bool is_last);
        void on_account(AccountInfo& account);
        void on_quote(const Quote& quote);
        void on_1min_timer(int64_t nano);
        void on_daily_timer(int64_t nano);
        void on_switch_day(const std::string& trading_day);

    private:
        std::shared_ptr<TraderDataFeedHandler> feed_handler_;
        std::shared_ptr<kungfu::storage::OrderStorage> order_storage_;
        std::shared_ptr<kungfu::storage::TradeStorage> trade_storage_;
        std::shared_ptr<kungfu::AccountManager> account_manager_;
        std::shared_ptr<oms::OrderManager> order_manager_;

        std::unique_ptr<UidGenerator> uid_generator_;

        std::vector<Position> rsp_pos_;
        std::vector<Position> rsp_pos_detail_;
    };
}

#endif //KUNGFU_GATEWAY_IMP_H
