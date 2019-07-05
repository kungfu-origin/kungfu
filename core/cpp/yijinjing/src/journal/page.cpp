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

#include <sstream>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/util/os.h>
#include <kungfu/yijinjing/journal/page.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace journal
        {

            page::page(const data::location_ptr &location, uint32_t dest_id, const int id, const size_t size, const bool lazy, uintptr_t address) :
                    location_(location), dest_id_(dest_id), page_id_(id), size_(size), lazy_(lazy), header_(reinterpret_cast<page_header *>(address))
            {
                assert(address > 0);
            }

            void page::set_last_frame_position(uint64_t position)
            {
                const_cast<page_header *>(header_)->last_frame_position = position;
            }

            void page::release()
            {
                if (!os::release_mmap_buffer(address(), size_, lazy_))
                {
                    throw journal_error("failed to release memory for page " + get_page_path(location_, dest_id_, page_id_));
                }
            }

            page_ptr page::load(const data::location_ptr &location, uint32_t dest_id, int page_id, bool is_writing, bool lazy)
            {
                uint32_t page_size = find_page_size(location, dest_id);
                std::string path = get_page_path(location, dest_id, page_id);
                uintptr_t address = os::load_mmap_buffer(path, page_size, is_writing, lazy);
                if (address < 0)
                {
                    throw journal_error("unable to load page for " + path);
                }

                auto header = reinterpret_cast<page_header *>(address);

                if (header->last_frame_position == 0)
                {
                    header->version = __JOURNAL_VERSION__;
                    header->page_header_length = sizeof(page_header);
                    header->page_size = page_size;
                    header->frame_header_length = sizeof(frame_header);
                    header->last_frame_position = header->page_header_length;
                }

                if (header->version != __JOURNAL_VERSION__)
                {
                    throw journal_error(
                            fmt::format("version mismatch for page {}, required {}, found {}", path, __JOURNAL_VERSION__, header->version));
                }
                if (header->page_header_length != sizeof(page_header))
                {
                    throw journal_error(fmt::format("header length mismatch for page {}, required {}, found {}",
                                                    path, sizeof(page_header), header->page_header_length));
                }
                if (header->page_size != page_size)
                {
                    throw journal_error(fmt::format("page size mismatch, required {}, found {}", page_size, header->page_size));
                }

                return std::shared_ptr<page>(new page(location, dest_id, page_id, page_size, lazy, address));
            }

            std::string page::get_page_path(const data::location_ptr &location, uint32_t dest_id, int id)
            {
                return location->locator->layout_file(location, data::layout::JOURNAL, fmt::format("{:08x}.{}", dest_id, id));
            }

            int page::find_page_id(const data::location_ptr &location, uint32_t dest_id, int64_t time)
            {
                std::vector<int> page_ids = location->locator->list_page_id(location, dest_id);
                if (page_ids.empty())
                {
                    return 1;
                }
                if (time == 0)
                {
                    return page_ids.front();
                }
                for (int i = page_ids.size() - 1; i >= 0; i--)
                {
                    page_ptr page = page::load(location, dest_id, page_ids[i], false, true);
                    if (page->begin_time() < time)
                    {
                        SPDLOG_INFO("found page [{}] begin time {}", page_ids[i], time::strftime(page->begin_time()));
                        return page_ids[i];
                    }
                }
                return page_ids.front();
            }
        }
    }
}
