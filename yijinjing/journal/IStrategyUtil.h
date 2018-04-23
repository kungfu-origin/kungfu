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
 * IStrategyUtil.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * strategy utilities abstraction
 */

#ifndef PROJECT_ISTRATEGYUTIL_H
#define PROJECT_ISTRATEGYUTIL_H

#include "YJJ_DECLARE.h"

YJJ_NAMESPACE_START

/** interface class */
class IStrategyUtil
{
public:
    /** register strategy in paged service */
    virtual bool register_strategy(int& requestIdStart, int& requestIdEnd) = 0;
    /** request kungfu trade engine login */
    virtual bool td_connect(short source) = 0;
    /** subscribe market data */
    virtual bool md_subscribe(const vector<string>& tickers, short source, short msg_type) = 0;
    /** destructor */
    virtual ~IStrategyUtil() {};
};

YJJ_NAMESPACE_END

#endif //PROJECT_ISTRATEGYUTIL_H
