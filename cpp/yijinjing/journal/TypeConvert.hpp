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
 * Define Type Convert utils here.
 * @Author qlu (qing.lu@taurus.ai)
 * @since  September, 2016
 */

#ifndef YIJINJING_TYPECONVERTOR_H
#define YIJINJING_TYPECONVERTOR_H

#include <vector>

#include "YJJ_DECLARE.h"
#include <boost/locale.hpp>
namespace py = pybind11;
YJJ_NAMESPACE_START

/**
 * extract int from python dict with key, store in value
 */
inline void getInt(const py::dict &d, const string &key, int *value)
{
    const char* c = key.c_str();
    if (d.contains(c))
    {
        py::object o = d[c];
        if (py::isinstance<py::int_>(o))
        {
            *value = o.cast<int32_t>();
        }
    }
}

/**
 * extract int64_t from python dict with key, store in value
 */
inline void getLong(const py::dict &d, const string &key, int64_t *value)
{
    const char* c = key.c_str();
    if (d.contains(c))
    {
        py::object o = d[c];
        if (py::isinstance<py::int_>(o))
        {
            *value = o.cast<int64_t>();
        }
    }
}

/**
 * extract short from python dict with key, store in value
 */
inline void getShort(const py::dict &d, const string &key, short *value)
{
    const char* c = key.c_str();
    if (d.contains(c))
    {
        py::object o = d[c];
        if (py::isinstance<py::int_>(o))
        {
            *value = o.cast<int16_t>();
        }
    }
}

/**
 * extract double from python dict with key, store in value
 */
inline void getDouble(const py::dict &d, const string &key, double *value)
{
    const char* c = key.c_str();
    if (d.contains(c))
    {
        py::object o = d[c];
        if (py::isinstance<py::float_>(o))
        {
            *value = o.cast<double>();
        }
    }
}

/**
 * extract char from python dict with key, store in value
 */
inline void getChar(const py::dict &d, const string &key, char *value)
{
    const char* c = key.c_str();
    if (d.contains(c))
    {
        py::object o = d[c];
        if (py::isinstance<py::str>(o))
        {
            string s = o.cast<std::string>();
            const char *buffer = s.c_str();
            *value = *buffer;
        }
    }
}

/**
 * extract string(char*) from python dict with key, store in value
 */
inline void getString(const py::dict &d, const string &key, char *value)
{
    const char* c = key.c_str();
    if (d.contains(c))
    {
        py::object o = d[c];
        if (py::isinstance<py::str>(o))
        {
            string s = o.cast<std::string>();
            const char *buffer = s.c_str();
            strcpy(value, buffer);
        }
    }
}

/**
 * convert gbk to utf8
 */
inline string gbk2utf8(const string& str)
{
    return boost::locale::conv::between(str, "UTF-8", "GBK");
}

YJJ_NAMESPACE_END

#endif //YIJINJING_TYPECONVERT_H
