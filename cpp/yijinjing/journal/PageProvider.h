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
 * Page Provider.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * implements IPageProvider, diverge to different usage
 */

#ifndef YIJINJING_PAGEPROVIDER_H
#define YIJINJING_PAGEPROVIDER_H

#include "passive.h"
#include "IPageProvider.h"

#include <nlohmann/json.hpp>

YJJ_NAMESPACE_START

#define SOCKET_MESSAGE_MAX_LENGTH       1024 /**< max length of a socket buffer */

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
    nn::socket     client_request_socket_;
    passive::emitter emitter_;
    nlohmann::json request_;
    char response_buf[SOCKET_MESSAGE_MAX_LENGTH];
    string  response_str_;
    void*   memory_msg_buffer_;
    int     hash_code_;

    nlohmann::json request(const string &path);
};

YJJ_NAMESPACE_END

#endif //YIJINJING_PAGEPROVIDER_H
