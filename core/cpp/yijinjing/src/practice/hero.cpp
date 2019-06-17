//
// Created by Keren Dong on 2019-06-15.
//

#include <spdlog/spdlog.h>

#include <rxcpp/rx.hpp>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/practice/hero.h>

namespace rx
{
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}
using namespace rx;

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu
{
    namespace practice
    {

        hero::hero(yijinjing::io_device_ptr io_device) : io_device_(io_device)
        {
            os::handle_os_signals();
            log::setup_log(io_device_->get_home(), io_device_->get_home()->name);
            reader_ = io_device_->open_reader_to_subscribe();
        }

        void hero::register_location(const yijinjing::data::location_ptr location)
        {
            locations_[location->uid] = location;
        }

        const yijinjing::data::location_ptr hero::get_location(uint32_t hash)
        {
            return locations_[hash];
        }

        void hero::pre_run()
        {
            for (auto handler : event_handlers_)
            {
//                handler->configure_event_source(shared_from_this());
            }
        }

        void test(event &e)
        {}

        void hero::run()
        {
            SPDLOG_INFO("{} started", io_device_->get_home()->uname);
            writers_[io_device_->get_home()->uid]->open_session();
            pre_run();

            auto events = rx::observable<>::create<event_ptr>(
                    [&](rx::subscriber<event_ptr> dest)
                    {
                        while (live_)
                        {
                            if (io_device_->get_observer()->wait())
                            {
                                std::string notice = io_device_->get_observer()->get_notice();
                                auto event = std::make_shared<nanomsg_json>(notice);
                                dest.on_next(event);
                            }
                            if (reader_->data_available())
                            {
                                dest.on_next(reader_->current_frame());
                                reader_->next();
                            }
                        }
                        dest.on_completed();
                    }).
                    publish();

            events.filter([](event_ptr e)
                          { return e->msg_type() == MsgType::Register; })
                    .subscribe([&](event_ptr e)
                               {
                                   auto data = e->data<action::RequestSubscribe>();
                                   auto location = get_location(e->source());
                                   reader_->subscribe(location, 0, data.from_time);
                                   reader_->subscribe(location, get_home_uid(), data.from_time);
                               });

            events.filter([](event_ptr e)
                          { return e->msg_type() == MsgType::RequestPublish; })
                    .subscribe([&](event_ptr e)
                               {
                                   auto data = e->data<action::RequestPublish>();
                                   if (writers_.find(data.dest_id) != writers_.end())
                                   {
                                       writers_[data.dest_id] = get_io_device()->open_writer(data.dest_id);
                                       writers_[data.dest_id]->open_session();
                                   } else
                                   {
                                       SPDLOG_ERROR("Ask publish for more than once");
                                   }
                               });

            events.on_error_resume_next(
                    [&](std::exception_ptr e) -> rx::observable<event_ptr>
                    {
                        try
                        { std::rethrow_exception(e); }
                        catch (const nn_exception &ex)
                        {
                            switch (ex.num())
                            {
                                case EINTR:
                                case EAGAIN:
                                case ETIMEDOUT:
                                {
                                    SPDLOG_INFO("apprentice quit because {}", ex.what());
                                    return rx::observable<>::empty<event_ptr>();
                                }
                                default:
                                {
                                    SPDLOG_ERROR("Unexpected nanomsg error: {}", ex.what());
                                    return rx::observable<>::error<event_ptr>(ex);
                                }
                            }
                        }
                        catch (const std::exception &ex)
                        {
                            SPDLOG_ERROR("Unexpected exception: {}", ex.what());
                            return rx::observable<>::error<event_ptr>(ex);
                        }
                    });

            events.finally([&](){
                writers_[io_device_->get_home()->uid]->close_session();
            });

            events.connect();
            post_run();
            SPDLOG_INFO("{} finished", io_device_->get_home()->uname);
        }

        void hero::post_run()
        {
            for (auto handler : event_handlers_)
            {
                handler->finish();
            }
        }

        void hero::try_once()
        {
            if (io_device_->get_observer()->wait())
            {
                std::string notice = io_device_->get_observer()->get_notice();
                nanomsg_json_ptr event = std::make_shared<nanomsg_json>(notice);
                for (auto handler : event_handlers_)
                {
                    handler->handle(event);
                }
            }

            if (reader_->data_available())
            {
                auto frame = reader_->current_frame();
                switch (frame->msg_type())
                {
                    case MsgType::RequestSubscribe:
                    {
                        auto data = frame->data<action::RequestSubscribe>();
                        auto location = get_location(data.source_id);
                        reader_->subscribe(location, 0, data.from_time);
                        reader_->subscribe(location, get_home_uid(), data.from_time);
                        break;
                    }
                    case MsgType::RequestPublish:
                    {
                        auto data = frame->data<action::RequestPublish>();
                        if (writers_.find(data.dest_id) != writers_.end())
                        {
                            writers_[data.dest_id] = get_io_device()->open_writer(data.dest_id);
                            writers_[data.dest_id]->open_session();
                        } else
                        {
                            SPDLOG_ERROR("Ask publish for more than once");
                        }
                        break;
                    }
                    default:
                    {
                        for (auto handler : event_handlers_)
                        {
                            handler->handle(reader_->current_frame());
                        }
                        break;
                    }
                }
                reader_->next();
            }
        }
    }
}
