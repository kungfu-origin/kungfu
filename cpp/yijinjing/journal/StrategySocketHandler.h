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
 * Strategy Socket Handler.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * strategy needs some functions to page engine
 * As ClientPageProvider has already provides socket access,
 * StrategySocketHandler inherits ClientPageProvider
 * and wrap-up some other features for the ease of usage
 */

#ifndef PROJECT_STRATEGYSOCKETHANDLER_H
#define PROJECT_STRATEGYSOCKETHANDLER_H

#include "YJJ_DECLARE.h"
#include "PageProvider.h"
#include "IStrategyUtil.h"

YJJ_NAMESPACE_START

/**
 * Strategy socket handler
 */
class StrategySocketHandler: public ClientPageProvider, public IStrategyUtil
{
public:
    /** default constructor with strategy name */
    StrategySocketHandler(const string& strategy_name);
    /** register strategy first, with rid_start, rid_end returned */
    virtual bool register_strategy(int& rid_start, int& rid_end);
    /** connect trade engine with source_id determined */
    virtual bool td_connect(short source);
    /** subscribe market data with tickers / source / msg_type assigned */
    virtual bool md_subscribe(const vector<string>& tickers, short source, short msg_type);
};

DECLARE_PTR(StrategySocketHandler);

YJJ_NAMESPACE_END

#endif //PROJECT_STRATEGYSOCKETHANDLER_H
