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

        hero::hero(yijinjing::io_device_with_reply_ptr io_device) :
                io_device_(std::move(io_device)), now_(0), begin_time_(time::now_in_nano()), end_time_(INT64_MAX)
        {
            if (spdlog::default_logger()->name().empty())
            {
                log::setup_log(io_device_->get_home(), io_device_->get_home()->name);
            }
            os::handle_os_signals(this);
            SPDLOG_DEBUG("creating {} low_latency {}", io_device_->get_home()->uname, io_device_->is_low_latency());
            reader_ = io_device_->open_reader_to_subscribe();
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

        writer_ptr hero::get_writer(uint32_t dest_id)
        {
            if (writers_.find(dest_id) == writers_.end())
            {
                return writer_ptr();
            }
            return writers_[dest_id];
        }

        void hero::run()
        {
            SPDLOG_INFO("{} observing", get_home_uname());

            events_ = observable<>::create<event_ptr>(
                    [&](subscriber<event_ptr> sb)
                    {
                        produce(sb);
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
                                    SPDLOG_INFO("kungfu app quit because {}", ex.what());
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

            react();

            events_.connect();

            on_exit();
            SPDLOG_INFO("{} finished", get_home_uname());
        }

        void hero::register_location(int64_t trigger_time, const location_ptr &location)
        {
            locations_[location->uid] = location;
            SPDLOG_INFO("registered location {:08x} {} at {}", location->uid, location->uname, time::strftime(trigger_time));
        }

        void hero::deregister_location(int64_t trigger_time, const uint32_t location_uid)
        {
            SPDLOG_INFO("deregistered location {:08x} {} at {}", location_uid, get_location(location_uid)->uname, time::strftime(trigger_time));
            locations_.erase(location_uid);
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

        void hero::require_read_from(uint32_t dest_id, int64_t trigger_time, uint32_t source_id, bool pub)
        {
            auto writer = get_writer(dest_id);
            auto msg_type = pub ? msg::type::RequestReadFromPublic : msg::type::RequestReadFrom;
            msg::data::RequestReadFrom &msg = writer->open_data<msg::data::RequestReadFrom>(trigger_time, msg_type);
            msg.source_id = source_id;
            msg.from_time = trigger_time;
            writer->close_data();
            SPDLOG_INFO("request {} [{:08x}] subscribe to {} [{:08x}]", get_location(dest_id)->uname, dest_id,
                        get_location(source_id)->uname, source_id);
        }

        void hero::produce(const rx::subscriber<yijinjing::event_ptr> &sb)
        {
            try
            {
                SPDLOG_INFO("{} generating events with source id {:08x}", get_home_uname(), get_home_uid());
                SPDLOG_INFO("from {} until {}", time::strftime(begin_time_), time::strftime(end_time_));
                while (live_)
                {
                    if (not produce_one(sb))
                    {
                        break;
                    }
                }
            } catch (...)
            {
                sb.on_error(std::current_exception());
            }
            sb.on_completed();
        }

        bool hero::produce_one(const rx::subscriber<yijinjing::event_ptr> &sb)
        {
            if (io_device_->get_home()->mode == mode::LIVE)
            {
                if (io_device_->get_observer()->wait())
                {
                    const std::string &notice = io_device_->get_observer()->get_notice();
                    now_ = time::now_in_nano();
                    if (notice.length() > 2)
                    {
                        SPDLOG_DEBUG("got notice {}", notice);
                        sb.on_next(std::make_shared<nanomsg_json>(notice));
                    } else
                    {
                        on_notify();
                    }
                }
                if (io_device_->get_rep_sock()->recv() > 0)
                {
                    const std::string &msg = io_device_->get_rep_sock()->last_message();
                    now_ = time::now_in_nano();
                    sb.on_next(std::make_shared<nanomsg_json>(msg));
                }
            }
            if (reader_->data_available())
            {
                if (reader_->current_frame()->gen_time() <= end_time_)
                {
                    now_ = reader_->current_frame()->gen_time();
                    sb.on_next(reader_->current_frame());
                    reader_->next();
                } else
                {
                    SPDLOG_INFO("reached journal end {}", time::strftime(reader_->current_frame()->gen_time()));
                    return false;
                }
            } else if (get_io_device()->get_home()->mode != mode::LIVE)
            {
                SPDLOG_INFO("reached journal end {}", time::strftime(reader_->current_frame()->gen_time()));
                return false;
            }
            return true;
        }
    }
}
