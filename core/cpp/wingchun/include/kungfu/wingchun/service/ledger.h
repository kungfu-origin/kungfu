//
// Created by Keren Dong on 2019-06-28.
//

#ifndef WINGCHUN_WATCHER_H
#define WINGCHUN_WATCHER_H

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/book/book.h>

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

                book::BookContext_ptr get_book_context();

                void publish(const std::string &msg);

                void publish_broker_states(int64_t trigger_time);

                void new_order_single(const yijinjing::event_ptr &event, uint32_t account_location_uid, msg::data::OrderInput &order_input);

                void cancel_order(const yijinjing::event_ptr &event, uint32_t account_location_uid, uint64_t order_id);

                virtual std::string handle_request(const yijinjing::event_ptr &event, const std::string &msg) = 0;

                virtual void handle_instrument_request(const yijinjing::event_ptr &event) = 0;

                virtual void handle_asset_request(const yijinjing::event_ptr &event, const yijinjing::data::location_ptr &app_location) = 0;

                virtual void on_app_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location) = 0;

                virtual void on_quote(yijinjing::event_ptr event, const msg::data::Quote &quote) = 0;

                virtual void on_order(yijinjing::event_ptr event, const msg::data::Order &order) = 0;

                virtual void on_trade(yijinjing::event_ptr event, const msg::data::Trade &trade) = 0;

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

                book::BookContext_ptr book_context_;

                std::unordered_map<uint32_t, msg::data::BrokerState> broker_states_;

                void monitor_instruments(uint32_t broker_location);

                msg::data::BrokerState get_broker_state(uint32_t broker_location) const;

                void watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location);

                void monitor_market_data(int64_t trigger_time, uint32_t md_location_uid);

                void alert_market_data(int64_t trigger_time, uint32_t md_location_uid);

                void request_subscribe(uint32_t account_location_id, const std::vector<msg::data::Instrument> &instruments);

                template <class T> std::vector<msg::data::Instrument> convert_to_instruments(const std::vector<T>& data)
                {
                    std::vector<msg::data::Instrument> instruments;
                    for (const auto& item: data)
                    {
                        msg::data::Instrument instrument = {};
                        strcpy(instrument.instrument_id, item.instrument_id);
                        strcpy(instrument.exchange_id, item.exchange_id);
                        instruments.push_back(instrument);
                    }
                    std::sort(instruments.begin(), instruments.end());
                    auto it= std::unique(instruments.begin(), instruments.end());
                    instruments.erase(it, instruments.end());
                    return instruments;
                }
            };
        }
    }
}

#endif //WINGCHUN_WATCHER_H
