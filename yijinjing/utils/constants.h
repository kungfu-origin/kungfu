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
 * Some basic constants here.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 */

#ifndef YIJINJING_CONSTANTS_H
#define YIJINJING_CONSTANTS_H

#include "YJJ_DECLARE.h"

YJJ_NAMESPACE_START

/** seed of hash function */
const uint HASH_SEED = 97;

#define JOURNAL_SHORT_NAME_MAX_LENGTH   30
#define JOURNAL_FOLDER_MAX_LENGTH       100

/** seed of hash function */
const long TIME_FROM_FIRST = 0;
const long TIME_TO_LAST = -1;

/** size related */
const int KB = 1024;
const int MB = KB * KB;
const int JOURNAL_PAGE_SIZE = 128 * MB;
const int PAGE_MIN_HEADROOM = 2 * MB;

YJJ_NAMESPACE_END

#endif //YIJINJING_CONSTANTS_H
