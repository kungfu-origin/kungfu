//
// Created by dkr on 8/27/2019.
//

#ifndef KUNGFU_STACKTRACE_H
#define KUNGFU_STACKTRACE_H

#ifdef _WINDOWS
#include <cstdio>
#include <Windows.h>
#include <Psapi.h>
#endif // _WINDOWS

namespace kungfu {
    namespace yijinjing {
        namespace util {

#ifdef _WINDOWS
            DWORD print_stack_trace(EXCEPTION_POINTERS *ep);
#else
            void print_stack_trace(FILE *out = stderr);
#endif // _WINDOWS
        }
    }
}

#endif //KUNGFU_STACKTRACE_H
