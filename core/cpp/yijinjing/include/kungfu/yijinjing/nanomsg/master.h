//
// Created by Keren Dong on 2019-06-06.
//

#ifndef KUNGFU_NANOMSG_MASTER_H
#define KUNGFU_NANOMSG_MASTER_H

#include <kungfu/yijinjing/nanomsg/socket.h>


namespace kungfu {

    namespace yijinjing {

        namespace nanomsg {

            class master_service {
            public:
                master_service(data::mode m, url_factory_ptr factory);

                ~master_service() { socket_.close(); };

                const std::string &request(const std::string &json_message);

            private:
                socket socket_;
            };
            DECLARE_PTR(master_service)

            class master_publisher {
            public:
                virtual ~master_publisher() {};

                virtual int poke() = 0;

                virtual int publish(const std::string &json_message) = 0;
            };
            DECLARE_PTR(master_publisher)

            class master_observer {
            public:
                virtual ~master_observer() {};

                virtual bool wait_for_notice() = 0;

                virtual const std::string &get_notice() = 0;
            };
            DECLARE_PTR(master_observer)

            class master_messenger {
            public:
                master_messenger(master_service_ptr service, master_publisher_ptr publisher, master_observer_ptr observer): service_(service), publisher_(publisher), observer_(observer) {};
                virtual void poke();
                master_service_ptr get_service() {return service_;};
                master_publisher_ptr get_publisher() {return publisher_;};
                master_observer_ptr get_observer() {return observer_;};
            private:
                master_service_ptr service_;
                master_publisher_ptr publisher_;
                master_observer_ptr observer_;
            };
            DECLARE_PTR(master_messenger)

            class master_messenger_noop : public master_messenger {
            public:
                master_messenger_noop(): master_messenger(master_service_ptr(), master_publisher_ptr(), master_observer_ptr()) {};
                void poke() override {};
            };

            class slow_publisher : public master_publisher {
            public:
                slow_publisher(data::mode m, url_factory_ptr factory);

                ~slow_publisher() override;

                int poke() override;

                int publish(const std::string &msg) override;

            private:
                socket socket_;
            };

            class slow_observer : public master_observer {
            public:
                slow_observer(data::mode m, url_factory_ptr factory, int timeout = DEFAULT_NOTICE_TIMEOUT);

                ~slow_observer() override;

                bool wait_for_notice() override;

                const std::string &get_notice() override;

            private:
                socket socket_;
                int timeout_;
            };

            class low_latency_publisher : public slow_publisher {
            public:
                low_latency_publisher(data::mode m, url_factory_ptr factory) : slow_publisher(m, factory) {};

                inline int poke() override { return 0; };
            };

            class low_latency_observer : public slow_observer {
            public:
                low_latency_observer(data::mode m, url_factory_ptr factory) : slow_observer(m, factory, 0) {};
            };
        }
    }
}

#endif //KUNGFU_NANOMSG_MASTER_H
