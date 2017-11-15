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
 * JournalReader
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * provide read access to journals.
 * one journal reader may access multiple journals
 * and combine all these streams into one stream
 * with nano time order.
 */

#ifndef YIJINJING_JOURNALREADER_H
#define YIJINJING_JOURNALREADER_H

#include "JournalHandler.h"
#include "IJournalVisitor.h"
#include "Frame.hpp" // for inline function
#include "Journal.h" // for inline function

YJJ_NAMESPACE_START

FORWARD_DECLARE_PTR(JournalReader);
/**
 * Journal Reader
 */
class JournalReader : public JournalHandler
{
private:
    /** current journal in use */
    JournalPtr curJournal;
    /** visitor list */
    vector<IJournalVisitor*> visitors;
    /** map from journal short name to its idx */
    map<string, size_t> journalMap;
    /** private constructor */
    JournalReader(PageProviderPtr ptr);

public:
    /** [usage]: next frame, and process the frame */
    FramePtr getNextFrame();
    /** to keep the last time's getNextFrame's source. */
    string   getFrameName() const;
    /** [usage]: keep looping and visiting */
    void  startVisiting();

    /** override JournalHandler's addJournal,
     * allow re-add journal with same name */
    virtual size_t addJournal(const string& dir, const string& jname);
    /** add visitor for "startVisiting" usage  */
    bool  addVisitor(IJournalVisitor* visitor);
    /** all journals jump to start time */
    void jumpStart(long startTime);
    /** expire one of the journal by index,
     * return true if expire the journal successfully */
    bool  expireJournal(size_t idx);
    /** expire one of the journal by journal short name,
     * return true if expire the journal successfully */
    bool  expireJournalByName(const string& jname);
    /** seek nano-time of a journal by index,
     * return true if that journal exists and seeked successfully */
    bool  seekTimeJournal(size_t idx, long nano);
    /** seek nano-time of a journal by journal short name,
     * return true if that journal exists and seeked successfully */
    bool  seekTimeJournalByName(const string& jname, long nano);

public:
    // creators
    static JournalReaderPtr create(const vector<string>& dirs,
                                   const vector<string>& jnames,
                                   long startTime,
                                   const string& readerName);

    static JournalReaderPtr create(const vector<string>& dirs,
                                   const vector<string>& jnames,
                                   const vector<IJournalVisitor*>& visitors,
                                   long startTime,
                                   const string& readerName);

    static JournalReaderPtr create(const string& dir,
                                   const string& jname,
                                   long startTime,
                                   const string& readerName);

    static JournalReaderPtr create(const vector<string>& dirs,
                                   const vector<string>& jnames,
                                   long startTime);

    static JournalReaderPtr create(const string& dir,
                                   const string& jname,
                                   long startTime);

    static JournalReaderPtr createReaderWithSys(const vector<string>& dirs,
                                                const vector<string>& jnames,
                                                long startTime,
                                                const string& readerName);

    static JournalReaderPtr createSysReader(const string& readerName);
    /** revisable reader is a reader with authority to revise data it reads */
    static JournalReaderPtr createRevisableReader(const string& readerName);

    static const string PREFIX;

};

inline FramePtr JournalReader::getNextFrame()
{
    long  minNano = TIME_TO_LAST;
    void* res_address = nullptr;
    for (JournalPtr& journal: journals)
    {
        FrameHeader* header = (FrameHeader*)(journal->locateFrame());
        if (header != nullptr)
        {
            long nano = header->nano;
            if (minNano == TIME_TO_LAST || nano < minNano)
            {
                minNano = nano;
                res_address = header;
                curJournal = journal;
            }
        }
    }
    if (res_address != nullptr)
    {
        curJournal->passFrame();
        return FramePtr(new Frame(res_address));
    }
    else
    {
        return FramePtr();
    }
}

YJJ_NAMESPACE_END
#endif //YIJINJING_JOURNALREADER_H
