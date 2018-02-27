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
 * Journal Finder
 * @Author kdong (keren.dong@taurus.ai)
 * @since  Feb, 2018
 * Find journal names and folders from longfist meta data.
 */

#include "JournalFinder.h"

#include "PageUtil.h"

USING_YJJ_NAMESPACE

void JournalFinder::loadJournalInfo(short source, JournalPair (*getJournalPair)(short)) {
    JournalPair pair = getJournalPair(source);
    if (pair.first.length() > 0) {
        all_journal[pair.second] = pair.first;
        all_journal_names.push_back(pair.second);

        vector<short> pageNums = PageUtil::GetPageNums(pair.first, pair.second);
        if (!pageNums.empty()) {
            avaliable_journal_names.push_back(pair.second);
            avaliable_journal_folders.push_back(pair.first);
        }
    }
}

void JournalFinder::loadJournalInfo(short source) {
    loadJournalInfo(source, getMdJournalPair);
    loadJournalInfo(source, getMdRawJournalPair);
    loadJournalInfo(source, getL2MdJournalPair);
    loadJournalInfo(source, getL2MdRawJournalPair);
    loadJournalInfo(source, getL2IndexJournalPair);
    loadJournalInfo(source, getL2IndexRawJournalPair);
    loadJournalInfo(source, getL2OrderJournalPair);
    loadJournalInfo(source, getL2OrderRawJournalPair);
    loadJournalInfo(source, getL2TradeJournalPair);
    loadJournalInfo(source, getL2TradeRawJournalPair);
    loadJournalInfo(source, getTdJournalPair);
    loadJournalInfo(source, getTdRawJournalPair);
    loadJournalInfo(source, getTdSendJournalPair);
    loadJournalInfo(source, getTdQJournalPair);
}

JournalFinder::JournalFinder() {
    loadJournalInfo(SOURCE_CTP);
    loadJournalInfo(SOURCE_XTP);
}
