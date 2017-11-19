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
 * Frame Header.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Provide necessary information for frame
 */

//#define __FRAME_HEADER_VERSION__ 1
// changes: 1. type of error_id from short to int
//             some gateway's error_id is too big for short to handle.
#define __FRAME_HEADER_VERSION__ 2

#ifndef YIJINJING_FRAMEHEADER_H
#define YIJINJING_FRAMEHEADER_H

#include "constants.h"

YJJ_NAMESPACE_START

#define ERROR_MSG_MAX_LENGTH 100

// data types of all fields in frame header
// which need to be fully controlled for data length.
typedef byte    FH_TYPE_STATUS;
typedef short   FH_TYPE_SOURCE;
typedef long    FH_TYPE_NANOTM;
typedef int     FH_TYPE_LENGTH;
typedef uint    FH_TYPE_HASHNM;
typedef short   FH_TYPE_MSG_TP;
typedef byte    FH_TYPE_LASTFG;
typedef int     FH_TYPE_REQ_ID;
typedef int     FH_TYPE_ERR_ID;

//////////////////////////////////////////
/// (byte) JournalFrameStatus
//////////////////////////////////////////
#define JOURNAL_FRAME_STATUS_RAW        0
#define JOURNAL_FRAME_STATUS_WRITTEN    1
#define JOURNAL_FRAME_STATUS_PAGE_END   2

//////////////////////////////////////////
/// (byte) JournalFrameLastFlag
//////////////////////////////////////////
#define JOURNAL_FRAME_NOT_LAST          0 /**< false */
#define JOURNAL_FRAME_IS_LAST           1 /**< true */

struct FrameHeader
{
    /** JournalFrameStatus */
    volatile FH_TYPE_STATUS      status;
    /** source (system id) of this frame */
    FH_TYPE_SOURCE      source;
    /** nano time of the frame data */
    FH_TYPE_NANOTM      nano;
    /** total frame length (including header and errorMsg) */
    FH_TYPE_LENGTH      length;
    /** hash code of data part (not whole frame) */
    FH_TYPE_HASHNM      hash;
    /** msg type of the data in frame */
    FH_TYPE_MSG_TP      msg_type;
    /** JournalFrameLastFlag */
    FH_TYPE_LASTFG      last_flag;
    /** request id of this frame data */
    FH_TYPE_REQ_ID      req_id;
    /** extra nano time for usage.
     * example: reference md_nano for order / exanic net nano */
    FH_TYPE_NANOTM      extra_nano;
    /** error id
     * =0 if not error and no errorMsg,
     * !=0 means some info in errorMsg */
    FH_TYPE_ERR_ID      err_id;
} __attribute__((packed));

/** length of frame header */
const FH_TYPE_LENGTH BASIC_FRAME_HEADER_LENGTH = sizeof(FrameHeader);
/** length of frame header with error msg */
const FH_TYPE_LENGTH ERROR_FRAME_HEADER_LENGTH = BASIC_FRAME_HEADER_LENGTH + sizeof(char) * ERROR_MSG_MAX_LENGTH;

YJJ_NAMESPACE_END

#endif //YIJINJING_FRAMEHEADER_H
