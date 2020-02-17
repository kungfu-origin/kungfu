//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_HERO_H
#define KUNGFU_HERO_H

#include <unordered_map>

#include <kungfu/longfist/longfist.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::yijinjing::practice
{
    class hero : public resource
    {
    public:
        explicit hero(yijinjing::io_device_with_reply_ptr io_device);

        virtual ~hero() = default;

        bool is_usable() override
        { return io_device_->is_usable(); }

        void setup() override;

        virtual void on_notify()
        {}

        virtual void on_exit()
        {}

        void set_begin_time(int64_t begin_time)
        { begin_time_ = begin_time; }

        void set_end_time(int64_t end_time)
        { end_time_ = end_time; }

        void step();

        void run();

        bool is_live()
        { return live_; }

        void signal_stop()
        { live_ = false; };

        yijinjing::io_device_with_reply_ptr get_io_device() const
        { return io_device_; }

        uint32_t get_home_uid() const
        { return get_io_device()->get_home()->uid; }

        uint32_t get_live_home_uid() const
        { return get_io_device()->get_live_home()->uid; }

        const std::string &get_home_uname() const
        { return get_io_device()->get_home()->uname; }

        int64_t now()
        { return now_; }

        yijinjing::journal::reader_ptr get_reader() const
        { return reader_; }

        bool has_location(uint32_t hash);

        yijinjing::data::location_ptr get_location(uint32_t hash);

        bool has_writer(uint32_t dest_id);

        yijinjing::journal::writer_ptr get_writer(uint32_t dest_id);

        bool has_channel(uint64_t hash) const;

        const longfist::types::Channel &get_channel(uint64_t hash) const;

        std::unordered_map<uint64_t, longfist::types::Channel> &get_channels()
        { return channels_; }

    protected:
        int64_t begin_time_;
        int64_t end_time_;
        std::unordered_map<uint64_t, longfist::types::Channel> channels_;
        std::unordered_map<uint32_t, yijinjing::data::location_ptr> locations_;
        yijinjing::journal::reader_ptr reader_;
        std::unordered_map<uint32_t, yijinjing::journal::writer_ptr> writers_;
        rx::connectable_observable <event_ptr> events_;

        virtual void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location);

        virtual void deregister_location(int64_t trigger_time, uint32_t location_uid);

        virtual void register_channel(int64_t trigger_time, const longfist::types::Channel &channel);

        virtual void deregister_channel(int64_t trigger_time, uint64_t channel_uid);

        void deregister_channel_by_source(uint32_t source_id);

        void require_read_from(int64_t trigger_time, uint32_t dest_id, uint32_t source_id, int64_t from_time);

        void require_read_from_public(int64_t trigger_time, uint32_t dest_id, uint32_t source_id, int64_t from_time);

        void require_write_to(int64_t trigger_time, uint32_t source_id, uint32_t dest_id);

        void produce(const rx::subscriber <event_ptr> &sb);

        virtual bool produce_one(const rx::subscriber <event_ptr> &sb);

        virtual void react() = 0;

    private:
        yijinjing::io_device_with_reply_ptr io_device_;
        rx::composite_subscription cs_;
        int64_t now_;
        volatile bool continual_ = true;
        volatile bool live_ = false;

        inline bool check_location(uint32_t source_id, uint32_t dest_id)
        {
            if (not has_location(source_id))
            {
                SPDLOG_ERROR("source location [{:08x}] not exists", source_id);
                return false;
            }
            if (dest_id != 0 and not has_location(dest_id))
            {
                SPDLOG_ERROR("dest location [{:08x}] not exists", dest_id);
                return false;
            }
            return true;
        }

        template<typename T>
        std::enable_if_t<T::reflect, void> do_require_read_from(yijinjing::journal::writer_ptr &&writer,
                                                                int64_t trigger_time,
                                                                uint32_t dest_id, uint32_t source_id,
                                                                int64_t from_time)
        {
            if (not check_location(source_id, dest_id))
            {
                return;
            }
            T &msg = writer->template open_data<T>(trigger_time);
            msg.source_id = source_id;
            msg.from_time = from_time;
            writer->close_data();
            SPDLOG_INFO("require {} read from {} from {}",
                        get_location(dest_id)->uname, get_location(source_id)->uname, time::strftime(msg.from_time));
        }

        static void delegate_produce(hero *instance, const rx::subscriber <event_ptr> &sb);
    };
}
#endif //KUNGFU_HERO_H
