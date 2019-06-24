//
// Created by Keren Dong on 2019-06-15.
//

#include <utility>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/practice/hero.h>

using namespace kungfu::rx;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu
{
    namespace practice
    {

        hero::hero(yijinjing::io_device_ptr io_device) : io_device_(std::move(io_device))
        {
            os::handle_os_signals();
            reader_ = io_device_->open_reader_to_subscribe();
        }

        yijinjing::journal::writer_ptr hero::get_writer(uint32_t dest_id)
        {
            if (writers_.find(dest_id) == writers_.end())
            {
                throw yijinjing_error(fmt::format("writer {:x} not exists", dest_id));
            }
            return writers_[dest_id];
        }

        void hero::register_location(const yijinjing::data::location_ptr &location)
        {
            locations_[location->uid] = location;
        }

        bool hero::has_location(uint32_t hash)
        {
            return locations_.find(hash) != locations_.end();
        }

        const yijinjing::data::location_ptr hero::get_location(uint32_t hash)
        {
            return locations_[hash];
        }

        void hero::run()
        {
            SPDLOG_INFO("{} observing", get_home_uname());

            auto events = observable<>::create<event_ptr>(
                    [&](subscriber<event_ptr> sb)
                    {
                        try
                        {
                            SPDLOG_INFO("{} generating events with source id {:08x}", get_home_uname(), get_home_uid());

                            while (live_)
                            {
                                if (io_device_->get_observer()->wait())
                                {
                                    std::string notice = io_device_->get_observer()->get_notice();
                                    SPDLOG_INFO("got notice {}", notice);
                                    auto event = std::make_shared<nanomsg_json>(notice);
                                    sb.on_next(event);
                                }
                                if (reader_->data_available())
                                {
                                    sb.on_next(reader_->current_frame());
                                    reader_->next();
                                }
                            }

                        } catch (...)
                        {
                            sb.on_error(std::current_exception());
                        }

                        sb.on_completed();

                    }) | on_error_resume_next(
                    [&](std::exception_ptr e) -> observable<event_ptr>
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
                                    return observable<>::empty<event_ptr>();
                                }
                                default:
                                {
                                    SPDLOG_ERROR("Unexpected nanomsg error: {}", ex.what());
                                    return observable<>::error<event_ptr>(ex);
                                }
                            }
                        }
                        catch (const std::exception &ex)
                        {
                            SPDLOG_ERROR("Unexpected exception: {}", ex.what());
                            return observable<>::error<event_ptr>(ex);
                        }
                    }) | finally(
                    [&]()
                    {
                        if (writers_.find(0) != writers_.end())
                        {
                            writers_[0]->close_session();
                        }

                    }) | publish();

            events | is(msg::type::RequestPublish) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestPublish &data = e->data<msg::data::RequestPublish>();
                  if (writers_.find(data.dest_id) == writers_.end())
                  {
                      writers_[data.dest_id] = get_io_device()->open_writer(data.dest_id);
                      if (data.dest_id == 0)
                      {
                          writers_[data.dest_id]->open_session();
                      }
                  } else
                  {
                      SPDLOG_ERROR("{} [{:08x}] asks publish to {} [{:08x}] for more than once", get_location(e->source())->uname, e->source(),
                              get_location(data.dest_id)->uname, data.dest_id);
                  }
              });

            events | is(msg::type::RequestSubscribe) |
            $([&](event_ptr e)
              {
                  const msg::data::RequestSubscribe &data = e->data<msg::data::RequestSubscribe>();
                  reader_->join(get_location(data.source_id), get_home_uid(), data.from_time);
              });

            react(events);

            events.connect();
            SPDLOG_INFO("{} finished", get_home_uname());
        }

        void hero::request_publish(uint32_t source_id, int64_t trigger_time, uint32_t dest_id)
        {
            auto writer = get_writer(source_id);
            msg::data::RequestPublish &msg = writer->open_data<msg::data::RequestPublish>(trigger_time, msg::type::RequestPublish);
            msg.dest_id = dest_id;
            writer->close_data();
            SPDLOG_INFO("request {} [{:08x}] publish to {} [{:08x}]", get_location(source_id)->uname, source_id,
                    dest_id == 0 ? "public" : get_location(dest_id)->uname, dest_id);
        }

        void hero::request_subscribe(uint32_t dest_id, int64_t trigger_time, uint32_t source_id)
        {
            auto writer = get_writer(dest_id);
            msg::data::RequestSubscribe &msg = writer->open_data<msg::data::RequestSubscribe>(trigger_time, msg::type::RequestSubscribe);
            msg.source_id = source_id;
            writer->close_data();
            SPDLOG_INFO("request {} [{:08x}] subscribe to {} [{:08x}]", get_location(dest_id)->uname, dest_id,
                    get_location(source_id)->uname, source_id);
        }
    }
}
