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
 * Define basic macros.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * This file is supposed to be included by all files in this project
 */

#ifndef YIJINJING_DECLARE_H
#define YIJINJING_DECLARE_H

/** python is necessary in most situation,
 * to remove the annoying warning on _POSIX_C_SOURCE,
 * simply put it in the top position. */
#include <boost/python.hpp>
/** we use boost shared_ptr as our smart pointer */
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

using std::string; /**< default using string */
using std::vector; /**< default using vector */
using std::map;    /**< default using map */

template<class Key, class T>
using umap = std::unordered_map<Key, T>; /** unordered_map usage, will merge more */

typedef unsigned char byte;
typedef unsigned int uint;
typedef std::pair<int, int> IntPair;

#define KUNGFU_FOLDER "/shared/kungfu/"                 /**< base folder of kungfu system */
#define KUNGFU_JOURNAL_FOLDER KUNGFU_FOLDER "journal/"  /**< where we put journal files */
#define KUNGFU_SOCKET_FOLDER KUNGFU_FOLDER "socket/"    /**< where we put socket files */
#define KUNGFU_LOG_FOLDER KUNGFU_FOLDER "log/"          /**< where we put log files */
#define PAGED_JOURNAL_FOLDER KUNGFU_JOURNAL_FOLDER "system/" /**< paged system journal folder */
#define PAGED_JOURNAL_NAME "SYSTEM"                     /**< paged system journal name */

#define DECLARE_PTR(X) typedef boost::shared_ptr<X> X##Ptr; /**< define smart ptr */
#define FORWARD_DECLARE_PTR(X) class X; DECLARE_PTR(X)      /**< forward defile smart ptr */
#define YJJ_NAMESPACE_START namespace kungfu { namespace yijinjing { /**< start of yjj namespace */
#define YJJ_NAMESPACE_END }};                               /**< end of yjj namespace */
#define USING_YJJ_NAMESPACE using namespace kungfu::yijinjing;

#define JOURNAL_PREFIX string("yjj")        /**< journal file prefix */
#define JOURNAL_SUFFIX string("journal")    /**< journal file suffix */

/** fast type convert for moving address forward */
#define ADDRESS_ADD(x, delta) (void*)((uintptr_t)x + delta)

#endif //YIJINJING_DECLARE_H
