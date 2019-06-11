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

#include <mutex>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {
            writer::writer(page_provider_factory_ptr factory, data::mode m, data::category c, const std::string &group, const std::string &name,
                           publisher_ptr publisher) :
                    publisher_(publisher)
            {
                page_provider_ptr page_provider = factory->make_page_provider(m, c, group, name, true);
                journal_ = std::make_shared<journal>(page_provider);
                journal_->seek_to_time(time::now_in_nano());
            }

            frame &writer::open_frame(int64_t trigger_time, int16_t msg_type, int16_t source)
            {
                writer_mtx_.lock();
                frame &frame = journal_->current_frame();
                frame.set_header_length();
                frame.set_trigger_time(trigger_time);
                frame.set_msg_type(msg_type);
                frame.set_source(source);
                return frame;
            }

            void writer::close_frame(size_t data_length)
            {
                frame &frame = journal_->current_frame();
                frame.set_hashcode(util::hash(reinterpret_cast<void *>(frame.address()), frame.frame_length(), KUNGFU_HASH_SEED));
                frame.set_gen_time(time::now_in_nano());
                frame.set_data_length(data_length);
                journal_->current_page_->set_last_frame_position(frame.address() - journal_->current_page_->address());
                journal_->next();
                writer_mtx_.unlock();
                publisher_->notify();
            }

            void writer::write_raw(int64_t trigger_time, int16_t msg_type, int16_t source, char *data, int32_t length)
            {
                frame &frame = open_frame(trigger_time, msg_type, source);
                memcpy(const_cast<void*>(frame.data_address()), data, length);
                close_frame(length);
            }

            void writer::open_session()
            {
                open_frame(time::now_in_nano(), MsgType::SessionStart, 0);
                close_frame(1);
            }

            void writer::close_session()
            {
                open_frame(time::now_in_nano(), MsgType::SessionEnd, 0);
                close_frame(1);
            }
        }
    }
}
