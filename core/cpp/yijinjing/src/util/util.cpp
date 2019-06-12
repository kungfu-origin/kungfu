//
// Created by Keren Dong on 2019-06-10.
//

#ifdef __llvm__
#include <execinfo.h>
#endif

#include <regex>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/util/os.h>

using namespace kungfu::yijinjing::journal;

namespace kungfu {

    namespace yijinjing {

        namespace util {

            std::string make_path(std::initializer_list<std::string> list, bool is_file)
            {
                boost::filesystem::path target_path = get_kungfu_home();
                for (std::string dir : list)
                {
                    target_path /= dir;
                }
                boost::filesystem::path target_folder = is_file ? target_path.parent_path() : target_path;
                if (!boost::filesystem::exists(target_folder))
                {
                    boost::filesystem::create_directories(target_folder);
                }
                return target_path.string();
            }

            std::vector<int> list_journal_page_id(const std::string &path, const std::string &name)
            {
                std::string page_filename_regex = JOURNAL_PREFIX + "\\." + name + "\\.[0-9]+\\." + JOURNAL_SUFFIX;
                std::regex pattern(page_filename_regex);
                boost::filesystem::path journal_folder_path(path);
                std::vector<int> res;
                for (auto &file : boost::filesystem::directory_iterator(journal_folder_path))
                {
                    std::string filename = file.path().filename().string();
                    if (std::regex_match(filename.begin(), filename.end(), pattern))
                    {
                        int begin = JOURNAL_PREFIX.length() + name.length() + 2;
                        int end = filename.length() - JOURNAL_SUFFIX.length();
                        std::string page_id_str = filename.substr(begin, end);
                        res.push_back(atoi(page_id_str.c_str()));
                    }
                }
                std::sort(res.begin(), res.end());
                return res;
            }

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
