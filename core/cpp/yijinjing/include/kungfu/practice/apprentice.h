/*****************************************************************************
 * Copyright [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

//
// Created by Keren Dong on 2019-06-01.
//

#ifndef KUNGFU_APPRENTICE_H
#define KUNGFU_APPRENTICE_H

#include <unordered_map>

#include <kungfu/yijinjing/io.h>

namespace kungfu {
    namespace practice {

        class apprentice : public yijinjing::event_source, public std::enable_shared_from_this<apprentice> {
        public:
            explicit apprentice(const std::string &name, bool low_latency = false);

            void setup_output(yijinjing::data::mode m, yijinjing::data::category c, const std::string &group, const std::string &name) override;

            void subscribe(yijinjing::data::mode m, yijinjing::data::category c, const std::string &group, const std::string &name) override;

            yijinjing::journal::writer_ptr get_writer() override { return writer_; };

            yijinjing::nanomsg::socket_ptr get_socket_reply() override { return socket_reply_; };
            yijinjing::nanomsg::socket_ptr get_socket_publish() override { return socket_publish_; };

            void add_event_handler(yijinjing::event_handler_ptr handler);

            void go();

            void stop() { live_ = false; };

            yijinjing::io_device& get_io_device() override { return io_device_; };

        private:
            yijinjing::io_device io_device_;
            std::vector<kungfu::yijinjing::event_handler_ptr> event_handlers_;
            yijinjing::journal::reader_ptr reader_;
            yijinjing::journal::writer_ptr writer_;
            yijinjing::nanomsg::socket_ptr socket_reply_;
            yijinjing::nanomsg::socket_ptr socket_publish_;
            std::unordered_map<std::string, yijinjing::nanomsg::socket_ptr> sub_sockets_;
            bool live_ = true;

            void try_once();
        };
    }
}

#endif //KUNGFU_APPRENTICE_H
