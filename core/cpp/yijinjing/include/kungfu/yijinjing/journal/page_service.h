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

/**
 * Memory service for YiJinJing
 * Provide centralized memory mapped file "load and lock" service
 * Enable manual controlling on the whole kungfu system via system journal
 */

#ifndef YIJINJING_PAGEENGINE_H
#define YIJINJING_PAGEENGINE_H

#include <kungfu/yijinjing/io.h>

namespace kungfu {

    namespace yijinjing {

        namespace journal {

            namespace paged {

                enum class state : int8_t {
                    IDLE = 0,                       /**< this msg block is not allocated (default) */
                    WAITING = 1,                  /**< comm msg idx occupied (by server) */
                    REQUESTING = 2,                /**< page number specified (by client) */
                    RELEASING = 3                /**< page number specified (by client) */
                };

                /*
                 * PROCESS:
                 *
                 *      * default:                          -- PAGE_COMM_RAW
                 *
                 *      user:   socket request
                 *      server: allocate idx                -> PAGE_COMM_ALLOCATED
                 *      user:   setup folder / short_name_    -> PAGE_HOLDING
                 *
                 *      user:   fill in page_id            -> REQUESTING
                 *      server: find requesting, allocate
                 *              finish allocating           -> ALLOCATED (success)
                 *                                          -> PAGE_NON_EXIST (reader wants a not existing page)
                 *                                          -> PAGE_MEM_OVERFLOW (current locking memory overflows)
                 *
                 */
                struct page_request {
                    /** PagedCommTypeConstants (by both server and client) */
                    volatile state status;
                    int new_page_id;
                    int old_page_id;
                };

                inline page_request *get_page_request(uintptr_t buffer, int idx) {
                    return reinterpret_cast<page_request *>(buffer + idx * sizeof(page_request));
                }

                class page_service {
                public:
                    /** default constructor */
                    page_service(bool low_latency);

                    io_device &get_io_device() { return io_device_; };

                    std::string get_memory_msg_file();

                    size_t get_memory_msg_file_size();

                    int get_mm_block(int8_t mode, int8_t category, const std::string &group, const std::string &name, bool is_writing);

                    void release_mm_block(int id);

                    void process_memory_message();

                private:
                    std::map<int, page_provider_ptr> page_providers_;
                    std::map<std::string, bool> writers_;
                    io_device io_device_;

                    uintptr_t memory_message_buffer_; /**< message buffer */
                    const size_t memory_msg_file_size_ = MAX_MEMORY_MSG_NUMBER * sizeof(page_request) + 1024;
                    size_t memory_message_limit_;     /**< max index of current assigned memory message block */
                    std::string memory_msg_file_;   /**< comm file linked to memory */
                };
            }
        }
    }
}

#endif //YIJINJING_PAGEENGINE_H
