//
// Created by Keren Dong on 2019-06-15.
//

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/log/setup.h>

#include <kungfu/practice/hero.h>

namespace kungfu {
    namespace practice {

        void hero::register_location(const yijinjing::data::location_ptr location)
        {
            locations_[location->hash()] = location;
        }

        const yijinjing::data::location_ptr hero::get_location(uint32_t hash)
        {
            return locations_[hash];
        }

        void hero::go()
        {
            try
            {
                while (live_)
                {
                    try_once();
                }
            }
            catch (const yijinjing::nanomsg::nn_exception &e)
            {
                switch (e.num())
                {
                    case EINTR:
                    case EAGAIN:
                    case ETIMEDOUT:
                    {
                        SPDLOG_INFO("apprentice quit because {}", e.what());
                        break;
                    }
                    default:
                    {
                        SPDLOG_ERROR("Unexpected nanomsg error: {}", e.what());
                    }
                }
            }
            catch (const std::runtime_error &e)
            {
                SPDLOG_ERROR("Unexpected runtime error: {}", e.what());
            }
            catch (const std::exception &e)
            {
                SPDLOG_ERROR("Unexpected exception: {}", e.what());
            }
            SPDLOG_INFO("apprentice {} finished", io_device_->get_home()->journal_path());
        }
    }
}
