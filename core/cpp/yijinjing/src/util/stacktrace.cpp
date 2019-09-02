//
// Created by dkr on 8/30/2019.
//

#include <spdlog/spdlog.h>

#ifdef _WINDOWS

#include <kungfu/yijinjing/util/stacktrace.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

#include <kungfu/yijinjing/util/StackWalker.h>

#else
#include <cerrno>
#include <execinfo.h>
#include <cxxabi.h>
#endif // _WINDOWS

namespace kungfu
{
    namespace yijinjing
    {
        namespace util
        {

#ifdef _WINDOWS
            DWORD print_stack_trace(EXCEPTION_POINTERS *ep)
            {
                SPDLOG_CRITICAL("Uncaught exception");
                StackWalker sw;
                sw.ShowCallstack();
                return EXCEPTION_EXECUTE_HANDLER;
            }
#else
            void print_stack_trace(FILE *out = stderr) {
                unsigned int max_frames = 127;
                // storage array for stack trace address data
                void *addrlist[max_frames + 1];

                // retrieve current stack addresses
                unsigned int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

                if (addrlen == 0) {
                    fprintf(out, "  \n");
                    return;
                }

                // resolve addresses into strings containing "filename(function+address)",
                // Actually it will be ## program address function + offset
                // this array must be free()-ed
                char **symbollist = backtrace_symbols(addrlist, addrlen);

                // iterate over the returned symbol lines. skip the first, it is the
                // address of this function.
                for (unsigned int i = 4; i < addrlen; i++) {
                    char *begin_name = nullptr;
                    char *begin_offset = nullptr;
                    char *end_offset = nullptr;

                    // find parentheses and +address offset surrounding the mangled name
#ifdef __APPLE__
                    // OSX style stack trace
                    for (char *p = symbollist[i]; *p; ++p)
                    {
                        if ((*p == '_') && (*(p - 1) == ' '))
                            begin_name = p - 1;
                        else if (*p == '+')
                            begin_offset = p - 1;
                    }

                    if (begin_name && begin_offset && (begin_name < begin_offset))
                    {
                        *begin_name++ = '\0';
                        *begin_offset++ = '\0';

                        // mangled name is now in [begin_name, begin_offset) and caller
                        // offset in [begin_offset, end_offset). now apply
                        // __cxa_demangle():
                        int status;
                        size_t funcnamesize = 8192;
                        char funcname[8192];
                        char *ret = abi::__cxa_demangle(begin_name, &funcname[0], &funcnamesize, &status);
                        SPDLOG_CRITICAL(" {:<30} {:<40} {}", symbollist[i], status == 0 ? ret : begin_name, begin_offset);
#else // !__APPLE__ - but is posix
                    // not OSX style
                    // ./module(function+0x15c) [0x8048a6d]
                    for (char *p = symbollist[i]; *p; ++p) {
                        if (*p == '(')
                            begin_name = p;
                        else if (*p == '+')
                            begin_offset = p;
                        else if (*p == ')' && (begin_offset || begin_name))
                            end_offset = p;
                    }

                    if (begin_name && end_offset && (begin_name > end_offset)) {
                        *begin_name++ = '\0';
                        *end_offset++ = '\0';
                        if (begin_offset)
                            *begin_offset++ = '\0';

                        // mangled name is now in [begin_name, begin_offset) and caller
                        // offset in [begin_offset, end_offset). now apply
                        // __cxa_demangle():

                        int status = 0;
                        size_t funcnamesize = 8192;
                        char funcname[8192];
                        char *ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
                        SPDLOG_CRITICAL("{}", ret);
                        SPDLOG_CRITICAL("{:<30} ( {:<40}  + {:<6}) {}", symbollist[i], status == 0 ? ret : begin_name, begin_offset ? begin_offset : "", end_offset);
#endif  // !DARWIN - but is posix
                    } else {
                        // couldn't parse the line? print the whole line.
                        SPDLOG_CRITICAL("{}", symbollist[i]);
                    }
                }
                free(symbollist);
            }
#endif // _WINDOWS
        }
    }
}
