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

#include <utility>

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/service/page_service.h>

YJJ_NAMESPACE_START

#define TEMP_PAGE KUNGFU_JOURNAL_FOLDER + "TEMP_PAGE"
#define MEMORY_MSG_FILE KUNGFU_JOURNAL_FOLDER + "PAGE_SERVICE_MSG"

/*
 * PROCESS:
 *
 *      * default:                          -- PAGE_COMM_RAW
 *
 *      user:   socket request
 *      server: allocate idx                -> PAGE_COMM_ALLOCATED
 *      user:   setup folder / shortName    -> PAGE_HOLDING
 *
 *      user:   fill in page_num            -> PAGE_REQUESTING
 *      server: find requesting, allocate
 *              finish allocating           -> PAGE_ALLOCATED (success)
 *                                          -> PAGE_NON_EXIST (reader wants a not existing page)
 *                                          -> PAGE_MEM_OVERFLOW (current locking memory overflows)
 *
 */
struct PageServiceMessage
{
    /** PagedCommTypeConstants (by both server and client) */
    volatile int8_t    status;
    /** journal folder (by client) */
    char    folder[JOURNAL_FOLDER_MAX_LENGTH];
    /** journal name (by client) */
    char    name[JOURNAL_SHORT_NAME_MAX_LENGTH];
    /** return true if the client is writer (by client) */
    bool    is_writer;
    /** page number to request (by client) */
    int16_t   page_num;
    /** page number requested (by server) */
    int16_t   last_page_num;

    // operators for map key
    bool const operator == (const PageServiceMessage &p) const
    {
        return page_num == p.page_num && strcmp(folder, p.folder) == 0 && strcmp(name, p.name) == 0;
    }

    bool const operator < (const PageServiceMessage &p) const
    {
        return (strcmp(folder, p.folder) != 0) ? strcmp(folder, p.folder) < 0
                                               : (strcmp(name, p.name) != 0) ? strcmp(name, p.name) < 0
                                                                             : page_num < p.page_num;
    }
#ifndef _WIN32
} __attribute__((packed));
#else
};
#pragma pack(pop)
#endif

/** max number of communication users in the same time */
#define MAX_MEMORY_MSG_NUMBER 1000
/** REQUEST_ID_RANGE * MAX_MEMORY_MSG_NUMBER < 2147483647(max num of int) */
#define REQUEST_ID_RANGE 1000000
/** based on the max number, the memory message file size is determined */
const int MEMORY_MSG_FILE_SIZE = MAX_MEMORY_MSG_NUMBER * sizeof(PageServiceMessage) + 1024;
/** fast type convert */
#define GET_MEMORY_MSG(buffer, idx) ((PageServiceMessage*)(ADDRESS_ADD(buffer, idx * sizeof(PageServiceMessage))))

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
