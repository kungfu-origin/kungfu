//
// Created by Keren Dong on 2019-06-06.
//

#include <spdlog/spdlog.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>

#include <kungfu/yijinjing/nanomsg/master.h>

using namespace kungfu::yijinjing::nanomsg;

master_service::master_service(data::mode m, url_factory_ptr factory): socket_(protocol::REQUEST)
{
    auto url = factory->make_url_connect(m, data::category::SYSTEM, "master", "master", socket_.get_protocol());
    SPDLOG_DEBUG("ready to use master service [{}]", url);
    socket_.connect(url);
}

const std::string& master_service::request(const std::string &json_message)
{
    socket_.send(json_message);
    return socket_.recv_msg();
}

void master_messenger::poke()
{
    publisher_->poke();
}

slow_publisher::slow_publisher(data::mode m, url_factory_ptr factory): socket_(protocol::PUSH)
{
    auto url = factory->make_url_connect(m, data::category::SYSTEM, "master", "master", socket_.get_protocol());
    socket_.connect(url);
    SPDLOG_DEBUG("ready to publish and poke to master [{}]", url);
}

slow_publisher::~slow_publisher()
{
    SPDLOG_DEBUG("master publisher closing");
    socket_.close();
    SPDLOG_DEBUG("master publisher closed");
}

int slow_publisher::poke()
{
    return publish("{}");
}

int slow_publisher::publish(const std::string &json_message)
{
    return socket_.send(json_message);
}

slow_observer::slow_observer(data::mode m, url_factory_ptr factory, int timeout): socket_(protocol::SUBSCRIBE)
{
    auto url = factory->make_url_connect(m, data::category::SYSTEM, "master", "master", socket_.get_protocol());
    socket_.connect(url);
    socket_.setsockopt_int(NN_SOL_SOCKET, NN_RCVTIMEO, timeout);
    socket_.setsockopt_str(NN_SUB, NN_SUB_SUBSCRIBE, "");
    SPDLOG_DEBUG("observing master chanel with timeout {}ms [{}]", timeout, url);
}

slow_observer::~slow_observer()
{
    SPDLOG_DEBUG("master observer closing");
    socket_.close();
    SPDLOG_DEBUG("master observer closed");
}

bool slow_observer::wait_for_notice()
{
    try
    {
        int rc = socket_.recv();
        SPDLOG_TRACE("master_observer received {}", rc);
        return rc > 2;
    }
    catch(const nn_exception& e)
    {
        switch(e.num())
        {
            case ETIMEDOUT:
                break;
            default:
                throw e;
        }
        return false;
    }
}

const std::string& slow_observer::get_notice()
{
    return socket_.last_message();
}

