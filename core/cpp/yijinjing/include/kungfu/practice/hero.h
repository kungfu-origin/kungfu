//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_HERO_H
#define KUNGFU_HERO_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>

namespace kungfu {
    namespace practice {

        class hero : public yijinjing::event_source {
        public:
            explicit hero(yijinjing::io_device_ptr io_device);

            virtual ~hero() {}

            yijinjing::io_device_ptr get_io_device() const override
            { return io_device_; }

            yijinjing::journal::writer_ptr get_writer(uint32_t dest_id) override
            { return writers_[dest_id]; }

            inline uint32_t get_home_uid() const
            { return get_io_device()->get_home()->uid; }

            void register_location(const yijinjing::data::location_ptr location);

            const yijinjing::data::location_ptr get_location(uint32_t hash);

            inline void add_event_handler(yijinjing::event_handler_ptr handler)
            {
                event_handlers_.push_back(handler);
            }

            void pre_run();

            void run();

            void post_run();

            void stop() { live_ = false; };

        protected:
            void try_once();

            yijinjing::journal::reader_ptr reader_;
            std::unordered_map<uint32_t, yijinjing::journal::writer_ptr> writers_;

        private:
            yijinjing::io_device_ptr io_device_;
            bool live_ = true;
            std::unordered_map<uint32_t, yijinjing::data::location_ptr> locations_;
            std::vector<kungfu::yijinjing::event_handler_ptr> event_handlers_;
        };
    }
}
#endif //KUNGFU_HERO_H
