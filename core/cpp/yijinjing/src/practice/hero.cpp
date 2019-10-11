//
// Created by Keren Dong on 2019-06-15.
//

#include <utility>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/util/stacktrace.h>
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
            reader_ = io_device_->open_reader_to_subscribe();
        }

        bool hero::has_location(uint32_t hash)
        {
            return locations_.find(hash) != locations_.end();
        }

        location_ptr hero::get_location(uint32_t hash)
        {
            if (has_location(hash))
            {
                return locations_[hash];
            } else
            {
                throw yijinjing_error(fmt::format("location with uid {:08x} does not exist", hash));
            }
        }

        bool hero::has_channel(uint64_t hash) const
        {
            return channels_.find(hash) != channels_.end();
        }

        const yijinjing::msg::data::Channel& hero::get_channel(uint64_t hash) const
        {
            if(not this->has_channel(hash))
            {
                throw yijinjing_error(fmt::format("has no channel for [{:08x}]", hash));
            }
            return channels_.at(hash);
        }

        bool hero::has_writer(uint32_t dest_id)
        {
            return writers_.find(dest_id) != writers_.end();
        }

        writer_ptr hero::get_writer(uint32_t dest_id)
        {
            if (writers_.find(dest_id) == writers_.end())
            {
                if (has_location(dest_id))
                {
                    throw yijinjing_error(fmt::format("has no writer for [{:08x}] {}", dest_id, get_location(dest_id)->uname));
                } else
                {
                    throw yijinjing_error(fmt::format("has no writer for [{:08x}]", dest_id));
                }
            }
            return writers_[dest_id];
        }

        void hero::run()
        {
            SPDLOG_INFO("{} [{:08x}] running", get_home_uname(), get_home_uid());
            SPDLOG_INFO("from {} until {}", time::strftime(begin_time_), end_time_ == INT64_MAX ? "end of world" : time::strftime(end_time_));

            events_ = observable<>::create<event_ptr>(
                    [&, this](subscriber<event_ptr> sb)
                    {
                        delegate_produce(this, sb);
                    }) | on_error_resume_next(
                    [&](std::exception_ptr e) -> observable<event_ptr>
                    {
                        SPDLOG_ERROR("on error resume next");
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
                    }) | publish();

            react();

            events_.connect();

            on_exit();
            SPDLOG_INFO("{} finished", get_home_uname());
        }

        void hero::register_location(int64_t trigger_time, const location_ptr &location)
        {
            locations_[location->uid] = location;
            SPDLOG_INFO("registered location {} [{:08x}]", location->uname, location->uid);
        }

        void hero::deregister_location(int64_t trigger_time, const uint32_t location_uid)
        {
            if (has_location(location_uid))
            {
                auto location = get_location(location_uid);
                locations_.erase(location_uid);
                SPDLOG_INFO("deregister-ed location {} [{:08x}] {}", location->uname, location_uid, has_location(location_uid));
            } else
            {
                SPDLOG_ERROR("location [{:08x}] not exists", location_uid);
            }
        }

        void hero::register_channel(int64_t trigger_time, const yijinjing::msg::data::Channel &channel)
        {
            uint64_t channel_uid = uint64_t(channel.source_id) << 32 | channel.dest_id;
            channels_[channel_uid] = channel;
            SPDLOG_INFO("registered channel [{:08x}] from {} [{:08x}] to {} [{:08x}] ", channel_uid,
                    has_location(channel.source_id) ? get_location(channel.source_id)->uname : "unknown", channel.source_id,
                    has_location(channel.dest_id) ? get_location(channel.dest_id)->uname : "unknown", channel.dest_id);
        }

        void hero::deregister_channel(int64_t trigger_time, uint64_t channel_uid)
        {
            if (has_channel(channel_uid))
            {
                channels_.erase(channel_uid);
                SPDLOG_INFO("deregister-ed channel [{:08x}]", channel_uid);
            } else
            {
                SPDLOG_ERROR("channel [{:08x}] not exists", channel_uid);
            }
        }

        void hero::deregister_channel_by_source(uint32_t source_id)
        {
            auto channel_it = channels_.begin();
            while (channel_it != channels_.end())
            {
                if (channel_it->second.source_id == source_id)
                {
                    const auto& channel_uid = channel_it->first;
                    const auto& channel = channel_it->second;
                    SPDLOG_INFO("deregister-ed channel [{:08x}] from {} [{:08x}] to {} [{:08x}]", channel_uid,
                                has_location(channel.source_id) ? get_location(channel.source_id)->uname : "unknown", channel.source_id,
                                has_location(channel.dest_id) ? get_location(channel.dest_id)->uname : "unknown", channel.dest_id);
                    channel_it = channels_.erase(channel_it);
                }
                else
                {
                    channel_it++;
                }
            }
        }

        void hero::require_write_to(uint32_t source_id, int64_t trigger_time, uint32_t dest_id)
        {
            if (has_location(source_id))
            {
                auto writer = get_writer(source_id);
                msg::data::RequestWriteTo &msg = writer->open_data<msg::data::RequestWriteTo>(trigger_time, msg::type::RequestWriteTo);
                msg.dest_id = dest_id;
                writer->close_data();
                SPDLOG_INFO("request {} [{:08x}] publish to {} [{:08x}]", get_location(source_id)->uname, source_id,
                            dest_id == 0 ? "public" : get_location(dest_id)->uname, dest_id);
            } else
            {
                SPDLOG_ERROR("location [{:08x}] not exists", source_id);
            }
        }

        void hero::require_read_from(uint32_t dest_id, int64_t trigger_time, uint32_t source_id, bool pub)
        {
            if (has_location(dest_id))
            {
                auto writer = get_writer(dest_id);
                auto msg_type = pub ? msg::type::RequestReadFromPublic : msg::type::RequestReadFrom;
                msg::data::RequestReadFrom &msg = writer->open_data<msg::data::RequestReadFrom>(trigger_time, msg_type);
                msg.source_id = source_id;
                msg.from_time = trigger_time;
                writer->close_data();
                SPDLOG_INFO("request {} [{:08x}] subscribe to {} [{:08x}]", get_location(dest_id)->uname, dest_id,
                            get_location(source_id)->uname, source_id);
            } else
            {
                SPDLOG_ERROR("location [{:08x}] not exists", dest_id);
            }
        }

        void hero::produce(const rx::subscriber<yijinjing::event_ptr> &sb)
        {
            try
            {
                while (live_)
                {
                    live_ = produce_one(sb) && live_;
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
            while (reader_->data_available())
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
            }
            if (get_io_device()->get_home()->mode != mode::LIVE and not reader_->data_available())
            {
                SPDLOG_INFO("reached journal end {}", time::strftime(reader_->current_frame()->gen_time()));
                return false;
            }
            return true;
        }

        void hero::delegate_produce(hero *instance, const rx::subscriber<yijinjing::event_ptr> &sb)
        {
#ifdef _WINDOWS
            __try {
                    instance->produce(sb);
            } __except(yijinjing::util::print_stack_trace(GetExceptionInformation())) {}
#else
            instance->produce(sb);
#endif
        }
    }
}
