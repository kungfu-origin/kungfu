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
 * IPageProvider.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Provide page, mainly utilized by journal
 */

#ifndef YIJINJING_IPAGEPROVIDER_H
#define YIJINJING_IPAGEPROVIDER_H

#include "YJJ_DECLARE.h"

YJJ_NAMESPACE_START

FORWARD_DECLARE_PTR(Page);

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

YJJ_NAMESPACE_END

#endif //YIJINJING_IPAGEPROVIDER_H
