//
// Created by qlu on 2019-10-14.
//

#include <kungfu/wingchun/common.h>
#include <kungfu/wingchun/book/book.h>

using namespace kungfu::rx;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::practice;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;

namespace kungfu::wingchun::book
{

    longfist::types::Position Book::get_long_position(const longfist::types::Quote &quote)
    {
        return get_position(Direction::Long, quote);
    }

    longfist::types::Position Book::get_short_position(const longfist::types::Quote &quote)
    {
        return get_position(Direction::Short, quote);
    }

    Bookkeeper::Bookkeeper(apprentice &app, const broker::Client &broker_client) :
            app_(app), broker_client_(broker_client), instruments_(), books_()
    {}

    Book &Bookkeeper::get_book(uint32_t uid)
    {
        auto &result = books_[uid];
        result.asset.holder_uid = uid;
        return result;
    }

    const Instrument &Bookkeeper::get_inst_info(const std::string &instrument_id) const
    {
        auto id = yijinjing::util::hash_str_32(instrument_id);
        if (this->instruments_.find(id) == this->instruments_.end())
        {
            throw wingchun_error(fmt::format("no instrument info found for {}", instrument_id));
        }
        return this->instruments_.at(id);
    }

    std::vector<Instrument> Bookkeeper::all_inst_info() const
    {
        std::vector<Instrument> res(instruments_.size());
        transform(instruments_.begin(), instruments_.end(), res.begin(), [](auto kv)
        { return kv.second; });
        return res;
    }

    std::vector<Book> Bookkeeper::get_books()
    {
        std::vector<Book> result(books_.size());
        auto selector = [](auto pair)
        { return pair.second; };
        std::transform(books_.begin(), books_.end(), result.begin(), selector);
        return result;
    }

    void Bookkeeper::set_accounting_method(longfist::enums::InstrumentType instrument_type, AccountingMethod_ptr accounting_method)
    {
        accounting_methods_.emplace(instrument_type, accounting_method);
    }

    void Bookkeeper::on_start(const rx::connectable_observable<event_ptr> &events)
    {
        events | is_own<Quote>(broker_client_) |
        $([&](const event_ptr &event)
          {
              const Quote &data = event->data<Quote>();
              auto accounting_method = accounting_methods_.at(data.instrument_type);
              for (const auto &item: books_)
              {
                  accounting_method->apply_quote(item.second, data);
              }
          });

        events | is(OrderInput::tag) |
        $([&](const event_ptr &event)
          {
              const OrderInput &data = event->data<OrderInput>();
              auto accounting_method = accounting_methods_.at(data.instrument_type);
              accounting_method->apply_order_input(get_book(event->source()), data);
              accounting_method->apply_order_input(get_book(event->dest()), data);
          });

        events | is(Order::tag) |
        $([&](const event_ptr &event)
          {
              const Order &data = event->data<Order>();
              auto accounting_method = accounting_methods_.at(data.instrument_type);
              accounting_method->apply_order(get_book(event->source()), data);
              accounting_method->apply_order(get_book(event->dest()), data);
          });

        events | is(Trade::tag) |
        $([&](const event_ptr &event)
          {
              const Trade &data = event->data<Trade>();
              auto accounting_method = accounting_methods_.at(data.instrument_type);
              accounting_method->apply_trade(get_book(event->source()), data);
              accounting_method->apply_trade(get_book(event->dest()), data);
          });
    }
}