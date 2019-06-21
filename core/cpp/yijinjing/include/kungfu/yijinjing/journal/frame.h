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

#ifndef KUNGFU_YIJINJING_FRAME_H
#define KUNGFU_YIJINJING_FRAME_H

#include <kungfu/yijinjing/journal/common.h>

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {

            struct frame_header
            {
                /** total frame length (including header and data body) */
                volatile int32_t length;
                /** header length */
                int32_t header_length;
                /** generate time of the frame data */
                int64_t gen_time;
                /** trigger time for this frame, use for latency stats */
                int64_t trigger_time;
                /** msg type of the data in frame */
                int32_t msg_type;
                /** source (system id) of this frame */
                uint32_t source;
#ifndef _WIN32
            } __attribute__((packed));
#else
            };
#pragma pack(pop)
#endif

            /**
             * Basic memory unit,
             * holds header / data / errorMsg (if needs)
             */
            class frame : public event
            {
            public:

                ~frame() override = default;

                inline bool has_data() const
                { return header_->length > 0; }

                inline uintptr_t address() const
                { return reinterpret_cast<uintptr_t>(header_); }

                inline int32_t frame_length() const
                { return header_->length; }

                inline int32_t header_length() const
                { return header_->header_length; }

                inline int32_t data_length() const override
                { return frame_length() - header_length(); }

                inline int64_t gen_time() const override
                { return header_->gen_time; }

                inline int64_t trigger_time() const override
                { return header_->trigger_time; }

                inline int32_t msg_type() const override
                { return header_->msg_type; }

                inline uint32_t source() const override
                { return header_->source; }

                template<typename T>
                inline size_t copy_data(const T& data)
                {
                    size_t length = sizeof(T);
                    memcpy(const_cast<void *>(data_address()), &data, length);
                    return length;
                }

            protected:
                const void *data_address() const override
                { return reinterpret_cast<void *>(address() + header_length()); }

            private:

                /** address with type,
                 * will keep moving forward until change page */
                frame_header *header_;

                frame() = default;

                inline void set_address(uintptr_t address)
                { header_ = reinterpret_cast<frame_header *>(address); }

                inline void move_to_next()
                { set_address(address() + frame_length()); }

                inline void set_header_length()
                {
                    header_->header_length = sizeof(frame_header);
                }

                inline void set_data_length(int32_t length)
                {
                    assert(length > 0);
                    header_->length = header_length() + length;
                }

                inline void set_source(int16_t source)
                { header_->source = source; }

                inline void set_gen_time(int64_t gen_time)
                { header_->gen_time = gen_time; }

                inline void set_trigger_time(int64_t trigger_time)
                { header_->trigger_time = trigger_time; }

                inline void set_msg_type(int16_t msg_type)
                { header_->msg_type = msg_type; }

                friend class journal;

                friend class writer;
            };
        }
    }
}

#endif //KUNGFU_YIJINJING_FRAME_H
