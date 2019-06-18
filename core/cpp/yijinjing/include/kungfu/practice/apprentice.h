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
            explicit apprentice(yijinjing::data::location_ptr home, bool low_latency = false);

            void subscribe(yijinjing::data::location_ptr location) override;

        protected:
            void rx_subscribe(rx::observable<yijinjing::event_ptr> events) override ;

        private:
            yijinjing::data::location_ptr master_location_;
        };
    }
}

#endif //KUNGFU_APPRENTICE_H
