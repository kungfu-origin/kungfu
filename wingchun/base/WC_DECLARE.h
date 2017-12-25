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
 * Define basic marcos
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   April, 2017
 * supposed to be included by all files in wingchun
 */

#ifndef WINGCHUN_DECLARE_H
#define WINGCHUN_DECLARE_H

/** YJJ's declare is also useful */
#include "YJJ_DECLARE.h"
/** almost all wingchun engine needs multi-threading */
#include <thread>

#define KUNGFU_RUNTIME_FOLDER KUNGFU_FOLDER "runtime/"

typedef boost::shared_ptr<std::thread> ThreadPtr;

#define WC_NAMESPACE_START namespace kungfu { namespace wingchun {
#define WC_NAMESPACE_END }};
#define USING_WC_NAMESPACE using namespace kungfu::wingchun;

#define TICKER_MARKET_DELIMITER '@'

#define START_PYTHON_FUNC_CALLING \
        try{

#define END_PYTHON_FUNC_CALLING \
        } catch(boost::python::error_already_set& e) \
        {\
            PyErr_Print();\
            throw std::runtime_error(__func__); \
        }

#endif //WINGCHUN_DECLARE_H
