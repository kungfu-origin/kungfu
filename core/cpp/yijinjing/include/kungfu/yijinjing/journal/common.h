//
// Created by Keren Dong on 2019-06-04.
//

#ifndef KUNGFU_YIJINJING_JOURNAL_COMMON_H
#define KUNGFU_YIJINJING_JOURNAL_COMMON_H

#include <kungfu/yijinjing/common.h>

namespace kungfu {

    namespace yijinjing {

        namespace journal {

            class exception : public std::exception {
            public:
                exception(const std::string &message): message_(message) {}

                virtual const char *what() const throw () { return message_.c_str(); };

            private:
                const std::string message_;
            };

            class locator {
            public:
                virtual const std::string get_journal_dir(data::mode m, data::category c, const std::string &group) const = 0;
            };
            DECLARE_PTR(locator)

            FORWARD_DECLARE_PTR(frame)
            FORWARD_DECLARE_PTR(page)
            FORWARD_DECLARE_PTR(journal)
        }
    }
}

#endif //KUNGFU_YIJINJING_JOURNAL_COMMON_H
