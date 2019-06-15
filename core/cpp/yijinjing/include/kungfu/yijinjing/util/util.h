//
// Created by Keren Dong on 2019-06-04.
//

#ifndef KUNGFU_YIJINJING_UTIL_H
#define KUNGFU_YIJINJING_UTIL_H

#define KUNGFU_HASH_SEED 42

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
            uint32_t hash_32(const unsigned char *key, int32_t length, uint32_t seed=KUNGFU_HASH_SEED);

            uint32_t hash_str_32(const std::string &key, uint32_t seed=KUNGFU_HASH_SEED);
            uint16_t hash_str_16_low(const std::string &key, uint32_t seed=KUNGFU_HASH_SEED);
            uint16_t hash_str_16_high(const std::string &key, uint32_t seed=KUNGFU_HASH_SEED);

            std::string make_path(std::initializer_list<std::string> list, bool is_file=false);

            std::vector<int> list_journal_page_id(const std::string &path, uint32_t dest_id);

            void print_backtrace();

            void color_print(std::string level, std::string log);

            bool in_color_terminal();
        }
    }
}

#endif //KUNGFU_YIJINJING_UTIL_H
