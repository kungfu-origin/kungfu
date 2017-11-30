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
 * Structs for page engine socket communication.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   September, 2017
 */

#ifndef YIJINJING_PAGESOCKETSTRUCT_H
#define YIJINJING_PAGESOCKETSTRUCT_H

#include "constants.h"

#define PAGED_SOCKET_FILE KUNGFU_SOCKET_FOLDER "paged.sock"

YJJ_NAMESPACE_START

#define SOCKET_MESSAGE_MAX_LENGTH       500 /**< max length of a socket buffer */
#define SOCKET_ERROR_MAX_LENGTH         100 /**< max length of error msg for socket */

//////////////////////////////////////////
/// (byte) PagedSocketTypeConstants
//////////////////////////////////////////
// test 0~9
#define PAGED_SOCKET_CONNECTION_TEST    0  /**< test connection, return "hello world!" */
// register / exit 10 ~ 19
#define PAGED_SOCKET_STRATEGY_REGISTER  10 /**< register strategy (has to be writer) */
#define PAGED_SOCKET_JOURNAL_REGISTER   11 /**< register journal */
#define PAGED_SOCKET_READER_REGISTER    12 /**< register client (reader) */
#define PAGED_SOCKET_WRITER_REGISTER    13 /**< register client (writer) */
#define PAGED_SOCKET_CLIENT_EXIT        19 /**< exit a client */
// subscribe 20 ~ 29
#define PAGED_SOCKET_SUBSCRIBE          20 /**< subscribe market data */
#define PAGED_SOCKET_SUBSCRIBE_TBC      21 /**< subscribe market data (to be continued)
                                             * affected by SOCKET_MESSAGE_MAX_LENGTH */
// trade engine req 30 ~ 39
#define PAGED_SOCKET_TD_LOGIN           22 /**< login trade engine */
// timer req
#define TIMER_SEC_DIFF_REQUEST          99 /**< timer update request */

/**
 * Socket Request msg, fully utilized by "register"
 */
struct PagedSocketRequest
{
    /** PagedSocketTypeConstants */
    byte    type;
    /** name utilized for CLIENT / JOURNAL / STRATEGY */
    char    name[JOURNAL_SHORT_NAME_MAX_LENGTH];
    /** process id (only utilized when registering client) */
    int     pid;
    /** process id (only take effect when exiting client) */
    int     hash_code;
    /** source id (only take effect when login trade engine) */
    short   source;
} __attribute__((packed));

struct PagedSocketResponse
{
    /** PagedSocketTypeConstants */
    byte    type;
    /** return true if success */
    bool    success;
    /** error message if failure '\0' if success. */
    char    error_msg[SOCKET_ERROR_MAX_LENGTH];
} __attribute__((packed));

struct PagedSocketRspClient: public PagedSocketResponse
{
    /** comm_file is provided for further page usage */
    char    comm_file[JOURNAL_FOLDER_MAX_LENGTH];
    /** size of comm_file */
    int     file_size;
    /** hash code of this client */
    int     hash_code;
};

struct PagedSocketRspJournal: public PagedSocketResponse
{
    /** the index in the comm_file */
    int     comm_idx;
};

struct PagedSocketRspStrategy: public PagedSocketResponse
{
    /** start of request id */
    int     rid_start;
    /** end of request id */
    int     rid_end;
};

YJJ_NAMESPACE_END

#endif //YIJINJING_PAGESOCKETSTRUCT_H
