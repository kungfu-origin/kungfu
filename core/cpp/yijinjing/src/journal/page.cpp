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
#include <regex>
#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>

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

            page::page(const data::location &location, const int id, const size_t size, const bool lazy, page_header *header) :
                    location_(location), id_(id), size_(size), lazy_(lazy), header_(header),
                    frame_count_(header->frame_count),
                    frame_(address() + sizeof(page_header))
            {}

            void page::seek_next()
            {
                frame_.move_to_next();
            }

            void page::seek_begin()
            {
                frame_.move_to(address() + sizeof(page_header));
            }

            void page::seek_end()
            {
                frame_.move_to(address() + header_->last_frame_position);
            }

            bool page::seek_to_writable()
            {
                while (frame_.has_data())
                {
                    frame_.move_to_next();
                }
                return frame_.address() + frame_.frame_length() < address_border();
            }

            bool page::seek_to_time(int64_t nanotime)
            {
                SPDLOG_DEBUG("seek to time {}, frame at {}", time::strftime(nanotime), frame_.address());
                SPDLOG_DEBUG("page begin time {}, frame_count   {}", time::strftime(begin_time()), header_->frame_count);
                SPDLOG_DEBUG("page end time   {}, last frame    {}", time::strftime(end_time()), header_->last_frame_position);
                if (end_time() < nanotime)
                {
                    seek_end();
                }
                while (frame_.has_data() && frame_.gen_time() < nanotime)
                {
                    SPDLOG_DEBUG("frame {}", time::strftime(frame_.gen_time()));
                    frame_.move_to_next();
                }
                SPDLOG_DEBUG("seeked frame {}, address  {}", time::strftime(frame_.gen_time()), frame_.address());
                return !reached_end();
            }

            void page::on_frame_added()
            {
                frame_count_++;
                header_->frame_count = frame_count_;
                header_->last_frame_position = frame_.address() - address();
            }

            void page::release()
            {
                if (!os::release_mmap_buffer(address(), size_, lazy_))
                {
                    throw exception("failed to release memory for page " + get_page_path(location_, id_));
                }
            }

            page_ptr page::load(const data::location &location, int page_id, bool is_writing, bool lazy)
            {
                std::string path = get_page_path(location, page_id);
                uintptr_t address = os::load_mmap_buffer(path, JOURNAL_PAGE_SIZE, is_writing, lazy);
                if (address < 0)
                {
                    throw exception("unable to load page for " + path);
                }

                page_header *header = reinterpret_cast<page_header *>(address);

                if (header->last_frame_position == 0)
                {
                    header->version = __JOURNAL_VERSION__;
                    header->header_length = sizeof(page_header);
                    header->frame_count = 0;
                    header->last_frame_position = header->header_length;
                }

                if (header->version != __JOURNAL_VERSION__ or header->header_length != sizeof(page_header))
                {
                    std::stringstream ss;
                    ss << "version mismatch for page " << path << ", required " << __JOURNAL_VERSION__ << ", found" << header->version;
                    throw exception(ss.str().c_str());
                }

                return std::shared_ptr<page>(new page(location, page_id, JOURNAL_PAGE_SIZE, lazy, header));
            }

            std::string page::get_page_path(const data::location &location, int id)
            {
                std::string page_filename = JOURNAL_PREFIX + "." + location.name + "." + std::to_string(id) + "." + JOURNAL_SUFFIX;
                return os::make_path({KF_DIR_JOURNAL, data::get_mode_name(location.mode), data::get_category_name(location.category), location.group, page_filename}, true);
            }

            std::vector<int> page::list_page_ids(const data::location &location)
            {
                std::string page_filename_regex = JOURNAL_PREFIX + "\\." + location.name + "\\.[0-9]+\\." + JOURNAL_SUFFIX;
                std::regex pattern(page_filename_regex);
                std::string dir = os::make_path({KF_DIR_JOURNAL, data::get_mode_name(location.mode), data::get_category_name(location.category), location.group}, false);
                boost::filesystem::path journal_folder_path(dir);
                std::vector<int> res;
                for (auto &file : boost::filesystem::directory_iterator(journal_folder_path))
                {
                    std::string filename = file.path().filename().string();
                    if (std::regex_match(filename.begin(), filename.end(), pattern))
                    {
                        int begin = JOURNAL_PREFIX.length() + location.name.length() + 2;
                        int end = filename.length() - JOURNAL_SUFFIX.length();
                        std::string page_id_str = filename.substr(begin, end);
                        res.push_back(atoi(page_id_str.c_str()));
                    }
                }
                std::sort(res.begin(), res.end());
                return res;
            }

            int page::find_page_id(const data::location &location, int64_t time)
            {
                std::vector<int> page_ids = list_page_ids(location);
                if (page_ids.empty())
                {
                    return 1;
                }
                if (time == 0)
                {
                    return page_ids.front();
                }
                for (int idx = page_ids.size() - 1; idx >= 0; idx--)
                {
                    page_ptr page = page::load(location, page_ids[idx], false, true);
                    if (page->begin_time() < time)
                    {
                        return page_ids[idx];
                    }
                }
                return page_ids.front();
            }
        }
    }
}
