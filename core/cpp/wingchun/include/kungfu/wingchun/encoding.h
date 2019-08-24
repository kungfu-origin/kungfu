//
// Created by PolarAir on 2019-08-24.
//

#ifndef KUNGFU_ENCODING_H
#define KUNGFU_ENCODING_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <iconv.h>
#endif

namespace kungfu
{
    namespace wingchun
    {
#ifdef _WIN32
        inline std::string gbk2utf8(const std::string& src_str)
        {
            int len = MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, NULL, 0);
            wchar_t* wstr = new wchar_t[len + 1];
            memset(wstr, 0, len + 1);
            MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, wstr, len);
            len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
            char* str = new char[len + 1];
            memset(str, 0, len + 1);
            WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
            std::string strTemp = str;
            if (wstr) delete[] wstr;
            if (str) delete[] str;
            return strTemp;
        }
#else
        inline std::string gbk2utf8(const std::string& src_str)
        {
            iconv_t conv = iconv_open("utf8", "gbk");
            if (conv == (iconv_t)-1)
                return "";

            char* src = (char*)src_str.c_str();
            char dst[4096] = {0};
            char* out = dst;
            size_t inbytes = strlen(src);
            size_t outbytes = 4096;

            if (iconv(conv, &src, &inbytes, &out, &outbytes) == (size_t)-1)
            {
                return "";
            }
            iconv_close(conv);

            dst[4096 - outbytes] = 0;

            return std::string(dst);
        }
#endif
    }
}

#endif //KUNGFU_ENCODING_H
