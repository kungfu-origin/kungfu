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
 * Journal.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * An abstraction of one continuous and infinite memory.
 * may write / read through page.
 */

#ifndef YIJINJING_JOURNAL_H
#define YIJINJING_JOURNAL_H

#include "YJJ_DECLARE.h"
#include "Page.h"

YJJ_NAMESPACE_START

FORWARD_DECLARE_PTR(Journal);
FORWARD_DECLARE_PTR(IPageProvider);

/**
 * Journal class, the abstraction of continuous memory access
 */
class Journal
{
private:
    /** page provider for further page acquire / release */
    IPageProviderPtr pageProvider;
    /** service index is allocated by provider, and will be utilized when asking page provider */
    int     serviceIdx;
    /** basic information, directory path */
    string  directory;
    /** basic information, journal short name */
    string  shortName;
    /** !!for performance only, cache pageProvider->isWriter() */
    bool    isWriting;
    /** flag to determine whether read its frame */
    bool    expired;
    /** current page in use */
    PagePtr curPage;
    /** private constructor, make create the only builder */
    Journal(): expired(false){};

public:
    /** the only entrance of creating a Journal */
    static JournalPtr create(const string& dir, const string& jname, int serviceIdx, IPageProviderPtr provider);

    /** expire this journal, won't get any frame until reset by seekTime */
    void    expire();
    /** seek to time in nanoseconds-timestamp */
    void    seekTime(long time);
    /** get frame address return nullptr if no available */
    void*   locateFrame();
    /** move forward to next frame */
    void    passFrame();
    /** load next page, current page will be released if not empty */
    void    loadNextPage();
    /** get current page number */
    short   getCurPageNum() const;
    /** get journal short name */
    string  getShortName() const;
};

inline void* Journal::locateFrame()
{
    if (expired)
        return nullptr;
    // if writing, we need an empty frame
    // if reading, we need an applicable frame
    if (isWriting)
    {
        void* frame = curPage->locateWritableFrame();
        while (frame == nullptr)
        {
            loadNextPage();
            frame = curPage->locateWritableFrame();
        }
        return frame;
    }
    else
    {
        if ((curPage.get() != nullptr && curPage->isAtPageEnd()) || curPage.get() == nullptr)
            loadNextPage();
        if (curPage.get() != nullptr)
            return curPage->locateReadableFrame();
    }
    return nullptr;
};

inline void Journal::passFrame()
{   // only called after frame is taken, so current frame is applicable for sure; just skip
    curPage->passFrame();
}

inline short Journal::getCurPageNum() const
{
    return curPage->getPageNum();
}

inline string Journal::getShortName() const
{
    return shortName;
}

YJJ_NAMESPACE_END
#endif //YIJINJING_JOURNAL_H
