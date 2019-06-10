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

            const std::string get_page_path(const data::location &location, int id);
            std::vector<int> list_page_ids(const data::location &location);
            int find_page_id(const data::location &location, int64_t time);

            void color_print(std::string level, std::string log);

            bool in_color_terminal();
        }
    }
}

#endif //KUNGFU_YIJINJING_UTIL_H
