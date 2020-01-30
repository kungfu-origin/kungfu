//
// Created by qlu on 2019-10-14.
//

#ifndef KUNGFU_WINGCHUN_BOOK_H
#define KUNGFU_WINGCHUN_BOOK_H

#include <kungfu/longfist/types.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace book
        {
            FORWARD_DECLARE_PTR(BookContext)

            class Book
            {
            public:
                Book(): ready_(false) {}

                virtual void on_quote(event_ptr event, const longfist::types::Quote &quote) = 0;

                virtual void on_trade(event_ptr event, const longfist::types::Trade &trade) = 0;

                virtual void on_order(event_ptr event, const longfist::types::Order &order) = 0;

                virtual void on_order_input(event_ptr event, const longfist::types::OrderInput &input) = 0;

                virtual void on_positions(const std::vector<longfist::types::Position>& positions) = 0;

                virtual void on_position_details(const std::vector<longfist::types::PositionDetail>& position_details) = 0;

                virtual void on_asset(event_ptr event, const longfist::types::Asset& asset) = 0;

                virtual void on_trading_day(event_ptr event, int64_t daytime) = 0;

                virtual ~Book() = default;

                bool is_ready() const { return ready_; }

            private:
                bool ready_;

                friend class BookContext;
            };

            DECLARE_PTR(Book)

            class BookContext
            {
            public:
                explicit BookContext(practice::apprentice &app, const rx::connectable_observable<event_ptr> &events);

                ~BookContext() = default;

                void add_book(const yijinjing::data::location_ptr& location, const Book_ptr& book);

                void pop_book(uint32_t location_uid);

                void monitor_instruments();

                const longfist::types::Instrument& get_inst_info(const std::string& instrument_id) const;

                std::vector<longfist::types::Instrument> all_inst_info() const;

                std::vector<Book_ptr> get_books()
                {
                    std::vector<Book_ptr> book_vec;
                    for(const auto& item: books_)
                    {
                        book_vec.push_back(item.second);
                    }
                    return book_vec;
                };

            private:
                void monitor_positions(const yijinjing::data::location_ptr& location, const Book_ptr& book);

                void monitor_position_details(const yijinjing::data::location_ptr& location, const Book_ptr& book);

                practice::apprentice &app_;

                const rx::connectable_observable<event_ptr> &events_;

                yijinjing::data::location_ptr service_location_;

                std::unordered_map<uint32_t, longfist::types::Instrument> instruments_;

                std::unordered_map<uint32_t, Book_ptr> books_;
            };
        }
    }
}

#endif //KUNGFU_WINGCHUN_BOOK_H
