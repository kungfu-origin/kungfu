/*****************************************************************************
 * Copyright [taurus.ai]
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

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/journal/frame.h> // for inline function
#include <kungfu/yijinjing/nanomsg/passive.h>

YJJ_NAMESPACE_START

#define JOURNAL_NAME_PATTERN JOURNAL_PREFIX + "\\.(\\w+)\\.[0-9]+\\." + JOURNAL_SUFFIX
typedef std::pair<std::string, std::string> JournalPair;

FORWARD_DECLARE_PTR(Journal)
FORWARD_DECLARE_PTR(IPageProvider)
FORWARD_DECLARE_PTR(PageProvider)
FORWARD_DECLARE_PTR(JournalReader)
FORWARD_DECLARE_PTR(JournalWriter)

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
    void    seekTime(int64_t time);
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

/**
 * visitor mode for reader,
 * any class with this interface can be added into JournalReader
 */
class IJournalVisitor
{
public:
    virtual void visit(const string& name, Frame& frame) = 0;
};

/*
 * JournalHandler
 */
class JournalHandler
{
protected:
    /** all page-engine-interact stuff is handled by pageProvider */
    PageProviderPtr page_provider;
    /** journals */
    vector<JournalPtr> journals;
    /** current journal */
    JournalPtr  curJournal;
public:
    /** default constructor with outside page provider */
    JournalHandler(PageProviderPtr ptr): page_provider(ptr) {};
    /** default destructor */
    virtual ~JournalHandler();
    /** return the journal's index in the vector */
    virtual size_t addJournal(const string& dir, const string& jname);
    /** default name */
    static string getDefaultName(const string& prefix);
};

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
    void jumpStart(int64_t startTime);
    /** expire one of the journal by index,
     * return true if expire the journal successfully */
    bool  expireJournal(size_t idx);
    /** expire one of the journal by journal short name,
     * return true if expire the journal successfully */
    bool  expireJournalByName(const string& jname);
    /** seek nano-time of a journal by index,
     * return true if that journal exists and seeked successfully */
    bool  seekTimeJournal(size_t idx, int64_t nano);
    /** seek nano-time of a journal by journal short name,
     * return true if that journal exists and seeked successfully */
    bool  seekTimeJournalByName(const string& jname, int64_t nano);

public:
    // creators
    static JournalReaderPtr create(const vector<string>& dirs,
                                   const vector<string>& jnames,
                                   int64_t startTime,
                                   const string& readerName);

    static JournalReaderPtr create(const vector<string>& dirs,
                                   const vector<string>& jnames,
                                   const vector<IJournalVisitor*>& visitors,
                                   int64_t startTime,
                                   const string& readerName);

    static JournalReaderPtr create(const string& dir,
                                   const string& jname,
                                   int64_t startTime,
                                   const string& readerName);

    static JournalReaderPtr create(const vector<string>& dirs,
                                   const vector<string>& jnames,
                                   int64_t startTime);

    static JournalReaderPtr create(const string& dir,
                                   const string& jname,
                                   int64_t startTime);

    static JournalReaderPtr createReaderWithSys(const vector<string>& dirs,
                                                const vector<string>& jnames,
                                                int64_t startTime,
                                                const string& readerName);

    static JournalReaderPtr createSysReader(const string& readerName);
    /** revisable reader is a reader with authority to revise data it reads */
    static JournalReaderPtr createRevisableReader(const string& readerName);

    static const string FILE_PREFIX;

};

inline FramePtr JournalReader::getNextFrame()
{
    int64_t  minNano = TIME_TO_LAST;
    void* res_address = nullptr;
    for (JournalPtr& journal: journals)
    {
        FrameHeader* header = (FrameHeader*)(journal->locateFrame());
        if (header != nullptr)
        {
            int64_t nano = header->nano;
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

/**
 * Journal Writer
 */
class JournalWriter : public JournalHandler
{
private:
    passive::emitter emitter_;

protected:
    /** the journal will write in */
    JournalPtr journal;
    /** private constructor */
    JournalWriter(PageProviderPtr ptr): JournalHandler(ptr) {}

public:
    /** init journal */
    void init(const string& dir, const string& jname);
    /** get current page number */
    short getPageNum() const;
    /* seek to the end of the journal
     * journal can only be appended in the back,
     * no modification of existing frames is allowed. */
    void seekEnd();
    /** to write a string into journal */
    int64_t writeStr(const string& str);
    /** same as write_frame, but address field is optimized to fit python binding */
    int64_t writePyData(uintptr_t data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                        FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId);

    /** write a frame with full information */
    virtual int64_t write_frame_full(const void* data, FH_TYPE_LENGTH length,
                                     FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                     FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                                     FH_TYPE_NANOTM extraNano, FH_TYPE_ERR_ID errorId,
                                     const char* errorMsg);
    /** write a basic frame */
    inline int64_t write_frame(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                               FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId)
    {
        return write_frame_full(data, length, source, msgType, lastFlag, requestId, 0, 0, nullptr);
    }
    /** write a frame with extra nano */
    inline int64_t write_frame_extra(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                                     FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag,
                                     FH_TYPE_REQ_ID requestId, FH_TYPE_NANOTM extraNano)
    {
        return write_frame_full(data, length, source, msgType, lastFlag, requestId, extraNano, 0, nullptr);
    }
    /** write a error frame */
    inline int64_t write_error_frame(const void* data, FH_TYPE_LENGTH length, FH_TYPE_SOURCE source,
                                     FH_TYPE_MSG_TP msgType, FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                                     FH_TYPE_ERR_ID errorId, const char* errorMsg)
    {
        return write_frame_full(data, length, source, msgType, lastFlag, requestId, 0, errorId, errorMsg);
    }
public:
    // creators
    static JournalWriterPtr create(const string& dir, const string& jname, const string& writerName);
    static JournalWriterPtr create(const string& dir, const string& jname, const string& writerName, const bool client);
    static JournalWriterPtr create(const string& dir, const string& jname, PageProviderPtr ptr);
    static JournalWriterPtr create(const string& dir, const string& jname);

public:
    static const string FILE_PREFIX;
};


class JournalSafeWriter: public JournalWriter
{
protected:
    JournalSafeWriter(PageProviderPtr ptr): JournalWriter(ptr) {}

public:
    /** write a frame with full information */
    virtual int64_t write_frame_full(const void* data, FH_TYPE_LENGTH length,
                                     FH_TYPE_SOURCE source, FH_TYPE_MSG_TP msgType,
                                     FH_TYPE_LASTFG lastFlag, FH_TYPE_REQ_ID requestId,
                                     FH_TYPE_NANOTM extraNano, FH_TYPE_ERR_ID errorId,
                                     const char* errorMsg);

    // create a thread safe writer (with mutex in write_frame)
    static JournalWriterPtr create(const string& dir, const string& jname, const string& writerName);
};

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
#endif //YIJINJING_JOURNAL_H
