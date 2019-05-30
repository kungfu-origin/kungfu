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

#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif // _WINDOWS

#include <cstdio>
#include <sstream>
#include <regex>

#include <boost/filesystem.hpp>

#include <kungfu/yijinjing/log/setup.h>
#include <kungfu/yijinjing/time.h>
#include <kungfu/yijinjing/journal/page.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace journal
        {
            const std::string page_fileinfo::get_path(int id) const
            {
                boost::filesystem::path page_path(dir);
                page_path /= JOURNAL_PREFIX + "." + name + "." + std::to_string(id) + "." + JOURNAL_SUFFIX;
                return page_path.string();
            }

            page::page(const page_fileinfo &fileinfo, const int id, const size_t &size, const bool &lazy, page_header *header) :
                    fileinfo_(fileinfo), id_(id), size_(size), lazy_(lazy), header_(header),
                    frame_count_(header->frame_count),
                    frame_(address() + sizeof(page_header))
            {}

            page_ptr page::load(const page_fileinfo &fileinfo, int page_id, bool is_writing, bool lazy)
            {
                std::string path = fileinfo.get_path(page_id);
                uintptr_t address = util::load_mmap_buffer(fileinfo.get_path(page_id), JOURNAL_PAGE_SIZE, is_writing, lazy);
                if (address < 0)
                {
                    throw exception("unable to load page for " + path);
                }

                page_header *header = reinterpret_cast<page_header *>(address);

                if (header->last_frame_position == 0)
                {
                    header->version = __FRAME_HEADER_VERSION__;
                    header->header_length = sizeof(page_header);
                    header->frame_count = 0;
                    header->last_frame_position = header->header_length;
                }

                if (header->version != __FRAME_HEADER_VERSION__ or header->header_length != sizeof(page_header))
                {
                    std::stringstream ss;
                    ss << "version mismatch for page " << path << ", required " << __FRAME_HEADER_VERSION__ << ", found" << header->version;
                    throw exception(ss.str().c_str());
                }

                return std::make_shared<page>(fileinfo, page_id, JOURNAL_PAGE_SIZE, lazy, header);
            }

            void page::seek_next()
            {
                frame_.move_to_next();
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
                SPDLOG_DEBUG("seek to time {}", time::strftime(nanotime));
                SPDLOG_DEBUG("page end time {}, last frame {}", time::strftime(end_time()), header_->last_frame_position);
                if (end_time() < nanotime)
                {
                    return false;
                }
                while (frame_.has_data() && frame_.gen_time() < nanotime)
                {
                    SPDLOG_DEBUG("frame {}", time::strftime(frame_.gen_time()));
                    frame_.move_to_next();
                }
                return reached_end();
            }

            void page::update_header()
            {
                header_->frame_count = frame_count_;
                header_->last_frame_position = frame_.address() - address();
            }

            void page::grow()
            {
                frame_count_++;
                header_->frame_count = frame_count_;
                header_->last_frame_position = frame_.address() - address();
            }

            void page::release()
            {
                void *buffer = reinterpret_cast<void *>(address());
#ifdef _WINDOWS
                UnmapViewOfFile(buffer);
#else
                //unlock and unmap
                if (!lazy_ && munlock(buffer, size_) != 0)
                {
                    throw exception("failed to unlock memory for page " + get_path());
                }

                if (munmap(buffer, size_) != 0)
                {
                    throw exception("failed to unmap memory for page " + get_path());
                }
#endif // _WINDOWS
            }

            std::vector<int> util::list_page_ids(const page_fileinfo &fileinfo)
            {
                std::string page_filename_regex = JOURNAL_PREFIX + "\\." + fileinfo.name + "\\.[0-9]+\\." + JOURNAL_SUFFIX;
                std::regex pattern(page_filename_regex);
                boost::filesystem::path journal_folder_path(fileinfo.dir);
                std::vector<int> res;
                for (auto &file : boost::filesystem::directory_iterator(journal_folder_path))
                {
                    std::string filename = file.path().filename().string();
                    if (std::regex_match(filename.begin(), filename.end(), pattern))
                    {
                        int begin = JOURNAL_PREFIX.length() + fileinfo.name.length() + 2;
                        int end = filename.length() - JOURNAL_SUFFIX.length();
                        std::string page_id_str = filename.substr(begin, end);
                        res.push_back(atoi(page_id_str.c_str()));
                    }
                }
                std::sort(res.begin(), res.end());
                return res;
            }

            int util::find_page_id(const page_fileinfo &fileinfo, int64_t time)
            {
                std::vector<int> page_ids = list_page_ids(fileinfo);
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
                    page_ptr page = page::load(fileinfo, page_ids[idx], false, true);
                    if (page->begin_time() < time)
                    {
                        return page_ids[idx];
                    }
                }
                return page_ids.front();
            }

            uintptr_t util::load_mmap_buffer(const std::string &path, size_t size, bool is_writing, bool lazy)
            {
                boost::filesystem::path page_path(path);
                boost::filesystem::path page_folder_path = page_path.parent_path();
                if (!boost::filesystem::exists(page_folder_path))
                {
                    boost::filesystem::create_directories(page_folder_path);
                }
#ifdef _WINDOWS
                HANDLE dumpFileDescriptor = CreateFileA(path.c_str(),
                    (is_writing) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    (is_writing) ? OPEN_ALWAYS : OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

                if (dumpFileDescriptor == INVALID_HANDLE_VALUE)
                {
                    if (!is_writing) return nullptr;

                    printf("Cannot create/write the file");
                    exit(EXIT_FAILURE);
                }


                HANDLE fileMappingObject = CreateFileMapping(dumpFileDescriptor,
                    NULL,
                    (is_writing) ? PAGE_READWRITE:PAGE_READONLY,
                    0,
                    size,
                    NULL);

                if (fileMappingObject == NULL)
                {
                    int nRet = GetLastError();
                    printf("LoadPageBuffer fail(%s): CreateFileMapping Error = %d, %s\n", is_writing ? "writer" : "reader", nRet, path.c_str());
                    return nullptr;
                    //exit(EXIT_FAILURE);
                }


                void* buffer = MapViewOfFile(fileMappingObject,
                    (is_writing) ? FILE_MAP_ALL_ACCESS:FILE_MAP_READ,
                    0,
                    0,
                    size);

                if (buffer == nullptr)
                {
                    int nRet = GetLastError();
                    throw exception("failed to load page " + path + ", MapViewOfFile Error " + std::to_string(nRet));
                }

                SPDLOG_DEBUG("loaded buffer for page {}", path);
                return reinterpret_cast<uintptr_t>(buffer);
#else
                int fd = open(path.c_str(), (is_writing ? O_RDWR : O_RDONLY) | O_CREAT, (mode_t) 0600);
                if (fd < 0)
                {
                    throw exception("failed to open file for page " + path);
                }

                if (/*!lazy &&*/ is_writing)
                {
                    if (lseek(fd, size - 1, SEEK_SET) == -1)
                    {
                        close(fd);
                        throw exception("failed to stretch for page " + path);
                    }
                    if (write(fd, "", 1) == -1)
                    {
                        close(fd);
                        throw exception("unable to write for page " + path);
                    }
                }

                void *buffer = mmap(0, size, (is_writing) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);

                if (buffer == MAP_FAILED)
                {
                    close(fd);
                    throw exception("Error mapping file to buffer");
                }

                if (!lazy && madvise(buffer, size, MADV_RANDOM) != 0 && mlock(buffer, size) != 0)
                {
                    munmap(buffer, size);
                    close(fd);
                    throw exception("failed to lock memory for page " + path);
                }

                close(fd);
                SPDLOG_DEBUG("loaded buffer for page {}, writing {}, lazy {}", path, is_writing, lazy);
                return reinterpret_cast<uintptr_t>(buffer);
#endif // _WINDOWS
            }
        }
    }
}
