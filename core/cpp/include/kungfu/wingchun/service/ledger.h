//
// Created by Keren Dong on 2019-06-28.
//

#ifndef KUNGFU_WINGCHUN_LEDGER_H
#define KUNGFU_WINGCHUN_LEDGER_H

#include <kungfu/longfist/serialize/journal.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/wingchun/book/book.h>

namespace kungfu::wingchun::service
{
    class Ledger : public yijinjing::practice::apprentice
    {
    public:
        explicit Ledger(yijinjing::data::locator_ptr locator, yijinjing::data::mode m, bool low_latency = false);

        virtual ~Ledger() = default;

        book::BookContext_ptr get_book_context();

        void publish(const std::string &msg);

        void publish_broker_states(int64_t trigger_time);

        uint64_t cancel_order(const event_ptr &event, uint32_t account_location_uid, uint64_t order_id);

        virtual std::string handle_request(const event_ptr &event, const std::string &msg) = 0;

        virtual void handle_instrument_request(const event_ptr &event) = 0;

        virtual void handle_asset_request(const event_ptr &event, const yijinjing::data::location_ptr &app_location) = 0;

        virtual void on_app_location(int64_t trigger_time, const yijinjing::data::location_ptr &app_location) = 0;

        virtual void on_quote(event_ptr event, const longfist::types::Quote &quote) = 0;

        virtual void on_order_action_error(event_ptr event, const longfist::types::OrderActionError &error) = 0;

        virtual void on_order(event_ptr event, const longfist::types::Order &order) = 0;

        virtual void on_trade(event_ptr event, const longfist::types::Trade &trade) = 0;

        virtual void on_instruments(const std::vector<longfist::types::Instrument> &instruments) = 0;

        virtual void pre_start() = 0;

    protected:

        void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location) override;

        void deregister_location(int64_t trigger_time, uint32_t location_uid) override;

        void on_write_to(const event_ptr &event) override;

        void on_read_from(const event_ptr &event) override;

        void on_start() override;

        void publish_broker_state(int64_t trigger_time, const yijinjing::data::location_ptr &broker_location, longfist::BrokerState state);

        void update_broker_state(int64_t trigger_time, const yijinjing::data::location_ptr &broker_location, longfist::BrokerState state);

    private:
        yijinjing::nanomsg::socket_ptr pub_sock_;
        longfist::journal::publisher publisher_;

        book::BookContext_ptr book_context_;

        std::unordered_map<uint32_t, longfist::BrokerState> broker_states_;

        void monitor_instruments(uint32_t broker_location);

        longfist::BrokerState get_broker_state(uint32_t broker_location) const;

        void watch(int64_t trigger_time, const yijinjing::data::location_ptr &app_location);

        void monitor_market_data(int64_t trigger_time, uint32_t md_location_uid);

        void alert_market_data(int64_t trigger_time, uint32_t md_location_uid);

        void request_subscribe(uint32_t account_location_id, const std::vector<longfist::types::Instrument> &instruments);

        template<class T>
        std::vector<longfist::types::Instrument> convert_to_instruments(const std::vector<T> &data)
        {
            std::vector<longfist::types::Instrument> instruments;
            for (const auto &item: data)
            {
                longfist::types::Instrument instrument = {};
                strcpy(instrument.instrument_id, item.instrument_id);
                strcpy(instrument.exchange_id, item.exchange_id);
                instruments.push_back(instrument);
            }
            std::sort(instruments.begin(), instruments.end(), [](const longfist::types::Instrument &a1, const longfist::types::Instrument &a2)
            {
                auto a1_value = get_symbol_id(a1.instrument_id, a1.exchange_id);
                auto a2_value = get_symbol_id(a2.instrument_id, a2.exchange_id);
                return a1_value < a2_value;
            });
            auto it = std::unique(instruments.begin(), instruments.end(),
                                  [](const longfist::types::Instrument &a1, const longfist::types::Instrument &a2)
                                  {
                                      return strcmp(a1.instrument_id, a2.instrument_id) == 0 && strcmp(a1.exchange_id, a2.exchange_id) == 0;
                                  });
            instruments.erase(it, instruments.end());
            return instruments;
        }
    };
}

#endif //KUNGFU_WINGCHUN_LEDGER_H
