//
// Created by qlu on 2019-10-14.
//

#include <kungfu/wingchun/common.h>
#include <kungfu/wingchun/book/book.h>
#include <kungfu/yijinjing/log/setup.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book
{
    BookContext::BookContext(yijinjing::practice::apprentice &app, const rx::connectable_observable<event_ptr> &events) :
            app_(app), events_(events)
    {
        auto home = app.get_io_device()->get_home();
        log::copy_log_settings(home, home->name);
        this->monitor_instruments();
        service_location_ = location::make_shared(mode::LIVE, category::SYSTEM, "service", "ledger", app.get_locator());
        SPDLOG_INFO("book context for {}", app.get_home_uname());
        SPDLOG_INFO("book context books {}", books_.size());
    }

    const Instrument &BookContext::get_inst_info(const std::string &instrument_id) const
    {
        auto id = yijinjing::util::hash_str_32(instrument_id);
        if (this->instruments_.find(id) == this->instruments_.end())
        {
            throw wingchun_error(fmt::format("no instrument info found for {}", instrument_id));
        }
        return this->instruments_.at(id);
    }

    std::vector<Instrument> BookContext::all_inst_info() const
    {
        std::vector<Instrument> res(instruments_.size());
        transform(instruments_.begin(), instruments_.end(), res.begin(), [](auto kv)
        { return kv.second; });
        return res;
    }

    std::vector<Book_ptr> BookContext::get_books()
    {
        std::vector<Book_ptr> result(books_.size());
        auto selector = [](auto pair)
        { return pair.second; };
        std::transform(books_.begin(), books_.end(), result.begin(), selector);
        return result;
    };

    void BookContext::on_quote(const event_ptr &event, const longfist::types::Quote &quote)
    {
        for (const auto &item: books_)
        {
            item.second->on_quote(event, quote);
        }
    }

    void BookContext::on_trade(const event_ptr &event, const longfist::types::Trade &trade)
    {
        for (const auto &item: books_)
        {
            if (item.second->get_location()->uid == event->source())
            {
                item.second->on_trade(event, trade);
            }
        }
    }

    void BookContext::on_order(const event_ptr &event, const longfist::types::Order &order)
    {
        for (const auto &item: books_)
        {
            if (item.second->get_location()->uid == event->source())
            {
                item.second->on_order(event, order);
            }
        }
    }

    void BookContext::on_order_input(const event_ptr &event, const longfist::types::OrderInput &input)
    {
        SPDLOG_INFO("on order input {}", input.to_string());
        SPDLOG_INFO("on order input books size {}", books_.size());
        for (const auto &item: books_)
        {
            if (item.second->get_location()->uid == event->source())
            {
                item.second->on_order_input(event, input);
            }
        }
    }

    void BookContext::on_asset(const event_ptr &event, const longfist::types::Asset &asset)
    {
        for (const auto &item: books_)
        {
            if (item.second->get_location()->uid == event->dest())
            {
                item.second->on_asset(event, asset);
            }
        }
    }

    void BookContext::on_trading_day(const event_ptr &event, int64_t daytime)
    {
        for (const auto &item: books_)
        {
            item.second->on_trading_day(event, daytime);
        }
    }

    void BookContext::monitor_instruments()
    {
        auto insts = events_ | to(app_.get_live_home_uid()) | take_while([&](const event_ptr &event)
                                                                         {
                                                                             return event->msg_type() != InstrumentEnd::tag;
                                                                         }) | is(Instrument::tag) | reduce(std::vector<Instrument>(),
                                                                                                           [](std::vector<Instrument> res,
                                                                                                              event_ptr event)
                                                                                                           {
                                                                                                               res.push_back(
                                                                                                                       event->data<Instrument>());
                                                                                                               return res;
                                                                                                           }) | as_dynamic();

        insts.subscribe([&](std::vector<Instrument> res)
                        {
                            if (!this->app_.is_live())
                            { return; }
                            SPDLOG_INFO("instrument info updated, size: {}", res.size());
                            this->instruments_.clear();
                            for (const auto &inst: res)
                            {
                                auto id = yijinjing::util::hash_str_32(inst.instrument_id);
                                this->instruments_[id] = inst;
                            }
                            this->monitor_instruments();
                        });
    }

    void BookContext::monitor_positions(const location_ptr &location, const Book_ptr &book)
    {
        auto positions = events_ | take_while([=](const event_ptr &event)
                                              {
                                                  return event->msg_type() != PositionEnd::tag or
                                                         event->data<PositionEnd>().holder_uid != location->uid;
                                              }) | is(Position::tag) |
                         reduce(std::vector<Position>(), [=](std::vector<Position> res, event_ptr event)
                         {
                             const auto &position = event->data<Position>();
                             if (position.holder_uid == location->uid)
                             {
                                 res.push_back(event->data<Position>());
                             }
                             return res;
                         }) | as_dynamic();

        positions.subscribe([=](std::vector<Position> res)
                            {
                                if (!this->app_.is_live())
                                { return; }
                                try
                                {
                                    book->on_positions(res);
                                    this->monitor_positions(location, book);
                                }
                                catch (const std::exception &e)
                                {
                                    SPDLOG_ERROR("Unexpected exception {}", e.what());
                                }
                            },
                            [=](std::exception_ptr e)
                            {
                                try
                                { std::rethrow_exception(e); }
                                catch (const rx::empty_error &ex)
                                { SPDLOG_WARN("{}", ex.what()); }
                                catch (const std::exception &ex)
                                { SPDLOG_WARN("Unexpected exception {}", ex.what()); }
                            });
    }

    void BookContext::monitor_position_details(const location_ptr &location, const Book_ptr &book)
    {
        auto positions = events_ | take_while([=](const event_ptr &event)
                                              {
                                                  return event->msg_type() != PositionDetailEnd::tag or
                                                         event->data<PositionDetailEnd>().holder_uid != location->uid;
                                              }) | is(PositionDetail::tag) |
                         reduce(std::vector<PositionDetail>(), [](std::vector<PositionDetail> res, event_ptr event)
                         {
                             res.push_back(event->data<PositionDetail>());
                             return res;
                         }) | as_dynamic();

        positions.subscribe([=](std::vector<PositionDetail> res)
                            {
                                if (!this->app_.is_live())
                                { return; }
                                try
                                {
                                    book->on_position_details(res);
                                    this->monitor_position_details(location, book);
                                } catch (const std::exception &e)
                                {
                                    SPDLOG_ERROR("Unexpected exception {}", e.what());
                                }
                            },
                            [=](std::exception_ptr e)
                            {
                                try
                                { std::rethrow_exception(e); }
                                catch (const rx::empty_error &ex)
                                { SPDLOG_WARN("{}", ex.what()); }
                                catch (const std::exception &ex)
                                { SPDLOG_WARN("Unexpected exception {}", ex.what()); }
                            });
    }

    void BookContext::pop_book(uint32_t location_uid)
    {
        books_.erase(location_uid);
    }

    void BookContext::add_book(const yijinjing::data::location_ptr &location, const Book_ptr &book)
    {
        if (books_.find(location->uid) != books_.end())
        {
            return;
        }

        if (location->category != category::TD and location->category != category::STRATEGY)
        {
            throw wingchun_error(fmt::format("invalid book location category: {}", get_category_name(location->category)));
        }

        books_.emplace(location->uid, book);

        this->monitor_positions(location, book);
        this->monitor_position_details(location, book);
    }
}