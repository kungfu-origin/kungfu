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

//
// Created by cjiang on 17/5/24.
//

#ifndef LONGFIST_SYS_MESSAGES_H_H
#define LONGFIST_SYS_MESSAGES_H_H

/* system message types  0 ~ 99 */
// 0 - 9 for test
const short MSG_TYPE_PYTHON_OBJ = 0;
// 10 - 19 strategy
const short MSG_TYPE_STRATEGY_START = 10;
const short MSG_TYPE_STRATEGY_END = 11;
const short MSG_TYPE_TRADE_ENGINE_LOGIN = 12; // we don't need logout, just use strategy_end.
const short MSG_TYPE_TRADE_ENGINE_ACK = 13; // when trade engine got login request, send ack back.
const short MSG_TYPE_STRATEGY_POS_SET = 14; // strategy setup pos json.
// 20 - 29 service
const short MSG_TYPE_PAGED_START = 20;
const short MSG_TYPE_PAGED_END = 21;
// 30 - 39 control
const short MSG_TYPE_TRADE_ENGINE_OPEN = 30;
const short MSG_TYPE_TRADE_ENGINE_CLOSE = 31;
const short MSG_TYPE_MD_ENGINE_OPEN = 32;
const short MSG_TYPE_MD_ENGINE_CLOSE = 33;
const short MSG_TYPE_SWITCH_TRADING_DAY = 34;
const short MSG_TYPE_STRING_COMMAND = 35;
// 50 - 89 utilities
const short MSG_TYPE_TIME_TICK = 50;
const short MSG_TYPE_SUBSCRIBE_MARKET_DATA = 51;
const short MSG_TYPE_SUBSCRIBE_L2_MD = 52;
const short MSG_TYPE_SUBSCRIBE_INDEX = 53;
const short MSG_TYPE_SUBSCRIBE_ORDER_TRADE = 54;
const short MSG_TYPE_ENGINE_STATUS = 60;
// 90 - 99 memory alert
const short MSG_TYPE_MEMORY_FROZEN = 90; // UNLESS SOME MEMORY UNLOCK, NO MORE LOCKING

#endif //LONGFIST_SYS_MESSAGES_H_H
