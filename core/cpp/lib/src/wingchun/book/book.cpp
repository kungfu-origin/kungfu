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

    longfist::types::Position &Book::get_long_position(const longfist::types::Quote &quote)
    {
        return get_position(Direction::Long, quote);
    }

    longfist::types::Position &Book::get_short_position(const longfist::types::Quote &quote)
    {
        return get_position(Direction::Short, quote);
    }

    Bookkeeper::Bookkeeper(apprentice &app, const broker::Client &broker_client) :
            app_(app), broker_client_(broker_client), instruments_(), books_()
    {}

    Book_ptr Bookkeeper::get_book(uint32_t location_uid)
    {
        if (books_.find(location_uid) == books_.end())
        {
            books_.emplace(location_uid, std::make_shared<Book>());
            auto &asset = books_.at(location_uid)->asset;
            auto location = app_.get_location(location_uid);
            asset.holder_uid = location_uid;
            asset.ledger_category = location->category == category::TD ? LedgerCategory::Account : LedgerCategory::Strategy;
            if (location->category == category::TD)
            {
                strcpy(asset.broker_id, location->group.c_str());
                strcpy(asset.account_id, location->name.c_str());
            }
        }
        return books_.at(location_uid);
    }

    void Bookkeeper::set_accounting_method(longfist::enums::InstrumentType instrument_type, AccountingMethod_ptr accounting_method)
    {
        accounting_methods_.emplace(instrument_type, accounting_method);
    }

    void Bookkeeper::restore(const longfist::StateMapType &state_map)
    {
        for (auto &pair : state_map[boost::hana::type_c<Asset>])
        {
            auto &state = pair.second;
            auto &asset = state.data;
            get_book(asset.holder_uid)->asset = asset;
        }
        for (auto &pair : state_map[boost::hana::type_c<Position>])
        {
            auto &state = pair.second;
            auto &position = state.data;
            auto book = get_book(position.holder_uid);
            auto &positions = position.direction == longfist::enums::Direction::Long ? book->long_positions : book->short_positions;
            positions.emplace(get_symbol_id(position.instrument_id, position.exchange_id), position);
        }
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