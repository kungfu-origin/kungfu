//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_HERO_H
#define KUNGFU_HERO_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu
{
    namespace practice
    {
        class hero
        {
        public:
            explicit hero(yijinjing::io_device_with_reply_ptr io_device);

            virtual ~hero()
            {}

            virtual void on_notify()
            {}

            virtual void on_exit()
            {}

            void set_begin_time(int64_t begin_time)
            { begin_time_ = begin_time; }

            void set_end_time(int64_t end_time)
            { end_time_ = end_time; }

            void run();

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

            bool has_location(uint32_t hash);

            bool has_location(yijinjing::data::mode m, yijinjing::data::category c, const std::string &group, const std::string &name);

            const yijinjing::data::location_ptr get_location(uint32_t hash);

            yijinjing::journal::writer_ptr get_writer(uint32_t dest_id);

            template<typename T>
            inline void write_to(int64_t trigger_time, int32_t msg_type, T &data, uint32_t dest_id = 0)
            {
                writers_[dest_id]->write(trigger_time, msg_type, get_home_uid(), data);
            }

        protected:
            std::unordered_map<uint32_t, yijinjing::data::location_ptr> locations_;
            yijinjing::journal::reader_ptr reader_;
            std::unordered_map<uint32_t, yijinjing::journal::writer_ptr> writers_;
            int64_t begin_time_;
            int64_t end_time_;
            int64_t now_;
            rx::connectable_observable<yijinjing::event_ptr> events_;

            virtual void register_location(int64_t trigger_time, const yijinjing::data::location_ptr &location);

            virtual void deregister_location(int64_t trigger_time, uint32_t location_uid);

            void require_write_to(uint32_t source_id, int64_t trigger_time, uint32_t dest_id);

            void require_read_from(uint32_t dest_id, int64_t trigger_time, uint32_t source_id, bool pub);

            virtual void produce(const rx::subscriber<yijinjing::event_ptr> &sb);

            virtual bool produce_one(const rx::subscriber<yijinjing::event_ptr> &sb);

            virtual void react() = 0;

        private:
            yijinjing::io_device_with_reply_ptr io_device_;
            bool live_ = true;
        };
    }
}
#endif //KUNGFU_HERO_H
