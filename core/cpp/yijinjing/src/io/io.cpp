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

#include <typeinfo>
#include <spdlog/spdlog.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>
#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/journal/page_service.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/io.h>

using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;
using namespace kungfu::yijinjing::nanomsg;

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {

            /**
             * local_page_provider,
             * provide local page, no need to connect with service.
             */
            class local_page_provider : public page_provider
            {
            public:
                local_page_provider(data::location &location, const bool is_writing, const bool lazy) :
                        page_provider(location, is_writing), lazy_(lazy)
                {};

                virtual page_ptr get_page(int new_page_id, int old_page_id)
                {
                    if (pages_.find(new_page_id) == pages_.end())
                    {
                        auto new_page = page::load(get_location(), new_page_id, is_writing_, lazy_);
                        pages_[new_page_id] = new_page;
                        page_refs_[new_page_id] = 1;
                    } else
                    {
                        page_refs_[new_page_id]++;
                    }
                    if (old_page_id >= 0)
                    {
                        release_page(old_page_id);
                    }
                    return pages_[new_page_id];
                }

                virtual void release_page(int page_id)
                {
                    page_refs_[page_id]--;
                    if (page_refs_[page_id] == 0)
                    {
                        pages_[page_id]->release();
                        pages_.erase(page_id);
                    }
                }

                virtual void release_all()
                {
                    std::vector<int> ids;
                    for (auto element : pages_)
                    {
                        ids.push_back(element.first);
                    }
                    for (auto page_id : ids)
                    {
                        release_page(page_id);
                    }
                }

            protected:
                const bool lazy_;
                std::unordered_map<int, page_ptr> pages_;
                std::unordered_map<int, int> page_refs_;
            };

            /**
             * client_page_provider,
             * provide page via memory service, socket & comm
             */
            class client_page_provider : public local_page_provider
            {
            public:
                client_page_provider(data::location &location, const bool is_writing,
                                     std::string client_name, master_service_ptr service, publisher_ptr publisher) :
                        local_page_provider(location, is_writing, true), client_name_(std::move(client_name)),
                        service_(std::move(service)), publisher_(std::move(publisher))
                {
                    SPDLOG_TRACE("client_page_provider {} request register", client_name);

#ifdef _WINDOWS
                    int pid = _getpid();
#else
                    int pid = getpid();
#endif
                    nlohmann::json request;
                    request["request"] = "journal/register";
                    request["pid"] = pid;
                    request["mode"] = location_.mode;
                    request["category"] = location_.category;
                    request["group"] = location_.group;
                    request["name"] = location_.name;
                    request["is_writing"] = is_writing;
                    std::string request_str = request.dump();
                    std::string response_str = service_->request(request_str);
                    SPDLOG_TRACE("got response: {}", response_str);
                    nlohmann::json response = nlohmann::json::parse(response_str);

                    if (response["success"])
                    {
                        int size = response["file_size"];
                        std::string memory_msg_file = response["memory_msg_file"];
                        memory_msg_id_ = response["memory_msg_id"];
                        memory_msg_buffer_ = os::load_mmap_buffer(memory_msg_file, size, true, false);
                    } else
                    {
                        std::string error_msg = response["error_msg"];
                        SPDLOG_ERROR("failed to register client {}, error: {}", client_name_, error_msg);
                        throw exception("cannot register client: " + client_name_);
                    }

                    paged::page_request *server_msg = paged::get_page_request(memory_msg_buffer_, memory_msg_id_);
                    if (server_msg->status != paged::state::WAITING)
                    {
                        throw exception("server buffer is not allocated: " + client_name_);
                    }
                }

                page_ptr get_page(int new_page_id, int old_page_id) override
                {
                    if (pages_.find(new_page_id) == pages_.end())
                    {
                        paged::page_request *server_msg = paged::get_page_request(memory_msg_buffer_, memory_msg_id_);
                        server_msg->new_page_id = new_page_id;
                        server_msg->old_page_id = old_page_id;
                        server_msg->status = paged::state::REQUESTING;

                        publisher_->notify();

                        while (server_msg->status == paged::state::REQUESTING)
                        {}

                        if (server_msg->status != paged::state::WAITING)
                        {
                            std::string path = page::get_page_path(get_location(), new_page_id);
                            throw exception("failed to request page " + path);
                        }

                        return local_page_provider::get_page(new_page_id, old_page_id);
                    } else
                    {
                        throw exception("client_page_provider does not support concurrent page request " + page::get_page_path(get_location(), new_page_id));
                    }
                }

                void release_page(int page_id) override
                {
                    local_page_provider::release_page(page_id);
                    if (page_refs_[page_id] == 0)
                    {
                        paged::page_request *server_msg = paged::get_page_request(memory_msg_buffer_, memory_msg_id_);
                        std::string path = page::get_page_path(get_location(), page_id);
                        server_msg->new_page_id = page_id;
                        server_msg->old_page_id = page_id;
                        server_msg->status = paged::state::RELEASING;

                        publisher_->notify();

                        while (server_msg->status == paged::state::RELEASING)
                        {}

                        if (server_msg->status != paged::state::WAITING)
                        {
                            throw exception("failed to release page " + path);
                        }
                    } else
                    {
                        throw exception("client_page_provider does not support concurrent page request " + page::get_page_path(get_location(), page_id));
                    }
                }

            private:
                std::string client_name_;
                master_service_ptr service_;
                publisher_ptr publisher_;
                int memory_msg_id_;
                uintptr_t memory_msg_buffer_;
            };
        }

        class buffered_page_provider_factory : public page_provider_factory
        {
        public:
            buffered_page_provider_factory()
            {}

            ~buffered_page_provider_factory()
            {
                providers_reading_.clear();
                providers_writing_.clear();
            }

            page_provider_ptr
            make_page_provider(mode m, category c, const std::string &group, const std::string &name, const bool is_writing) override
            {
                data::location location(m, c, group, name);
                auto providers = is_writing ? providers_writing_ : providers_reading_;
                if (providers.find(location.keyname()) == providers.end())
                {
                    auto new_provider = new_page_provider(location, is_writing);
                    providers[location.keyname()] = new_provider;
                    return new_provider;
                } else
                {
                    return providers[location.keyname()];
                }
            }

        protected:
            virtual page_provider_ptr new_page_provider(data::location &location, bool is_writing) = 0;

        private:
            std::unordered_map<std::string, page_provider_ptr> providers_writing_;
            std::unordered_map<std::string, page_provider_ptr> providers_reading_;
        };

        class page_provider_factory_local : public buffered_page_provider_factory
        {
        public:

            page_provider_ptr new_page_provider(data::location &location, const bool is_writing) override
            {
                return std::make_shared<local_page_provider>(location, is_writing, false);
            }
        };

        class page_provider_factory_client : public buffered_page_provider_factory
        {
        public:
            page_provider_factory_client(io_device_client_ptr io_device) : io_device_(io_device)
            {};

            page_provider_ptr new_page_provider(data::location &location, const bool is_writing) override
            {
                return std::make_shared<client_page_provider>(location, is_writing, io_device_->get_name(), io_device_->get_service(),
                                                              io_device_->get_publisher());
            }

        private:
            io_device_client_ptr io_device_;
        };

        class ipc_url_factory : public url_factory
        {
        public:

            const std::string make_url_bind(mode m, category c, const std::string &group, const std::string &name, protocol p) const override
            {
                std::string socket_dir = os::make_path({KF_DIR_SOCKET, get_category_name(c), group});
                return "ipc://" + socket_dir + "/" + name + "." + get_protocol_name(p);
            }

            const std::string make_url_connect(mode m, category c, const std::string &group, const std::string &name, protocol p) const override
            {
                std::string socket_dir = os::make_path({KF_DIR_SOCKET, get_category_name(c), group});
                return "ipc://" + socket_dir + "/" + name + "." + get_protocol_name(get_opposite_protol(p));
            }
        };

        class noop_publisher : public publisher
        {
        public:
            int notify() override
            {
                return 0;
            }

            int publish(const std::string &json_message) override
            {
                throw exception("noop publisher does not publish anything");
            }
        };

        class nanomsg_publisher : public publisher
        {
        public:
            nanomsg_publisher(url_factory_ptr factory, bool low_latency) : socket_(protocol::PUSH), low_latency_(low_latency)
            {
                auto url = factory->make_url_connect(data::mode::LIVE, data::category::SYSTEM, "master", "master", socket_.get_protocol());
                socket_.connect(url);
                SPDLOG_DEBUG("ready to publish and notify to master [{}]", url);
            }

            ~nanomsg_publisher() override
            {
                SPDLOG_DEBUG("master publisher closing");
                socket_.close();
                SPDLOG_DEBUG("master publisher closed");
            }

            int notify() override
            {
                return low_latency_ ? 0 : publish("{}");
            }

            int publish(const std::string &json_message) override
            {
                return socket_.send(json_message);
            }

        private:
            const bool low_latency_;
            socket socket_;
        };

        class nanomsg_observer : public observer
        {
        public:
            nanomsg_observer(url_factory_ptr factory, bool low_latency) : socket_(protocol::SUBSCRIBE)
            {
                int timeout = low_latency ? 0 : DEFAULT_NOTICE_TIMEOUT;
                auto url = factory->make_url_connect(data::mode::LIVE, data::category::SYSTEM, "master", "master", socket_.get_protocol());
                socket_.connect(url);
                socket_.setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
                socket_.setsockopt_str(NN_SUB, NN_SUB_SUBSCRIBE, "");
                SPDLOG_DEBUG("observing master chanel with timeout {}ms [{}]", timeout, url);
            }

            ~nanomsg_observer() override
            {
                SPDLOG_DEBUG("master observer closing");
                socket_.close();
                SPDLOG_DEBUG("master observer closed");
            }

            bool wait() override
            {
                try
                {
                    int rc = socket_.recv();
                    SPDLOG_TRACE("master_observer received {}", rc);
                    return rc > 2;
                }
                catch (const nn_exception &e)
                {
                    switch (e.num())
                    {
                        case ETIMEDOUT:
                            break;
                        default:
                            throw e;
                    }
                    return false;
                }
            }

            const std::string &get_notice() override
            {
                return socket_.last_message();
            }

        private:
            socket socket_;
        };


        class nanomsg_master_service : public master_service
        {
        public:
            nanomsg_master_service(url_factory_ptr factory) : socket_(protocol::REQUEST)
            {
                auto url = factory->make_url_connect(data::mode::LIVE, data::category::SYSTEM, "master", "master", socket_.get_protocol());
                SPDLOG_INFO("ready to use master service [{}]", url);
                socket_.connect(url);
            }

            ~nanomsg_master_service() override
            {
                socket_.close();
            }

            const std::string &request(const std::string &json_message) override
            {
                socket_.send(json_message);
                return socket_.recv_msg();
            }

        private:
            socket socket_;
        };


        io_device::io_device(const bool low_latency) : low_latency_(low_latency)
        {
            SPDLOG_DEBUG("creating io_device {}", low_latency);

            url_factory_ = std::make_shared<ipc_url_factory>();
        }

        io_device_ptr io_device::create_io_device(bool low_latency = false)
        {
            auto r = std::shared_ptr<io_device>(new io_device(low_latency));
            r->page_provider_factory_ = std::make_shared<page_provider_factory_local>();
            r->publisher_ = std::make_shared<noop_publisher>();
            return r;
        }

        reader_ptr io_device::open_reader_to_subscribe()
        {
            return std::make_shared<reader>(page_provider_factory_);
        }

        reader_ptr io_device::open_reader(mode m, category c, const std::string &group, const std::string &name)
        {
            auto r = std::make_shared<reader>(page_provider_factory_);
            r->subscribe(m, c, group, name, 0);
            return r;
        }

        writer_ptr io_device::open_writer(mode m, category c, const std::string &group, const std::string &name)
        {
            return std::make_shared<writer>(page_provider_factory_, m, c, group, name, publisher_);
        }

        socket_ptr io_device::connect_socket(mode m, category c, const std::string &group, const std::string &name, const protocol &p, int timeout)
        {
            socket_ptr s = std::make_shared<socket>(p);
            auto url = url_factory_->make_url_connect(m, c, group, name, p);
            s->connect(url);
            s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
            SPDLOG_INFO("connected socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), name, url, timeout);
            return s;
        }

        socket_ptr io_device::bind_socket(mode m, category c, const std::string &group, const std::string &name, const protocol &p, int timeout)
        {
            socket_ptr s = std::make_shared<socket>(p);
            auto url = url_factory_->make_url_bind(m, c, group, name, p);
            s->bind(url);
            s->setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
            SPDLOG_INFO("bind to socket [{}] {} at {} with timeout {}", nanomsg::get_protocol_name(p), name, url, timeout);
            return s;
        }

        io_device_client::io_device_client(std::string name, bool low_latency) : io_device(low_latency), name_(std::move(name))
    {
            SPDLOG_DEBUG("creating io_device_client {}", name);

            observer_ = std::make_shared<nanomsg_observer>(get_url_factory(), is_low_latency());

            service_ = std::make_shared<nanomsg_master_service>(get_url_factory());
        }

        io_device_client_ptr io_device_client::create_io_device(std::string name, bool low_latency = false)
        {
            auto r = std::shared_ptr<io_device_client>(new io_device_client(name, low_latency));
            r->page_provider_factory_ = std::make_shared<page_provider_factory_client>(r);
            r->publisher_ = std::make_shared<nanomsg_publisher>(r->get_url_factory(), low_latency);
            return r;
        }
    }
}


