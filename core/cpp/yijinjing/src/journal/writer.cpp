#include <utility>

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
#include <kungfu/yijinjing/msg.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {
            constexpr uint32_t PAGE_ID_TRANC    = 0xFFFF0000;
            constexpr uint32_t FRAME_ID_TRANC   = 0x0000FFFF;

            writer::writer(const data::location_ptr& location, uint32_t dest_id, bool lazy, publisher_ptr publisher) :
                    publisher_(std::move(publisher)), size_to_write_(0)
            {
                frame_id_base_ = location->uid;
                frame_id_base_ = frame_id_base_ << 32;
                journal_ = std::make_shared<journal>(location, dest_id, true, lazy);
                journal_->seek_to_time(time::now_in_nano());
            }

            uint64_t writer::current_frame_uid()
            {
                uint32_t page_part = (journal_->current_page_->page_id_ << 16) & PAGE_ID_TRANC;
                uint32_t frame_part = journal_->page_frame_nb_ & FRAME_ID_TRANC;
                return frame_id_base_ | (page_part | frame_part);
            }

            frame_ptr writer::open_frame(int64_t trigger_time, int32_t msg_type, uint32_t data_length)
            {
                assert(sizeof(frame_header) + data_length + sizeof(frame_header) <= journal_->current_page_->get_page_size());
                writer_mtx_.lock();
                if (journal_->current_frame()->address() + sizeof(frame_header) + data_length > journal_->current_page_->address_border())
                {
                    mark(trigger_time, msg::type::PageEnd);
                    journal_->load_next_page();
                }
                auto frame = journal_->current_frame();
                frame->set_header_length();
                frame->set_trigger_time(trigger_time);
                frame->set_msg_type(msg_type);
                frame->set_source(journal_->location_->uid);
                frame->set_dest(journal_->dest_id_);
                return frame;
            }

            void writer::close_frame(size_t data_length)
            {
                auto frame = journal_->current_frame();
                frame->set_gen_time(time::now_in_nano());
                frame->set_data_length(data_length);
                journal_->current_page_->set_last_frame_position(frame->address() - journal_->current_page_->address());
                journal_->next();
                writer_mtx_.unlock();
                publisher_->notify();
            }

            void writer::mark(int64_t trigger_time, int32_t msg_type)
            {
                open_frame(trigger_time, msg_type, 0);
                close_frame(0);
            }

            void writer::write_raw(int64_t trigger_time, int32_t msg_type, char *data, uint32_t length)
            {
                auto frame = open_frame(trigger_time, msg_type, length);
                memcpy(const_cast<void*>(frame->data_address()), data, length);
                close_frame(length);
            }
        }
    }
}
