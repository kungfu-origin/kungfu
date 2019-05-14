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
 * Page Service Tasks.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Page engine needs several tasks to be done in schedule.
 * here we define tasks which can be implemented in page engine
 */

#ifndef YIJINJING_PAGESERVICETASK_H
#define YIJINJING_PAGESERVICETASK_H

#include "YJJ_DECLARE.h"
#include "Log.h"

#include <pybind11/pybind11.h>
PYBIND11_DECLARE_HOLDER_TYPE(T, boost::shared_ptr<T>);
#include <pybind11/stl.h>
namespace py = pybind11;

YJJ_NAMESPACE_START

#define TEMP_PAGE KUNGFU_JOURNAL_FOLDER + "TEMP_PAGE"

class PageService;

class PstBase
{
public:
    virtual void go() {};
    virtual string getName() const { return "Base"; };
    virtual pybind11::dict getInfo() const { return pybind11::dict(); }
    virtual ~PstBase() {};
};
DECLARE_PTR(PstBase);

class PstPidCheck: public PstBase
{
public:
    PstPidCheck(PageService* pe);
    void go();
    string getName() const { return "PidCheck"; }
private:
    PageService* engine;
};
DECLARE_PTR(PstPidCheck);

class PstTimeTick: public PstBase
{
public:
    PstTimeTick(PageService* pe);
    void go();
    string getName() const { return "TimeTick"; }
private:
    PageService* engine;
};
DECLARE_PTR(PstTimeTick);

class PstTempPage: public PstBase
{
public:
    PstTempPage(PageService* pe);
    void go();
    string getName() const { return "TempPage"; }
private:
    PageService* engine;
};
DECLARE_PTR(PstTempPage);

YJJ_NAMESPACE_END

#endif //YIJINJING_PAGESERVICETASK_H
