//
// Created by qlu on 2019-10-14.
//

#ifndef KUNGFU_WINGCHUN_BOOK_H
#define KUNGFU_WINGCHUN_BOOK_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>
#include <kungfu/wingchun/broker/client.h>
#include <kungfu/longfist/enums.h>

namespace kungfu::wingchun::book
{
    struct Book
    {
        typedef std::unordered_map<uint32_t, longfist::types::Position> PositionMap;
        typedef std::unordered_map<uint32_t, longfist::types::PositionDetail> PositionDetailMap;

        longfist::types::Asset asset = {};

        PositionMap long_positions = {};
        PositionDetailMap long_position_details = {};

        PositionMap short_positions = {};
        PositionDetailMap short_position_details = {};

        std::unordered_map<uint64_t, longfist::types::Order> orders = {};

        longfist::types::Position &get_long_position(const longfist::types::Quote &quote);

        longfist::types::Position &get_short_position(const longfist::types::Quote &quote);

        template<typename DataType>
        longfist::types::Position &get_position(longfist::enums::Direction direction, const DataType &data)
        {
            assert(asset.holder_uid != 0);
            PositionMap &positions = direction == longfist::enums::Direction::Long ? long_positions : short_positions;
            auto position_id = get_symbol_id(data.instrument_id, data.exchange_id);
            auto &result = positions[position_id];
            if (not result.holder_uid)
            {
                result.holder_uid = asset.holder_uid;
                result.ledger_category = asset.ledger_category;
                result.instrument_id = data.instrument_id;
                result.exchange_id = data.exchange_id;
                result.direction = direction;
            }
            return result;
        }

        template<typename DataType>
        longfist::types::Position &get_position(const DataType &data)
        {
            return get_position(get_direction(data.instrument_type, data.side, data.offset), data);
        }
    };

    DECLARE_PTR(Book)

    class AccountingMethod
    {
    public:
        AccountingMethod() = default;

        virtual ~AccountingMethod() = default;

        virtual void apply_quote(Book_ptr book, const longfist::types::Quote &quote) = 0;

        virtual void apply_order_input(Book_ptr book, const longfist::types::OrderInput &input) = 0;

        virtual void apply_order(Book_ptr book, const longfist::types::Order &order) = 0;

        virtual void apply_trade(Book_ptr book, const longfist::types::Trade &trade) = 0;
    };

    DECLARE_PTR(AccountingMethod)

    class Bookkeeper
    {
    public:
        explicit Bookkeeper(yijinjing::practice::apprentice &app, const broker::Client &broker_client);

        virtual ~Bookkeeper() = default;

        Book_ptr get_book(uint32_t uid);

        void set_accounting_method(longfist::enums::InstrumentType instrument_type, AccountingMethod_ptr accounting_method);

        void restore(const longfist::StateMapType &state_map);

        void on_start(const rx::connectable_observable<event_ptr> &events);

    private:
        yijinjing::practice::apprentice &app_;
        const broker::Client &broker_client_;

        std::unordered_map<longfist::enums::InstrumentType, AccountingMethod_ptr> accounting_methods_ = {};
        std::unordered_map<uint32_t, Book_ptr> books_ = {};
        std::unordered_map<uint32_t, longfist::types::Instrument> instruments_ = {};

        template<typename DataType>
        static constexpr auto is_own = [](const broker::Client &broker_client)
        {
            return rx::filter([&](const event_ptr &event)
                              {
                                  const DataType &data = event->data<DataType>();
                                  return event->msg_type() == DataType::tag and
                                         broker_client.is_subscribed(event->source(), data.exchange_id, data.instrument_id);
                              });
        };
    };

    DECLARE_PTR(Bookkeeper)
}

#endif //KUNGFU_WINGCHUN_BOOK_H
