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

#include "PageCommStruct.h"
#include "PageSocketStruct.h"
#include "JournalWriter.h"

#include <utility>

YJJ_NAMESPACE_START

#define TEMP_PAGE KUNGFU_JOURNAL_FOLDER + "TEMP_PAGE"
#define MEMORY_MSG_FILE KUNGFU_JOURNAL_FOLDER + "PAGE_SERVICE_MSG"

/** we call each journal handler (writer or reader)
 *      -- a client for page engine.
 *  we call each "journal" linked by client
 *      -- a user for page engine.
 *  then each writer client may only have 1 user,
 *  while each reader client may have several users.
 *  all the necessary information are stored here.
 */
struct PageClientInfo
{
    /** the index of each user linked by this client */
    vector<int> user_index_vec;
    /** register nano time */
    int64_t  reg_nano;
    /** process id */
    int   pid;
    /** hash code for the client */
    int   hash_code;
    /** true if this client is a writer */
    bool  is_writing;
    /** true if this writer is associated with a strategy */
    bool  is_strategy;
    /** start rid of the strategy (strategy only) */
    int   rid_start;
    /** end rid of the strategy (strategy only) */
    int   rid_end;
};

class PageService
{
private:
    // internal data structures. be careful on its thread-safety
    /** map: client -> all info (all journal usage) */
    map<string, PageClientInfo> clientJournals;
    /** map: pid -> client */
    map<int, vector<string> > pidClient;
    /** map: file attached with number of writers */
    map<PageServiceMsg, int> fileWriterCounts;
    /** map: file attached with number of readers */
    map<PageServiceMsg, int> fileReaderCounts;
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

    /** start paged service, mainly start tasks */
    void start();
    /** sync stop paged service */
    void stop();

    /** write string content to system journal */
    bool write(string content, byte msg_type, bool is_last=true, short source=0);

    void process_memory_message();
    std::string    register_journal(const string& clientName);
    std::string    register_client(const string& clientName, int pid, bool isWriter);
    std::string    exit_client(const string& clientName, int hashCode, bool needHashCheck);

private:
    /** release the page assigned in comm msg */
    void release_page(const PageServiceMsg& msg);
    /** initialize the page assigned in comm msg */
    byte initiate_page(const PageServiceMsg& msg);
};

YJJ_NAMESPACE_END

#endif //YIJINJING_PAGEENGINE_H
