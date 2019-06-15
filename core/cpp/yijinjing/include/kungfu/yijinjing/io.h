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

#ifndef KUNGFU_IO_H
#define KUNGFU_IO_H

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/nanomsg/socket.h>

namespace kungfu
{

    namespace yijinjing
    {

        FORWARD_DECLARE_PTR(io_device)

        FORWARD_DECLARE_PTR(io_device_client)

        class event_source
        {
        public:
            virtual ~event_source() = default;

            virtual void subscribe(const data::location_ptr location) = 0;

            virtual journal::writer_ptr get_writer(uint32_t dest_id) = 0;

            virtual nanomsg::socket_ptr get_socket_reply() = 0;

            virtual nanomsg::socket_ptr get_socket_publish() = 0;

            virtual io_device_client_ptr get_io_device() = 0;
        };

        DECLARE_PTR(event_source)

        class event_handler
        {
        public:
            virtual ~event_handler() = default;

            virtual const std::string &get_name() const = 0;

            virtual void configure_event_source(event_source_ptr event_source) = 0;

            virtual void handle(const event *e) = 0;

            virtual void finish()
            {}
        };

        DECLARE_PTR(event_handler)

        class io_device
        {
        public:

            data::location_ptr get_home() { return home_; }

            bool is_low_latency()
            { return low_latency_; }

            journal::reader_ptr open_reader_to_subscribe();

            journal::reader_ptr open_reader(const data::location_ptr location, uint32_t dest_id);

            journal::writer_ptr open_writer(uint32_t dest_id);

            nanomsg::socket_ptr
            connect_socket(const data::location_ptr location, const nanomsg::protocol &p, int timeout = 0);

            nanomsg::socket_ptr
            bind_socket(const data::location_ptr location, const nanomsg::protocol &p, int timeout = 0);

            nanomsg::url_factory_ptr get_url_factory()
            { return url_factory_; }

            publisher_ptr get_publisher()
            { return publisher_; }

            static io_device_ptr create_io_device(data::location_ptr home, bool low_latency);

        protected:
            data::location_ptr home_;
            const bool low_latency_;
            const bool lazy_;
            nanomsg::url_factory_ptr url_factory_;
            publisher_ptr publisher_;

            io_device(data::location_ptr home, bool low_latency, bool lazy = false);
        };

        class master_service
        {
        public:
            virtual ~master_service() = default;

            virtual const std::string &request(const std::string &json_message) = 0;
        };

        DECLARE_PTR(master_service)

        class io_device_client : public io_device
        {
        public:

            observer_ptr get_observer()
            { return observer_; }

            master_service_ptr get_service()
            { return service_; }

            static io_device_client_ptr create_io_device(data::location_ptr home, bool low_latency);

        private:
            observer_ptr observer_;
            master_service_ptr service_;

            io_device_client(data::location_ptr home, bool low_latency);
        };
    }
}
#endif //KUNGFU_IO_H
