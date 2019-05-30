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

#ifndef YIJINJING_PAGE_H
#define YIJINJING_PAGE_H

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/nanomsg/master.h>

namespace kungfu {
    namespace yijinjing {
        namespace journal {

            struct page_header {
                /** version of frame header */
                int16_t version;
                int16_t header_length;
                /** how many frame in this page (only filled when closed) */
                int32_t frame_count;
                /** pos of last frame */
                int32_t last_frame_position;
#ifndef _WIN32
            } __attribute__((packed));
#else
            };
#pragma pack(pop)
#endif

            class page_fileinfo {
            public:
                const locator_ptr locator;
                const data::mode mode;
                const data::category category;
                const std::string group;
                const std::string name;
                const std::string dir;

                const std::string get_path(int id) const ;

                inline const std::string to_string() const {
                    return dir + "/" + name;
                }
            private:
                page_fileinfo(locator_ptr l, data::mode m, data::category c, const std::string &group, const std::string& name):
                        locator(l), mode(m), category(c), group(group), name(name), dir(locator->get_journal_dir(m, c, group)) {};
                friend class page_provider;
            };

            class page {
            public:

                page(const page_fileinfo &fileinfo, int id, const size_t &size, const bool &lazy, page_header * header_);

                static page_ptr load(const page_fileinfo &fileinfo, int page_id, bool is_writing=false, bool lazy=true);

                inline const int get_id() const { return id_; };
                inline const std::string get_path() const { return fileinfo_.get_path(id_); };
                inline const page_fileinfo &get_fileinfo() const { return fileinfo_; };

                inline frame &current_frame() { return frame_;};

                inline uintptr_t address() const { return reinterpret_cast<uintptr_t>(header_); }
                inline uintptr_t address_border() const { return address() + JOURNAL_PAGE_SIZE - PAGE_MIN_HEADROOM; };

                inline int64_t begin_time() const { return first_frame_header()->gen_time; };
                inline int64_t end_time() const { return last_frame_header()->gen_time; };

                inline bool reached_end() const {
                    return frame_.address() > address_border();
                }

                /**
                 * seek next frame, user is responsible to check reached_end
                 */
                void seek_next();

                /**
                 * seek to the first writable frame
                 * @return true if found a writable frame, false if the page is full
                 */
                bool seek_to_writable();

                /**
                 * seek to the next frame that passed given time
                 * @param nanotime time in nanoseconds
                 * @return true if found a frame, false if no frame found
                 */
                bool seek_to_time(int64_t nanotime);

                void grow();

                void update_header();

                /** direct memory manipulation without service
                 * release page buffer, buffer and size needs to be specified.
                 *  if lazy==True, no unlocking; if lazy==False, munlock the memory */
                void release();

            private:
                const page_fileinfo &fileinfo_;
                const int id_;
                const bool lazy_;
                const size_t size_;
                page_header * header_;
                frame frame_;
                int frame_count_;

                inline frame_header *first_frame_header() const { return reinterpret_cast<frame_header *>(address() + header_->header_length); };
                inline frame_header *last_frame_header() const { return reinterpret_cast<frame_header *>(address() + header_->last_frame_position); };
            };

            /** abstract interface class */
            class page_provider {
            protected:
                const page_fileinfo fileinfo_;
                const bool is_writing_;
            public:
                page_provider(locator_ptr l, data::mode m, data::category c, const std::string &group, const std::string& name, bool is_writing):
                    fileinfo_(l, m, c, group, name), is_writing_(is_writing) {};
                /** destructor */
                virtual ~page_provider() {};

                const page_fileinfo &get_fileinfo() const {return fileinfo_;};
                const bool &is_writing() const {return is_writing_;};

                virtual page_ptr get_page(int new_page_id, int old_page_id) = 0;
                virtual const std::unordered_map<int, page_ptr> &get_all_pages() = 0;
                virtual void release_page(int page_id) = 0;
                virtual void release_all() = 0;
            };
            DECLARE_PTR(page_provider)

            class page_provider_factory
            {
            public:
                virtual page_provider_ptr make_page_provider(data::mode m, data::category c, const std::string &group, const std::string& name, const bool is_writing) const = 0;
            };
            DECLARE_PTR(page_provider_factory)

            namespace util {

                /**
                 * load mmap buffer, return address of the file-mapped memory
                 * whether to write has to be specified in "is_writing"
                 * buffer memory is locked if not lazy
                 * @return the address of mapped memory
                 */
                uintptr_t load_mmap_buffer(const std::string &path, size_t size, bool is_writing = false, bool lazy = true);

                std::vector<int> list_page_ids(const page_fileinfo &fileinfo);
                int find_page_id(const page_fileinfo &fileinfo, int64_t time);
            }

            /**
             * local_page_provider,
             * provide local page, no need to connect with service.
             */
            class local_page_provider : public page_provider {
            public:
                local_page_provider(locator_ptr l,
                        data::mode m, data::category c, const std::string &group, const std::string& name, const bool is_writing, const bool lazy = false):
                    page_provider(l, m, c, group, name, is_writing), lazy_(lazy) {};

                virtual page_ptr get_page(int new_page_id, int old_page_id);
                virtual const std::unordered_map<int, page_ptr> &get_all_pages();

                virtual void release_page(int page_id);
                virtual void release_all();

            private:
                const bool lazy_;
                std::unordered_map<int, page_ptr> pages_;
            };

            /**
             * client_page_provider,
             * provide page via memory service, socket & comm
             */
            class client_page_provider : public local_page_provider {
            public:
                client_page_provider(locator_ptr l, data::mode m, data::category c, const std::string &group, const std::string &name, const bool is_writing,
                        const std::string &client_name, nanomsg::master_messenger_ptr messenger);

                page_ptr get_page(int new_page_id, int old_page_id) override ;

                void release_page(int page_id) override ;
                void release_all() override ;

            private:
                std::string client_name_;
                nanomsg::master_messenger_ptr messenger_;
                int memory_msg_id_;
                uintptr_t memory_msg_buffer_;
            };
        }
    }
}

#endif //YIJINJING_PAGE_H
