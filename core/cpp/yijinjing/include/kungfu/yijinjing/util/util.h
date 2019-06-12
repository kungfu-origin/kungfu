//
// Created by Keren Dong on 2019-06-04.
//

#ifndef KUNGFU_YIJINJING_UTIL_H
#define KUNGFU_YIJINJING_UTIL_H

#include <kungfu/yijinjing/journal/page.h>

#define KUNGFU_HASH_SEED 97

namespace kungfu
{
    namespace yijinjing
    {
        namespace util
        {
            /**
             * Murmur Hash 2
             * @param key content to be hashed
             * @param len length of key
             * @param seed
             * @return hash result
             */
            uint32_t hash(const void *key, int32_t len, uint32_t seed);

            std::string make_path(std::initializer_list<std::string> list, bool is_file=false);

            std::vector<int> list_journal_page_id(const std::string &path, const std::string &name);

            void print_backtrace();

            void color_print(std::string level, std::string log);

            bool in_color_terminal();
        }
    }
}

#endif //KUNGFU_YIJINJING_UTIL_H
