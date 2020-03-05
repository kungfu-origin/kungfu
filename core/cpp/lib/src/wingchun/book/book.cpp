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
        service_location_ = location::make_shared(mode::LIVE, category::SYSTEM, "service", "ledger", app.get_io_device()->get_home()->locator);
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

        auto error_handler = rx::error_handler_log(fmt::format("book [{}]", location->uname));

        books_[location->uid] = book;

        this->monitor_positions(location, book);
        this->monitor_position_details(location, book);

        for (const auto &item: app_.get_channels())
        {
            const auto &channel = item.second;
            if (channel.source_id == location->uid)
            {
                std::string dest_uname = app_.has_location(channel.dest_id) ? app_.get_location(
                        channel.dest_id)->uname : "unknown";
                app_.get_reader()->join(location, channel.dest_id, app_.now());
                SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                            location->uname, location->uid, location->category == category::TD ? "order/trades" : "input",
                            location->uname, location->uid, dest_uname, channel.dest_id);
            } else if (channel.dest_id == location->uid)
            {
                if (app_.has_location(channel.source_id))
                {
                    auto src_location = app_.get_location(channel.source_id);
                    app_.get_reader()->join(src_location, channel.dest_id, app_.now());
                    SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                                location->uname, location->uid, location->category == category::TD ? "input" : "order/trades",
                                src_location->uname, src_location->uid, location->uname, location->uid);
                } else
                {
                    SPDLOG_WARN("location {} not exist", channel.source_id);
                }
            }
        }

        events_ | is(Channel::tag) |
        $([=](const event_ptr &event)
          {
              const auto &channel = event->data<Channel>();
              if (channel.source_id == location->uid)
              {
                  std::string dest_uname = app_.has_location(channel.dest_id) ? app_.get_location(
                          channel.dest_id)->uname : "unknown";
                  app_.get_reader()->join(location, channel.dest_id, app_.now());
                  SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                              location->uname, location->uid, location->category == category::TD ? "order/trades" : "input",
                              location->uname, location->uid, dest_uname, channel.dest_id);
              } else if (channel.dest_id == location->uid)
              {
                  if (app_.has_location(channel.source_id))
                  {
                      auto src_location = app_.get_location(channel.source_id);
                      app_.get_reader()->join(src_location, channel.dest_id, app_.now());
                      SPDLOG_INFO("book {} [{:08x}] read {} from {} [{:08x}] to {} [{:08x}]",
                                  location->uname, location->uid, location->category == category::TD ? "input" : "order/trades",
                                  src_location->uname, src_location->uid, location->uname, location->uid);
                  } else
                  {
                      SPDLOG_WARN("location {} not exist", channel.source_id);
                  }
              }
          }, error_handler);

        events_ | filter([=](const event_ptr &event)
                         {
                             return event->msg_type() == PositionEnd::tag and event->data<PositionEnd>().holder_uid == location->uid;
                         }) | first() |
        $([=](const event_ptr &event)
          {
              book->ready_ = true;
          }, error_handler);
       
        events_ | is(Quote::tag) |
        $([=](const event_ptr &event)
          {
              try
              {
                  book->on_quote(event, event->data<Quote>());
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          }, error_handler);

        events_ | is(Trade::tag) | filter([=](const event_ptr &e)
                                          {
                                              return location->category == category::TD ? location->uid == e->source() : location->uid == e->dest();
                                          }) |
        $([=](const event_ptr &event)
          {
              try
              {
                  book->on_trade(event, event->data<Trade>());
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          }, error_handler);

        events_ | is(Order::tag) | filter([=](const event_ptr &e)
                                          {
                                              return location->category == category::TD ? location->uid == e->source() : location->uid == e->dest();
                                          }) |
        $([=](const event_ptr &event)
          {
              try
              {
                  const auto &data = event->data<Order>();
                  book->on_order(event, event->data<Order>());
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          }, error_handler);

        events_ | is(OrderInput::tag) | filter([=](const event_ptr &e)
                                               {
                                                   return location->category == category::TD ? location->uid == e->dest() : location->uid ==
                                                                                                                            e->source();
                                               }) |
        $([=](const event_ptr &event)
          {
              try
              {
                  const auto &data = event->data<OrderInput>();
                  book->on_order_input(event, event->data<OrderInput>());
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          }, error_handler);

        events_ | is(Asset::tag) | filter([=](const event_ptr &e)
                                          {
                                              const auto &asset = e->data<Asset>();
                                              return asset.holder_uid == location->uid;
                                          }) |
        $([=](const event_ptr &event)
          {
              try
              {
                  book->on_asset(event, event->data<Asset>());
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          }, error_handler);

        events_ | is(TradingDay::tag) |
        $([=](const event_ptr &event)
          {
              try
              {
                  book->on_trading_day(event, event->data<int64_t>());
              }
              catch (const std::exception &e)
              {
                  SPDLOG_ERROR("Unexpected exception {}", e.what());
              }
          }, error_handler);

        auto home = app_.get_io_device()->get_home();
        if (home->uid != service_location_->uid)
        {
            if (home->mode == mode::LIVE and not app_.has_location(service_location_->uid))
            {
                throw wingchun_error("has no location for ledger service");
            }

            if (not app_.has_writer(service_location_->uid))
            {
                events_ | is(RequestWriteTo::tag) |
                filter([=](const event_ptr &e)
                       {
                           const RequestWriteTo &data = e->data<RequestWriteTo>();
                           return data.dest_id == service_location_->uid;
                       }) | first() |
                $([=](const event_ptr &e)
                  {
                      nlohmann::json location_desc{};
                      location_desc["mode"] = location->mode;
                      location_desc["category"] = location->category;
                      location_desc["group"] = location->group;
                      location_desc["name"] = location->name;
                      location_desc["uname"] = location->uname;
                      location_desc["uid"] = location->uid;
                      auto msg = location_desc.dump();
                      auto writer = app_.get_writer(service_location_->uid);
                      auto &&frame = writer->open_frame(0, QryAsset::tag, msg.length());
                      memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                      writer->close_frame(msg.length());
                      SPDLOG_INFO("{}[{:08x}] asset requested", location->uname, location->uid);
                  }, error_handler);
            } else
            {
                nlohmann::json location_desc{};
                location_desc["mode"] = location->mode;
                location_desc["category"] = location->category;
                location_desc["group"] = location->group;
                location_desc["name"] = location->name;
                location_desc["uname"] = location->uname;
                location_desc["uid"] = location->uid;
                auto msg = location_desc.dump();
                auto writer = app_.get_writer(service_location_->uid);
                auto &&frame = writer->open_frame(0, QryAsset::tag, msg.length());
                memcpy(reinterpret_cast<void *>(frame->address() + frame->header_length()), msg.c_str(), msg.length());
                writer->close_frame(msg.length());
                SPDLOG_INFO("{}[{:08x}] asset requested", location->uname, location->uid);
            }

            app_.request_write_to(app_.now(), service_location_->uid);
            app_.request_read_from(app_.now(), service_location_->uid, app_.now());
        }
    }
}