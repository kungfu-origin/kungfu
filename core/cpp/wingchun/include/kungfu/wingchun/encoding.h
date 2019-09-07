//
// Created by PolarAir on 2019-08-24.
//

#ifndef KUNGFU_ENCODING_H
#define KUNGFU_ENCODING_H

#include<locale>
#include<codecvt>

#ifdef _WINDOWS
#define GB_ENCODING_NAME "zh-CN"
#else
#define GB_ENCODING_NAME "zh_CN.GB18030"
#endif

namespace kungfu
{
    namespace wingchun
    {
        // utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
        template<class Facet>
        struct deletable_facet : Facet
        {
            template<class ...Args>
            deletable_facet(Args &&...args) : Facet(std::forward<Args>(args)...)
            {}

            ~deletable_facet()
            {}
        };

        typedef deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> codec_facet_t;

        inline std::string gbk2utf8(const std::string &str)
        {
            std::wstring_convert<codec_facet_t> cv_gb(new codec_facet_t(GB_ENCODING_NAME));
            std::wstring_convert<std::codecvt_utf8<wchar_t>> cv_utf8;
            return cv_utf8.to_bytes(cv_gb.from_bytes(str));
        }
    }
}

#endif //KUNGFU_ENCODING_H
