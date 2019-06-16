//
// Created by Keren Dong on 2019-06-10.
//

#ifdef __llvm__
#include <execinfo.h>
#endif

#include <kungfu/yijinjing/util/os.h>

namespace kungfu {

    namespace yijinjing {

        namespace util {

            void print_backtrace()
            {
#ifdef __llvm__
                void* callstack[128];
                int i, frames = backtrace(callstack, 128);
                char** strs = backtrace_symbols(callstack, frames);
                for (i = 0; i < frames; ++i) {
                    printf("%s\n", strs[i]);
                }
                free(strs);
#endif
            }
        }
    }
}
