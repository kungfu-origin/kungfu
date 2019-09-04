//
// Created by Keren Dong on 2019-06-28.
//

#ifndef WINGCHUN_WATCHER_H
#define WINGCHUN_WATCHER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace service
        {
            class Ledger : public practice::apprentice
            {
            public:
                explicit Ledger(yijinjing::data::locator_ptr locator, yijinjing::data::mode m, bool low_latency = false);

                virtual ~Ledger() = default;

                void publish(const std::string &msg);

                void publish_broker_states(int64_t trigger_time);

                void new_order_single(const yijinjing::event_ptr &event, uint32_t account_location_uid, msg::data::OrderInput &order_input);

                void cancel_order(const yijinjing::event_ptr &event, uint32_t account_location_uid, uint64_t order_id);

                virtual std::string handle_request(const yijinjing::event_ptr &event, const std::string &msg) = 0;

                virtual void on_trader_started(int64_t trigger_time, const yijinjing::data::location_ptr &app_location) = 0;

                virtual void on_quote(yijinjing::event_ptr event, const msg::data::Quote &quote) = 0;

                virtual void on_order(yijinjing::event_ptr event, const msg::data::Order &order) = 0;

                virtual void on_trade(yijinjing::event_ptr event, const msg::data::Trade &trade) = 0;

                virtual void on_stock_account(const msg::data::Asset &asset, const std::vector<msg::data::Position> &positions) = 0;

                virtual void on_future_account(const msg::data::Asset &asset, const std::vector<msg::data::PositionDetail> &position_details) = 0;

                virtual void on_instruments(const std::vector<msg::data::Instrument> &instruments) = 0;

                virtual void pre_start() = 0;

            protected:

                void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

                void deregister_location(int64_t trigger_time, uint32_t location_uid) override;

                void on_write_to(const yijinjing::event_ptr &event) override;

                void on_read_from(const yijinjing::event_ptr &event) override;

                void on_start() override;

                void publish_broker_state(int64_t trigger_time, const yijinjing::data::location_ptr &broker_location, msg::data::BrokerState state);

                void update_broker_state(int64_t trigger_time, const yijinjing::data::location_ptr &broker_location, msg::data::BrokerState state);

            private:
                yijinjing::nanomsg::socket_ptr pub_sock_;
                std::unordered_map<uint32_t, msg::data::BrokerState> broker_states_;
                std::unordered_map<uint32_t, msg::data::Asset> asset_info_;
                std::unordered_map<uint32_t, std::vector<msg::data::Position>> position_buffer_;
                std::unordered_map<uint32_t, std::vector<msg::data::PositionDetail>> position_detail_buffer_;
                std::unordered_map<uint32_t, std::vector<msg::data::Instrument>> instrument_buffer_;

                void watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location);

                void monitor_market_data(int64_t trigger_time, uint32_t md_location_uid);

                void alert_market_data(int64_t trigger_time, uint32_t md_location_uid);

                void request_subscribe(const std::string& source_name, const std::vector<msg::data::Instrument> insts);

                void subscribe_holdings(uint32_t account_location_id, const std::vector<msg::data::Position> &positions);
            };
        }
    }
}

#endif //WINGCHUN_WATCHER_H
