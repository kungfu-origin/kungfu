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
        /*
        inline int gbk2utf8_convert(char *utfStr, size_t maxUtfStrlen, const char *srcStr)
        {
            if(!srcStr||!utfStr)
            {
                return -1;
            }

#if defined(_WIN32) || defined(_WIN64)
            int len = MultiByteToWideChar(CP_ACP, 0, (LPCCH)srcStr, -1, NULL,0);
            unsigned short * strUnicode = new unsigned short[len+1];
            memset(strUnicode, 0, len * 2 + 2);
            MultiByteToWideChar(CP_ACP, 0, (LPCCH)srcStr, -1, (LPWSTR)strUnicode, len);
            len = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)strUnicode, -1, NULL, 0, NULL, NULL);
            if (len > (int)maxUtfStrlen)
            {
                delete[] strUnicode;
                return -1;
            }
            WideCharToMultiByte (CP_UTF8, 0, (LPWSTR)strUnicode, -1, utfStr, len, NULL,NULL);
            delete[] strUnicode;
            return len;
#else
            if(nullptr == setlocale(LC_ALL,"zh_CN.gbk"))
            {
                return -1;
            }

            int unicodeLen = mbstowcs(NULL,srcStr,0);
            if(unicodeLen <= 0)
            {
                return -1;
            }
            wchar_t *unicodeStr = (wchar_t *)calloc(sizeof(wchar_t),unicodeLen+1);
            mbstowcs(unicodeStr,srcStr,strlen(srcStr));

            if(nullptr == setlocale(LC_ALL,"zh_CN.utf8"))
            {
                free(unicodeStr);
                return -1;
            }
            int utfLen=wcstombs(NULL,unicodeStr,0);
            if(utfLen<=0)
            {
                free(unicodeStr);
                return -1;
            }
            else if(utfLen>=(int)maxUtfStrlen)
            {
                free(unicodeStr);
                return -1;
            }
            wcstombs(utfStr,unicodeStr,utfLen);
            utfStr[utfLen] = 0;
            free(unicodeStr);
            return utfLen;
#endif
        }

        inline std::string gbk2utf8(const std::string& srcStr)
        {
            if (srcStr.empty())
            {
                return "";
            }
            int tarLen = srcStr.size() * 2 + 1;
            char *tarStr = new char[tarLen];
            gbk2utf8_convert(tarStr, tarLen - 1, srcStr.c_str());
            std::string target = tarStr;
            delete []tarStr;
            return target;
        }
         */

#ifdef _WIN32
        std::string gbk2utf8(const std::string& src_str)
        {
            int len = MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, NULL, 0);
            wchar_t* wstr = new wchar_t[len + 1];
            memset(wstr, 0, len + 1);
            MultiByteToWideChar(CP_ACP, 0, src_str.c_str(), -1, wstr, len);
            len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
            char* str = new char[len + 1];
            memset(str, 0, len + 1);
            WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
            string strTemp = str;
            if (wstr) delete[] wstr;
            if (str) delete[] str;
            return strTemp;
        }
#else
        std::string gbk2utf8(const std::string& src_str)
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
