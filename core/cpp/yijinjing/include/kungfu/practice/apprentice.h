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
#include <kungfu/practice/hero.h>

namespace kungfu
{
    namespace practice
    {
        class apprentice : public hero
        {
        public:
            apprentice(yijinjing::data::location_ptr home, bool low_latency = false);

            void observe(const yijinjing::data::location_ptr &location, int64_t from_time);

            void request_write_to(int64_t trigger_time, uint32_t dest_id);

            void request_read_from(int64_t trigger_time, uint32_t source_id);

            inline uint32_t get_master_commands_uid()
            {
                return master_commands_location_->uid;
            }

            inline std::string get_config_db_file(const std::string &name)
            {
                return config_location_->locator->layout_file(config_location_, yijinjing::data::layout::SQLITE, name);
            }

            inline std::string get_app_db_file(const std::string &name)
            {
                auto home = get_io_device()->get_home();
                return home->locator->layout_file(home, yijinjing::data::layout::SQLITE, name);
            }

        protected:
            yijinjing::data::location_ptr config_location_;

            void react(const rx::observable<yijinjing::event_ptr> &events) override;

            virtual void start()
            {}

        private:
            yijinjing::data::location_ptr master_commands_location_;
        };
    }
}

#endif //KUNGFU_APPRENTICE_H
