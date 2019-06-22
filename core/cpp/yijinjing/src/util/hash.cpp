//
// Created by Keren Dong on 2019-06-10.
//

#include <string>

#include <kungfu/yijinjing/util/util.h>
#include <kungfu/yijinjing/util/MurmurHash3.h>

namespace kungfu
{

    namespace yijinjing
    {

        namespace util
        {
            uint32_t hash_32(const unsigned char *key, int32_t length, uint32_t seed)
            {
                uint32_t h;
                MurmurHash3_x86_32(key, length, seed, &h);
                return h;
            }

            uint32_t hash_str_32(const std::string &key, uint32_t seed)
            {
                return hash_32(reinterpret_cast<const unsigned char *>(key.c_str()), key.length(), seed);
            }
        }
    }
}