//
// Created by Keren Dong on 2019-06-04.
//

#ifndef KUNGFU_YIJINJING_OS_H
#define KUNGFU_YIJINJING_OS_H

#include <string>
#include <initializer_list>

namespace kungfu
{
    namespace yijinjing
    {
        namespace os
        {
            /**
             * load mmap buffer, return address of the file-mapped memory
             * whether to write has to be specified in "is_writing"
             * buffer memory is locked if not lazy
             * @return the address of mapped memory
             */
            uintptr_t load_mmap_buffer(const std::string &path, size_t size, bool is_writing = false, bool lazy = true);

            bool release_mmap_buffer(uintptr_t address, size_t size, bool lazy);

            void handle_os_signals(void *hero);
        }
    }
}

#endif //KUNGFU_YIJINJING_OS_H
