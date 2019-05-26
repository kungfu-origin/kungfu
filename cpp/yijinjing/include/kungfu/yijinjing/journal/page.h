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
 * Page in Journal.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Page handle single page to provide continues journal abstraction
 */

#ifndef YIJINJING_PAGE_H
#define YIJINJING_PAGE_H

#include <nlohmann/json.hpp>

#include <kungfu/yijinjing/common.h>
#include <kungfu/yijinjing/journal/frame.h>
#include <kungfu/yijinjing/nanomsg/socket.h>
#include <kungfu/yijinjing/nanomsg/passive.h>

YJJ_NAMESPACE_START

/** reserve space for page header (long) */
#define PAGE_HEADER_RESERVE 10
//////////////////////////////////////////
/// (byte) JournalPageStatus
//////////////////////////////////////////
#define JOURNAL_PAGE_STATUS_RAW     0
#define JOURNAL_PAGE_STATUS_INITED  1

#define SOCKET_MESSAGE_MAX_LENGTH       1024 /**< max length of a socket buffer */

struct PageHeader
{
    /** JournalPageStatus */
    int8_t  status;
    /** journal name */
    char    journal_name[JOURNAL_SHORT_NAME_MAX_LENGTH];
    /** number of this page in journal */
    int16_t page_num;
    /** nano time of when the page started */
    int64_t start_nano;
    /** nano time of when the page closed */
    int64_t close_nano;
    /** how many frame in this page (only filled when closed) */
    int32_t frame_num;
    /** pos of last frame */
    int32_t last_pos;
    /** version of frame header (using reserve)*/
    int16_t frame_version;
    /** reserve space */
    int16_t reserve_short[3];
    int64_t reserve_long[PAGE_HEADER_RESERVE - 1];
#ifndef _WIN32
} __attribute__((packed));
#else
};
#pragma pack(pop)
#endif

enum PageStatus : int8_t
{
    // status in process 0 ~ 9
    PAGE_RAW = 0,                       /**< this msg block is not allocated (default) */
    PAGE_OCCUPIED = 1,                  /**< comm msg idx occupied (by server) */
    PAGE_HOLDING = 2,                   /**< folder / name ready (by client) */
    PAGE_REQUESTING = 3,                /**< page number specified (by client) */
    PAGE_ALLOCATED = 4,                 /**< finish allocated, user may getPage (by server) */
    // failures 10 ~ 19
    PAGE_NON_EXIST = 11,                /**< default position */
    PAGE_MEM_OVERFLOW = 12,             /**< default position */
    PAGE_MORE_THAN_ONE_WRITE = 13,      /**< default position */
    PAGE_CANNOT_RENAME_FROM_TEMP = 14   /**< default position */
};

/**
 * PageUtil is more likely a namespace for utility functions
 * more details are provided for each function
 * all functions are open for usage
 */
class PageUtil
{
public:
    // memory access
    /** direct memory manipulation without service
     * load page buffer, return address of the file-mapped memory
     *  whether to write has to be specified in "isWriting"
     *  if quickMode==True, no locking; if quickMode==False, mlock the memory for performance
     * the address of memory is returned */
    static void*  LoadPageBuffer(const string& path, int size, bool isWriting, bool quickMode);
    /** direct memory manipulation without service
     * release page buffer, buffer and size needs to be specified.
     *  if quickMode==True, no unlocking; if quickMode==False, munlock the memory */
    static void   ReleasePageBuffer(void* buffer, int size, bool quickMode);

    // name / pattern
    /** generate proper yjj file name by necessary information */
    static string GenPageFileName(const string& jname, short pageNum);
    /** generate proper yjj page full path by necessary information */
    static string GenPageFullPath(const string& dir, const string& jname, short pageNum);
    /** get the proper yjj file name pattern */
    static string GetPageFileNamePattern(const string& jname);

    // page number
    /** extract page number from file name */
    static short  ExtractPageNum(const string& filename, const string& jname);
    /** select page number from existing pages in directory which contains the nano time */
    static short  GetPageNumWithTime(const string& dir, const string& jname, int64_t time);
    /** get existing page numbers in directory with jname */
    static vector<short> GetPageNums(const string& dir, const string& jname);

    // header
    /** get header from necessary information */
    static PageHeader GetPageHeader(const string& dir, const string& jname, short pageNum);

    // file
    static bool FileExists(const string& filename);
};

/**
 * Page class
 */
FORWARD_DECLARE_PTR(Page);
class Page
{
private:
    /** current frame */
    Frame frame;
    /** address of mmap-file associated with this page */
    void * const buffer;
    /** current position in page */
    int position;
    /** number index of current frame in the page */
    int frameNum;
    /** number of the page for the journal */
    short pageNum;

    /** private constructor */
    Page(void *buffer);

private:
    /** internal usage */
    inline FH_TYPE_STATUS getCurStatus() const
    {
        return frame.getStatus();
    }

public:
    /** get page buffer */
    inline void* getBuffer() { return buffer; }
    /** get current page number */
    inline short getPageNum() const { return pageNum; };

    /** setup the page when finished */
    void finishPage();
    /** get writable frame address (enough space & clean)*/
    void *locateWritableFrame();
    /** get wrote frame address
     * return nullptr if no more frame */
    void *locateReadableFrame();
    /** current page is end */
    bool isAtPageEnd() const;
    /** move forward to next frame */
    void passFrame();
    /** pass all wrote frame */
    void passWrittenFrame();
    /** pass frame to nano_time */
    void passToTime(int64_t time);

public:
    /** load page, should be called by PageProvider
      * will not lock memory if in quickMode (locked by page engine service) */
    static  PagePtr load(const string& dir, const string& jname, short pageNum, bool isWriting, bool quickMode);
};


inline bool Page::isAtPageEnd() const
{
    return getCurStatus() == JOURNAL_FRAME_STATUS_PAGE_END;
}

inline void Page::passFrame()
{
    position += frame.next();
    frameNum += 1;
}

inline void Page::passWrittenFrame()
{
    while (getCurStatus() == JOURNAL_FRAME_STATUS_WRITTEN)
        passFrame();
}

inline void Page::passToTime(int64_t time)
{
    while (getCurStatus() == JOURNAL_FRAME_STATUS_WRITTEN && frame.getNano() < time)
        passFrame();
}

inline void* Page::locateWritableFrame()
{
    passWrittenFrame();
    return (getCurStatus() == JOURNAL_FRAME_STATUS_RAW
            && (position + PAGE_MIN_HEADROOM < JOURNAL_PAGE_SIZE))
           ? frame.get_address(): nullptr;
}

inline void* Page::locateReadableFrame()
{
    return (getCurStatus() == JOURNAL_FRAME_STATUS_WRITTEN)
           ? frame.get_address(): nullptr;
}


/** abstract interface class */
class IPageProvider
{
public:
    /** return wrapped Page via directory / journal short name / serviceIdx assigned / page number */
    virtual PagePtr getPage(const string &dir, const string &jname, int serviceIdx, short pageNum) = 0;
    /** release page after using */
    virtual void releasePage(void* buffer, int size, int serviceIdx) = 0;
    /** return true if this is for writing */
    virtual bool isWriter() const = 0;
    /** destructor */
    virtual ~IPageProvider() {};
};
DECLARE_PTR(IPageProvider);

/**
 * PageProvider,
 * abstract class with virtual interfaces,
 * utilized by JournalHandler
 */
class PageProvider: public IPageProvider
{
protected:
    /** true if provider is used by a JournalWriter */
    bool    is_writer_;
    /** true if it is allowed to revise */
    bool    revise_allowed_;
public:
    /** register journal when added into JournalHandler */
    virtual int  register_journal(const string& dir, const string& jname) { return -1; };
    /** exit client after JournalHandler is released */
    virtual void exit_client() {};
    /** override IPageProvider */
    virtual bool isWriter() const {return is_writer_; };
};

DECLARE_PTR(PageProvider);

/**
 * LocalPageProvider,
 * provide local page, no need to connect with service.
 */
class LocalPageProvider: public PageProvider
{
public:
    /** constructor */
    LocalPageProvider(bool is_writer, bool revise_allowed=false);
    /** override IPageProvider */
    virtual PagePtr getPage(const string &dir, const string &jname, int service_id, short page_num);
    /** override IPageProvider */
    virtual void releasePage(void* buffer, int size, int service_id);
};

/**
 * ClientPageProvider,
 * provide page via memory service, socket & comm
 */
class ClientPageProvider: public PageProvider
{
public:
    /** default constructor with client name and writing flag */
    ClientPageProvider(const string& client_name, bool is_writer, bool revise_allowed=false);
    /** register to service as a client */
    void register_client();
    /** override PageProvider */
    virtual int  register_journal(const string& dir, const string& jname);
    /** override PageProvider */
    virtual void exit_client();
    /** override IPageProvider */
    virtual PagePtr getPage(const string &dir, const string &jname, int service_id, short page_num);
    /** override IPageProvider */
    virtual void releasePage(void* buffer, int size, int service_id);

private:
    string  client_name_;
    nanomsg::socket     client_request_socket_;
    passive::emitter emitter_;
    nlohmann::json request_;
    void*   memory_msg_buffer_;
    int     hash_code_;

    nlohmann::json request(const string &path);
};
YJJ_NAMESPACE_END

#endif //YIJINJING_PAGE_H
