//
// Created by Keren Dong on 2019-06-04.
//

#ifndef KUNGFU_YIJINJING_JOURNAL_COMMON_H
#define KUNGFU_YIJINJING_JOURNAL_COMMON_H

#include <kungfu/yijinjing/common.h>

#define __JOURNAL_VERSION__ 4

namespace kungfu
{

    namespace yijinjing
    {

        namespace journal
        {

            FORWARD_DECLARE_PTR(frame)

            FORWARD_DECLARE_PTR(page)

            FORWARD_DECLARE_PTR(journal)

            FORWARD_DECLARE_PTR(reader)

            FORWARD_DECLARE_PTR(writer)

            class journal_error : public std::runtime_error
            {
            public:
                journal_error(const std::string &message) : runtime_error(message)
                {}
            };
        }
    }
}

#endif //KUNGFU_YIJINJING_JOURNAL_COMMON_H
