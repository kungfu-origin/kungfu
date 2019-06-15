//
// Created by Keren Dong on 2019-06-04.
//

#ifndef KUNGFU_YIJINJING_JOURNAL_COMMON_H
#define KUNGFU_YIJINJING_JOURNAL_COMMON_H

#include <kungfu/yijinjing/common.h>

#define JOURNAL_PREFIX std::string("yjj")        /** journal file prefix */
#define JOURNAL_SUFFIX std::string("journal")    /** journal file suffix */

#define __JOURNAL_VERSION__ 3

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

            class journal_error : public std::exception
            {
            public:
                journal_error(const std::string &message) : message_(message)
                {}

                virtual const char *what() const throw()
                { return message_.c_str(); };

            private:
                const std::string message_;
            };
        }
    }
}

#endif //KUNGFU_YIJINJING_JOURNAL_COMMON_H
