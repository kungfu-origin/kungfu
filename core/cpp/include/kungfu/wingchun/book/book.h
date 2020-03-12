//
// Created by qlu on 2019-10-14.
//

#ifndef KUNGFU_WINGCHUN_BOOK_H
#define KUNGFU_WINGCHUN_BOOK_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/practice/apprentice.h>

namespace kungfu::wingchun::book
{
    FORWARD_DECLARE_PTR(BookContext)

    class Book
    {
    public:
        longfist::types::Asset asset = {};

        std::unordered_map<uint64_t, longfist::types::Position> long_positions = {};
        std::unordered_map<uint64_t, longfist::types::PositionDetail> long_position_details = {};

        std::unordered_map<uint64_t, longfist::types::Position> short_positions = {};
        std::unordered_map<uint64_t, longfist::types::PositionDetail> short_position_details = {};

        virtual ~Book() = default;

        [[nodiscard]] virtual uint32_t get_location_uid() const = 0;

        virtual void on_trading_day(const event_ptr &event, int64_t daytime) = 0;

        virtual void on_asset(const event_ptr &event, const longfist::types::Asset &asset) = 0;

        virtual void on_positions(const std::vector<longfist::types::Position> &positions) = 0;

        virtual void on_position_details(const std::vector<longfist::types::PositionDetail> &position_details) = 0;

        virtual void on_quote(const event_ptr &event, const longfist::types::Quote &quote) = 0;

        virtual void on_order_input(const event_ptr &event, const longfist::types::OrderInput &input) = 0;

        virtual void on_order(const event_ptr &event, const longfist::types::Order &order) = 0;

        virtual void on_trade(const event_ptr &event, const longfist::types::Trade &trade) = 0;
    };

    DECLARE_PTR(Book)

    class BookContext
    {
    public:
        explicit BookContext(yijinjing::practice::apprentice &app, const rx::connectable_observable <event_ptr> &events);

        ~BookContext() = default;

        void add_book(const yijinjing::data::location_ptr &location, const Book_ptr &book);

        void pop_book(uint32_t location_uid);

        void monitor_instruments();

        [[nodiscard]] const longfist::types::Instrument &get_inst_info(const std::string &instrument_id) const;

        [[nodiscard]] std::vector<longfist::types::Instrument> all_inst_info() const;

        [[nodiscard]] std::vector<Book_ptr> get_books();

        void on_trading_day(const event_ptr &event, int64_t daytime);

        void on_asset(const event_ptr &event, const longfist::types::Asset &asset);

        void on_quote(const event_ptr &event, const longfist::types::Quote &quote);

        void on_order_input(const event_ptr &event, const longfist::types::OrderInput &input);

        void on_order(const event_ptr &event, const longfist::types::Order &order);

        void on_trade(const event_ptr &event, const longfist::types::Trade &trade);

    private:
        yijinjing::practice::apprentice &app_;
        const rx::connectable_observable <event_ptr> &events_;
        std::unordered_map<uint32_t, longfist::types::Instrument> instruments_;
        std::unordered_map<uint32_t, Book_ptr> books_;

        void monitor_positions(const yijinjing::data::location_ptr &location, const Book_ptr &book);

        void monitor_position_details(const yijinjing::data::location_ptr &location, const Book_ptr &book);
    };
}

#endif //KUNGFU_WINGCHUN_BOOK_H
