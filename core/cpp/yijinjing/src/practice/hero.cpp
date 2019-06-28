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

        hero::hero(yijinjing::io_device_ptr io_device) : io_device_(std::move(io_device)), last_check_(0)
        {
            os::handle_os_signals(this);
            reader_ = io_device_->open_reader_to_subscribe();
        }

        writer_ptr hero::get_writer(uint32_t dest_id)
        {
            if (writers_.find(dest_id) == writers_.end())
            {
                return writer_ptr();
            }
            return writers_[dest_id];
        }

        void hero::register_location(const location_ptr &location)
        {
            locations_[location->uid] = location;
        }

        void hero::deregister_location(const uint32_t location_uid)
        {
            locations_.erase(location_uid);
        }

        bool hero::has_location(uint32_t hash)
        {
            return locations_.find(hash) != locations_.end();
        }

        bool hero::has_location(mode m, category c, const std::string &group, const std::string &name)
        {
            location loc(m, c, group, name, get_io_device()->get_home()->locator);
            return has_location(loc.uid);
        }

        const location_ptr hero::get_location(uint32_t hash)
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
                                    const std::string &notice = io_device_->get_observer()->get_notice();
                                    if (notice.length() > 2)
                                    {
                                        SPDLOG_DEBUG("got notice {}", notice);
                                        sb.on_next(std::make_shared<nanomsg_json>(notice));
                                    } else
                                    {
                                        on_notify();
                                    }
                                }
                                if (reader_->data_available())
                                {
                                    sb.on_next(reader_->current_frame());
                                    reader_->next();
                                }
                                if (io_device_->get_rep_sock()->recv() > 0)
                                {
                                    const std::string &msg = io_device_->get_rep_sock()->last_message();
                                    sb.on_next(std::make_shared<nanomsg_json>(msg));
                                }
                                auto now = time::now_in_nano();
                                if (last_check_ + time_unit::NANOSECONDS_PER_SECOND < now)
                                {
                                    on_interval_check(now);
                                    last_check_ = now;
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
                            writers_[0]->mark(time::now_in_nano(), msg::type::SessionEnd);
                        }

                    }) | publish();

            react(events);

            events.connect();
            SPDLOG_INFO("{} finished", get_home_uname());
        }

        void hero::require_write_to(uint32_t source_id, int64_t trigger_time, uint32_t dest_id)
        {
            auto writer = get_writer(source_id);
            msg::data::RequestWriteTo &msg = writer->open_data<msg::data::RequestWriteTo>(trigger_time, msg::type::RequestWriteTo);
            msg.dest_id = dest_id;
            writer->close_data();
            SPDLOG_INFO("request {} [{:08x}] publish to {} [{:08x}]", get_location(source_id)->uname, source_id,
                        dest_id == 0 ? "public" : get_location(dest_id)->uname, dest_id);
        }

        void hero::require_read_from(uint32_t dest_id, int64_t trigger_time, uint32_t source_id)
        {
            auto writer = get_writer(dest_id);
            msg::data::RequestReadFrom &msg = writer->open_data<msg::data::RequestReadFrom>(trigger_time, msg::type::RequestReadFrom);
            msg.source_id = source_id;
            msg.from_time = trigger_time;
            writer->close_data();
            SPDLOG_INFO("request {} [{:08x}] subscribe to {} [{:08x}]", get_location(dest_id)->uname, dest_id,
                        get_location(source_id)->uname, source_id);
        }
    }
}
