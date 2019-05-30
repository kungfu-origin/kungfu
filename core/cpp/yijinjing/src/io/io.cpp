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

#include <spdlog/spdlog.h>
#include <initializer_list>
#include <boost/filesystem.hpp>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/io.h>

using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu {

    namespace yijinjing {

        std::string get_kf_dir(std::string root, std::initializer_list<std::string> list) {
            boost::filesystem::path target_path = root;
            for (std::string dir : list) {
                target_path /= dir;
            }
            if (!boost::filesystem::exists(target_path)) {
                boost::filesystem::create_directories(target_path);
            }
            return target_path.string();
        }

        class io_device_util {
        public:
            io_device_util(io_device &desk) : io_device_(desk),
                                            root_dir_(kungfu::get_kungfu_home()),
                                            socket_root_dir_(get_kf_dir(kungfu::get_kungfu_home(), {KF_DIR_SOCKET})),
                                            journal_root_dir_(get_kf_dir(kungfu::get_kungfu_home(), {KF_DIR_JOURNAL})),
                                            log_root_dir_(get_kf_dir(kungfu::get_kungfu_home(), {KF_DIR_LOG})) {}

        protected:
            io_device &io_device_;
            const std::string root_dir_;
            const std::string socket_root_dir_;
            const std::string journal_root_dir_;
            const std::string log_root_dir_;
        };

        class io_device_util_locator : public locator, io_device_util {
        public:
            io_device_util_locator(io_device &desk) : io_device_util(desk) {};

            const std::string get_journal_dir(mode m, category c, const std::string &tag) const override {
                boost::filesystem::path journal_dir = journal_root_dir_;
                journal_dir /= data::get_mode_name(m);
                journal_dir /= data::get_category_name(c);
                journal_dir /= tag;
                if (!boost::filesystem::exists(journal_dir)) {
                    boost::filesystem::create_directories(journal_dir);
                }
                return journal_dir.string();
            }
        };

        class io_device_util_pp_factory_local : public page_provider_factory, io_device_util {
        public:
            io_device_util_pp_factory_local(io_device &desk, locator_ptr journal_locator) : io_device_util(desk), journal_locator_(journal_locator) {};

            page_provider_ptr make_page_provider(mode m, category c, const std::string &group, const std::string& name, const bool is_writing) const override {
                return std::make_shared<local_page_provider>(journal_locator_, m, c, group, name, is_writing, false);
            }

        private:
            locator_ptr journal_locator_;
        };

        class io_device_util_pp_factory_client : public page_provider_factory, io_device_util {
        public:
            io_device_util_pp_factory_client(io_device &desk, locator_ptr journal_locator) : io_device_util(desk), journal_locator_(journal_locator) {};

            page_provider_ptr make_page_provider(mode m, category c, const std::string &group, const std::string& name, const bool is_writing) const override {
                return std::make_shared<client_page_provider>(journal_locator_, m, c, group, name, is_writing, io_device_.get_name(), io_device_.get_messenger());
            }

        private:
            locator_ptr journal_locator_;
        };

        class io_device_util_su_factory : public url_factory, io_device_util {
        public:
            io_device_util_su_factory(io_device &desk) : io_device_util(desk) {};

            const std::string make_url_bind(mode m, category c, const std::string &group, const std::string &name, protocol p) const override {
                std::string socket_dir = get_kf_dir(socket_root_dir_, {get_category_name(c), group});
                return "ipc://" + socket_dir + "/" + name + "." + get_protocol_name(p);
            }

            const std::string make_url_connect(mode m, category c, const std::string &group, const std::string &name, protocol p) const override {
                std::string socket_dir = get_kf_dir(socket_root_dir_, {get_category_name(c), group});
                return "ipc://" + socket_dir + "/" + name + "." + get_protocol_name(get_opposite_protol(p));
            }
        };

        io_device::io_device(const std::string &name, bool low_latency, bool master) : name_(name), low_latency_(low_latency) {
            SPDLOG_DEBUG("creating io_device {}", name_);

            journal_locator_ = std::make_shared<io_device_util_locator>(*this);
            url_factory_ = std::make_shared<io_device_util_su_factory>(*this);

            if (master) {
                page_provider_factory_ = std::make_shared<io_device_util_pp_factory_local>(*this, journal_locator_);
                messenger_ = std::make_shared<master_messenger_noop>();
            }
            else {
                page_provider_factory_ = std::make_shared<io_device_util_pp_factory_client>(*this, journal_locator_);

                master_service_ptr service = std::make_shared<master_service>(mode::LIVE, url_factory_);
                master_publisher_ptr publisher;
                master_observer_ptr observer;
                if (low_latency) {
                    SPDLOG_INFO("Run in low latency mode, CPU busy spin");
                    publisher = std::make_shared<low_latency_publisher>(mode::LIVE, url_factory_);
                    observer = std::make_shared<low_latency_observer>(mode::LIVE, url_factory_);
                } else {
                    SPDLOG_INFO("Run in low CPU cost mode, latency might be high");
                    publisher = std::make_shared<slow_publisher>(mode::LIVE, url_factory_);
                    observer = std::make_shared<slow_observer>(mode::LIVE, url_factory_);
                }
                messenger_ = std::make_shared<master_messenger>(service, publisher, observer);
            }
        }

        reader_ptr io_device::open_reader_to_subscribe() {
            return std::make_shared<aggregate_reader>(page_provider_factory_);
        }

        reader_ptr io_device::open_reader(mode m, category c, const std::string &group, const std::string &name) {
            auto reader = std::make_shared<aggregate_reader>(page_provider_factory_);
            reader->subscribe(m, c, group, name, 0);
            return reader;
        }

        writer_ptr io_device::open_writer(mode m, category c, const std::string &group, const std::string &name) {
            return std::make_shared<writer>(page_provider_factory_, m, c, group, name, messenger_);
        }

        socket_ptr io_device::connect_socket(mode m, category c, const std::string &group, const std::string &name, const protocol &p, int timeout) {
            socket_ptr s = std::make_shared<socket>(p);
            auto url = url_factory_->make_url_connect(m, c, group, name, p);
            s->connect(url);
            s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
            SPDLOG_INFO("connected socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), name, url, timeout);
            return s;
        }

        socket_ptr io_device::bind_socket(mode m, category c, const std::string &group, const std::string &name, const protocol &p, int timeout) {
            socket_ptr s = std::make_shared<socket>(p);
            auto url = url_factory_->make_url_bind(m, c, group, name, p);
            s->bind(url);
            s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
            SPDLOG_INFO("bind to socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), name, url, timeout);
            return s;
        }
    }
}


