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

#ifndef YIJINJING_JOURNALFINDER_H
#define YIJINJING_JOURNALFINDER_H

#include "YJJ_DECLARE.h"

#include "StrategyUtil.h"
#include "longfist/LFUtils.h"

#include <string>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

YJJ_NAMESPACE_START

#define JOURNAL_NAME_PATTERN JOURNAL_PREFIX + "\\.(\\w+)\\.[0-9]+\\." + JOURNAL_SUFFIX

class JournalFinder {
private:
    void addJournalInfo(std::string name, std::string folder);
    void loadJournalInfo(short source, JournalPair (*getJournalPair)(short));
    void loadJournalInfo(short source);
protected:
    vector<std::string> all_journal_names;
    vector<std::string> avaliable_journal_names;
    vector<std::string> avaliable_journal_folders;
    map<std::string, std::string> all_journal;
public:
    JournalFinder();
    vector<std::string> const &getAllJournalNames() const { return all_journal_names; }
    vector<std::string> const &getAvailableJournalNames() const { return avaliable_journal_names; }
    vector<std::string> const &getAvailableJournalFolders() const { return avaliable_journal_folders; }
    std::string const &getJournalFolder(const std::string &name) { return all_journal[name]; }
};

YJJ_NAMESPACE_END
#endif //YIJINJING_JOURNALFINDER_H
