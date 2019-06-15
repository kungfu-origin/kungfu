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
            explicit hero() = default;
            virtual ~hero() {}

            virtual yijinjing::journal::reader_ptr get_reader() = 0;
            virtual yijinjing::journal::writer_ptr get_writer() = 0;

            void register_location(const yijinjing::data::location_ptr location);
            const yijinjing::data::location_ptr get_location(uint32_t hash);

            void go();

            void stop() { live_ = false; };

        protected:
            virtual void try_once() = 0;

        private:
            yijinjing::io_device_ptr io_device_;
            bool live_ = true;
            std::unordered_map<uint32_t, yijinjing::data::location_ptr> locations_;
        };
    }
}
#endif //KUNGFU_HERO_H
