//
// Created by qlu on 2019-10-14.
//

#ifndef KUNGFU_WINGCHUN_BOOK_H
#define KUNGFU_WINGCHUN_BOOK_H

#include <kungfu/practice/apprentice.h>
#include <kungfu/wingchun/msg.h>

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
                virtual void on_quote(yijinjing::event_ptr event, const msg::data::Quote &quote) = 0;

                virtual void on_trade(yijinjing::event_ptr event, const msg::data::Trade &trade) = 0;

                virtual void on_positions(const std::vector<msg::data::Position>& positions) = 0;

                virtual void on_position_details(const std::vector<msg::data::PositionDetail>& position_details) = 0;

                virtual void on_asset(yijinjing::event_ptr event, const msg::data::Asset& asset) = 0;

                virtual ~Book() = default;
            };

            DECLARE_PTR(Book)

            class BookContext
            {
            public:
                explicit BookContext(practice::apprentice &app, const rx::connectable_observable<yijinjing::event_ptr> &events);

                ~BookContext() = default;

                void add_book(const yijinjing::data::location_ptr& location, const Book_ptr& book);

                void monitor_instruments();

                const msg::data::Instrument& get_inst_info(const std::string& instrument_id) const;

                std::vector<msg::data::Instrument> all_inst_info() const;

            private:
                void monitor_positions(const yijinjing::data::location_ptr& location, const Book_ptr& book);

                void monitor_position_details(const yijinjing::data::location_ptr& location, const Book_ptr& book);

                practice::apprentice &app_;
                const rx::connectable_observable<yijinjing::event_ptr> &events_;

                std::unordered_map<uint32_t, msg::data::Instrument> instruments_;

                std::unordered_map<uint32_t, Book_ptr> books_;
            };
        }
    }
}

#endif //KUNGFU_WINGCHUN_BOOK_H
