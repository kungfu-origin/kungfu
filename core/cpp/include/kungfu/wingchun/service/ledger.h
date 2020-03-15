//
// Created by Keren Dong on 2019-06-28.
//

#ifndef KUNGFU_WINGCHUN_LEDGER_H
#define KUNGFU_WINGCHUN_LEDGER_H

#include <kungfu/longfist/serialize/journal.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/wingchun/book/book.h>

namespace kungfu::wingchun::service
{
    class Ledger : public yijinjing::practice::apprentice
    {
    public:
        explicit Ledger(yijinjing::data::locator_ptr locator, longfist::enums::mode m, bool low_latency = false);

        virtual ~Ledger() = default;

        book::Bookkeeper &get_bookkeeper();

        void publish(const std::string &msg);

        uint64_t cancel_order(const event_ptr &event, uint32_t account_location_uid, uint64_t order_id);

        std::vector<longfist::types::Position> get_positions(const yijinjing::data::location_ptr &location);

        bool has_asset(const yijinjing::data::location_ptr &location);

        longfist::types::Asset get_asset(const yijinjing::data::location_ptr &location);

        const std::unordered_map<uint32_t, longfist::types::Instrument> &get_instruments() const;

        void dump_asset_snapshot(const longfist::types::Asset &asset);

        virtual std::string handle_request(const event_ptr &event, const std::string &msg) = 0;

        virtual void handle_instrument_request(const event_ptr &event) = 0;

        virtual void handle_asset_request(const event_ptr &event, const yijinjing::data::location_ptr &app_location) = 0;

    protected:

        void on_ready() override;

        void on_start() override;

    private:
        longfist::journal::publisher publish_state;

        yijinjing::nanomsg::socket_ptr pub_sock_;
        broker::Client broker_client_;
        book::Bookkeeper bookkeeper_;

        std::unordered_map<uint64_t, state<longfist::types::Asset>> &assets_;
        std::unordered_map<uint64_t, state<longfist::types::OrderStat>> &order_stats_;

        template<typename DataType>
        void write_book(int64_t trigger_time, const uint32_t location_uid, const DataType &data,
                        const std::string &source_id, const std::string &account_id, const std::string &client_id)
        {
            auto book = bookkeeper_.get_book(location_uid);
            auto &position = book->get_position(data);
            strcpy(position.source_id, source_id.c_str());
            strcpy(position.account_id, account_id.c_str());
            strcpy(position.client_id, client_id.c_str());
            write_to(trigger_time, position, location_uid);
            write_to(trigger_time, book->asset, location_uid);
        }

        void write_book(const event_ptr &event, const longfist::types::OrderInput &data)
        {
            auto source = get_location(event->source());
            auto dest = get_location(event->dest());
            write_book(event->gen_time(), event->source(), data, dest->group, dest->name, source->name);
            write_book(event->gen_time(), event->dest(), data, dest->group, dest->name, "");
        }

        template<typename DataType>
        void write_book(const event_ptr &event, const DataType &data)
        {
            auto source = get_location(event->source());
            write_book(event->gen_time(), event->source(), data, source->group, source->name, "");
            if (event->dest())
            {
                auto dest = get_location(event->dest());
                write_book(event->gen_time(), event->dest(), data, source->group, source->name, dest->name);
            }
        }
    };
}

#endif //KUNGFU_WINGCHUN_LEDGER_H
