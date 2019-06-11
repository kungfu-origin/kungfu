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

#ifndef YIJINJING_FRAME_HPP
#define YIJINJING_FRAME_HPP

#include <kungfu/yijinjing/journal/common.h>

namespace kungfu {

    namespace yijinjing {

        namespace journal {

            struct frame_header {
                /** total frame length (including header and data body) */
                volatile int32_t length;
                /** generate time of the frame data */
                int64_t gen_time;
                /** trigger time for this frame, use for latency stats */
                int64_t trigger_time;
                /** msg type of the data in frame */
                int16_t msg_type;
                /** source (system id) of this frame */
                int16_t source;
                /** hash code of data part (not whole frame) */
                int32_t hash;
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
            class frame : public event {
            public:

                frame(frame_header *header): header_(header) {};
                frame(uintptr_t address): frame(reinterpret_cast<frame_header *>(address)) {};

                inline bool has_data() const {return header_->length > 0;}

                inline uintptr_t address() const {return reinterpret_cast<uintptr_t>(header_);}

                inline int32_t frame_length() const {return header_->length;}

                inline int32_t header_length() const {return sizeof(frame_header);}

                inline int32_t data_length() const {return frame_length() - header_length();}

                inline int64_t gen_time() const override {return header_->gen_time;}

                inline int64_t trigger_time() const override {return header_->trigger_time;}

                inline int16_t msg_type() const override {return header_->msg_type;}

                inline int16_t source() const override {return header_->source;}

                inline int32_t hashcode() const {return header_->hash;}

                inline void set_data_length(int32_t length) {header_->length = header_length() + length;}

                inline void set_source(int16_t source) {header_->source = source;}

                inline void set_gen_time(int64_t gen_time) {header_->gen_time = gen_time;}

                inline void set_trigger_time(int64_t trigger_time) {header_->trigger_time = trigger_time;}

                inline void set_hashcode(int32_t hashcode) {header_->hash = hashcode;}

                inline void set_msg_type(int16_t msg_type) {header_->msg_type = msg_type;}

                template <typename T>
                inline int copy_data(const T *data) {
                    int length = sizeof(T);
                    memcpy(reinterpret_cast<void *>(address() + header_length()), data, length);
                    return length;
                }

            protected:
                const void * data_address() const override { return reinterpret_cast<void *>(address() + header_length());}

            private:
//                page *page_;
                /** address with type,
                 * will keep moving forward until change page */
                frame_header *header_;

                inline void move_to(uintptr_t last_frame_address) {
                    header_ = reinterpret_cast<frame_header *>(last_frame_address);
                }

                inline void move_to_next() {
                    move_to(address() + frame_length());
                }

                friend class page;
            };
        }
    }
}

#endif //YIJINJING_FRAME_HPP
