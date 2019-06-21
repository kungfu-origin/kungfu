//
// Created by qlu on 2019/2/21.
//

#ifndef KUNGFU_GATEWAY_IMP_H
#define KUNGFU_GATEWAY_IMP_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/gateway.h>
#include <kungfu/wingchun/calendar/calendar.h>
#include <kungfu/wingchun/util/uid_generator.h>
#include <kungfu/wingchun/storage/order_storage.h>
#include <kungfu/wingchun/storage/trade_storage.h>
#include <kungfu/wingchun/portfolio/account_manager.h>
#include <kungfu/wingchun/util/nn_publisher.h>
#include <kungfu/wingchun/event_loop/event_loop.h>

#include "subscription_storage.hpp"
#include "state_storage.hpp"

namespace kungfu
{
    namespace wingchun
    {
        namespace oms
        {
            class OrderManager;
        }

        class GatewayImpl : virtual public Gateway, public practice::apprentice
        {
        public:
            GatewayImpl(bool low_latency, yijinjing::data::category c, const std::string &source, const std::string &name);

            virtual ~GatewayImpl();

            inline std::string get_app_db_file(const std::string &name)
            {
                auto home = get_io_device()->get_home();
                return home->locator->layout_file(home, yijinjing::data::layout::SQLITE, name);
            }

            const std::string &get_name() const override
            { return name_; }

            const std::string &get_source() const override
            { return source_; }

            enum GatewayState get_state() const override
            { return state_; };

            void set_state(const enum GatewayState &state, const std::string &message = "") override;

            Calendar_ptr get_calendar() const
            { return calendar_; }

            const NNPublisher *get_publisher() const
            { return nn_publisher_.get(); }

            std::shared_ptr<GatewayStateStorage> get_state_storage() const
            { return state_storage_; }

            virtual void configure_event_source(kungfu::yijinjing::event_source_ptr event_source);

            void react(rx::observable <yijinjing::event_ptr> events) override;

        protected:
            std::string source_;
            std::string name_;
            yijinjing::event_source_ptr event_source_;
            enum GatewayState state_;

            Calendar_ptr calendar_;
            std::shared_ptr<GatewayStateStorage> state_storage_;
            std::unique_ptr<NNPublisher> nn_publisher_;
        };

        class MdGatewayImpl : virtual public MdGateway, public GatewayImpl
        {
        public:
            MdGatewayImpl(bool low_latency, const std::string &source);

            virtual ~MdGatewayImpl()
            {}

            virtual void on_started() override
            {};

            virtual void on_login(const std::string &source, const std::string &name, const std::string &client_id) override;

            void register_subscription_storage(std::shared_ptr<SubscriptionStorage> subscription_storage)
            { subscription_storage_ = subscription_storage; }

            std::shared_ptr<SubscriptionStorage> get_subscription_storage()
            { return subscription_storage_; }

            std::vector<msg::data::Instrument> get_subscriptions()
            { return subscription_storage_->get_subscriptions(); }

            void react(rx::observable <yijinjing::event_ptr> events) override;

        private:
            std::shared_ptr<SubscriptionStorage> subscription_storage_;
        };

        class TdGatewayImpl : virtual public TdGateway, public GatewayImpl
        {
        public:
            TdGatewayImpl(bool low_latency, const std::string &source, const std::string &name);

            virtual ~TdGatewayImpl()
            {};

            virtual void on_started() override;

            virtual void on_login(const std::string &source, const std::string &name, const std::string &client_id) override;

            virtual bool req_position_detail() override
            { return false; }

            virtual bool req_position() override = 0;

            virtual bool req_account() override = 0;

            void init_account_manager();

            void register_order_storage(std::shared_ptr<storage::OrderStorage> order_storage)
            { order_storage_ = order_storage; }

            void register_trade_storage(std::shared_ptr<storage::TradeStorage> trade_storage)
            { trade_storage_ = trade_storage; }

            std::shared_ptr<storage::OrderStorage> get_order_storage()
            { return order_storage_; };

            std::shared_ptr<storage::TradeStorage> get_trade_storage()
            { return trade_storage_; };

            std::shared_ptr<AccountManager> get_account_manager()
            { return account_manager_; }

            std::vector<uint64_t> get_pending_orders(const std::string &client_id = "") const;

            uint64_t next_id();

            bool add_market_feed(const std::string &source_name);

            void subscribe_holdings() const;

            void on_order_input(const msg::data::OrderInput &order_input);

            void on_order_action(const msg::data::OrderAction &order_action);

            void on_manual_order_input(msg::data::OrderInput &order_input);

            void on_manual_order_action(const std::string &account_id, const std::string &client_id, const std::vector<uint64_t> &order_ids);

            void on_order(msg::data::Order &order);

            void on_trade(msg::data::Trade &trade);

            void on_position(const msg::data::Position &pos, bool is_last);

            void on_position_detail(const msg::data::Position &pos_detail, bool is_last);

            void on_account(msg::data::AccountInfo &account);

            void on_quote(const msg::data::Quote &quote);

            void on_1min_timer(int64_t nano);

            void on_daily_timer(int64_t nano);

            void on_switch_day(const std::string &trading_day);

            void configure_event_source(kungfu::yijinjing::event_source_ptr event_source) override;

            void react(rx::observable <yijinjing::event_ptr> events) override;

        private:
            std::shared_ptr<storage::OrderStorage> order_storage_;
            std::shared_ptr<storage::TradeStorage> trade_storage_;
            std::shared_ptr<AccountManager> account_manager_;
            std::shared_ptr<oms::OrderManager> order_manager_;

            std::unique_ptr<UidGenerator> uid_generator_;

            std::vector<msg::data::Position> rsp_pos_;
            std::vector<msg::data::Position> rsp_pos_detail_;
        };
    }
}

#endif //KUNGFU_GATEWAY_IMP_H
