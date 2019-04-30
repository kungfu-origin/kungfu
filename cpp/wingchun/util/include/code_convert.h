//
// Created by qlu on 2019/1/14.
//

#ifndef WC_2_CODE_CONVERT_H
#define WC_2_CODE_CONVERT_H

#include <boost/locale.hpp>

namespace kungfu
{
    inline std::string gbk2utf8(const char* in)
    {
        return boost::locale::conv::between(std::string(in), "UTF-8", "GBK");
    }
}
#endif //WC_2_CODE_CONVERT_H
