/*****************************************************************************
 * Copyright [2017] [taurus.ai]
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

#include "PageServiceMessage.h"
#include "JournalWriter.h"

#include <utility>

YJJ_NAMESPACE_START

#define TEMP_PAGE KUNGFU_JOURNAL_FOLDER + "TEMP_PAGE"
#define MEMORY_MSG_FILE KUNGFU_JOURNAL_FOLDER + "PAGE_SERVICE_MSG"

class PageService
{
private:
    /** map: file attached with number of writers */
    map<PageServiceMessage, int> fileWriterCounts;
    /** map: file attached with number of readers */
    map<PageServiceMessage, int> fileReaderCounts;
    /** map: file to its page buffer */
    map<string, void*> fileAddrs;

    const string base_dir;
    JournalWriterPtr writer; /**< writer for system journal */
    void*   memory_message_buffer; /**< message buffer */
    size_t  memory_message_limit;     /**< max index of current assigned comm block */
    string  memory_msg_file;   /**< comm file linked to memory */

public:
    /** default constructor */
    PageService(const string& _base_dir);

    string get_memory_msg_file();
    size_t get_memory_msg_file_size();

    /** start paged service, mainly start tasks */
    void start();
    /** sync stop paged service */
    void stop();

    /** write string content to system journal */
    bool write(string content, byte msg_type, bool is_last=true, short source=0);

    void process_memory_message();
    int32_t register_journal(const string& client_name);
    uint32_t register_client(const string& client_name, int pid, bool is_writer);
    void release_page_at(int idx);

private:
    /** release the page assigned in comm msg */
    void release_page(const PageServiceMessage& msg);
    /** initialize the page assigned in comm msg */
    byte initiate_page(const PageServiceMessage& msg);
};

YJJ_NAMESPACE_END

#endif //YIJINJING_PAGEENGINE_H
