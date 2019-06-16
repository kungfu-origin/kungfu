//
// Created by Keren Dong on 2019-06-15.
//

#ifndef KUNGFU_MASTER_H
#define KUNGFU_MASTER_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>
#include <kungfu/practice/hero.h>

namespace kungfu {
    namespace practice {

        class master : public hero {
        public:
            master(yijinjing::data::location_ptr home, bool low_latency = false);

            void register_event_source(uint32_t source_id);

            yijinjing::io_device_master_ptr get_io_device() { return io_device_; };

        protected:
            void try_once() override ;

        private:
            yijinjing::io_device_master_ptr io_device_;
            yijinjing::journal::reader_ptr reader_;
            yijinjing::nanomsg::socket_ptr service_socket_;
            yijinjing::nanomsg::socket_ptr pull_socket_;
            std::unordered_map<uint32_t, yijinjing::journal::writer_ptr> writers_;
        };
    }
}
#endif //KUNGFU_MASTER_H
