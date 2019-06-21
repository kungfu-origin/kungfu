//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_HERO_H
#define KUNGFU_HERO_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>

namespace kungfu
{
    namespace practice
    {
        class hero
        {
        public:
            explicit hero(yijinjing::io_device_ptr io_device);

            virtual ~hero()
            {}

            void run();

            void signal_stop()
            { live_ = false; };

            virtual void observe(const yijinjing::data::location_ptr location) = 0;

            inline yijinjing::io_device_ptr get_io_device() const
            { return io_device_; }

            inline yijinjing::journal::writer_ptr get_writer(uint32_t dest_id)
            { return writers_[dest_id]; }

            inline uint32_t get_home_uid() const
            { return get_io_device()->get_home()->uid; }

            inline const std::string &get_home_uname() const
            { return get_io_device()->get_home()->uname; }

            void register_location(const yijinjing::data::location_ptr &location);

            bool has_location(uint32_t hash);

            const yijinjing::data::location_ptr get_location(uint32_t hash);

            template<typename T>
            inline void write_to(int64_t trigger_time, int32_t msg_type, T &data, uint32_t dest_id = 0)
            {
                writers_[dest_id]->write(trigger_time, msg_type, get_home_uid(), data);
            }

        protected:
            yijinjing::journal::reader_ptr reader_;
            std::unordered_map<uint32_t, yijinjing::journal::writer_ptr> writers_;

            virtual void react(rx::observable <yijinjing::event_ptr> events) = 0;

        private:
            yijinjing::io_device_ptr io_device_;
            bool live_ = true;
            std::unordered_map<uint32_t, yijinjing::data::location_ptr> locations_;
        };
    }
}
#endif //KUNGFU_HERO_H
