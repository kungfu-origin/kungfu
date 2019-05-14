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
 * Structs for page engine page communication.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 */

#ifndef YIJINJING_PAGECOMMSTRUCT_H
#define YIJINJING_PAGECOMMSTRUCT_H

#include "constants.h"
#include "sys_messages.h"
#include <string.h>

YJJ_NAMESPACE_START

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
//////////////////////////////////////////
/// (byte) PagedServiceMessage types
//////////////////////////////////////////
// status in process 0 ~ 9
#define PAGE_RAW          0   /**< this msg block is not allocated (default) */
#define PAGE_OCCUPIED     1   /**< comm msg idx occupied (by server) */
#define PAGE_HOLDING      2   /**< folder / name ready (by client) */
#define PAGE_REQUESTING   3   /**< page number specified (by client) */
#define PAGE_ALLOCATED    4   /**< finish allocated, user may getPage (by server) */
// failures 10 ~ 19
#define PAGE_NON_EXIST    11  /**< default position */
#define PAGE_MEM_OVERFLOW 12  /**< default position */
#define PAGE_MORE_THAN_ONE_WRITE      13  /**< default position */
#define PAGE_CANNOT_RENAME_FROM_TEMP  14  /**< default position */

struct PageServiceMessage
{
    /** PagedCommTypeConstants (by both server and client) */
    volatile byte    status;
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

YJJ_NAMESPACE_END

#endif //YIJINJING_PAGECOMMSTRUCT_H
