//
// Created by qlu on 2019/3/7.
//

#ifndef KUNGFU_FILESYSTEM_UTIL_H
#define KUNGFU_FILESYSTEM_UTIL_H

#include <boost/filesystem.hpp>

namespace kungfu
{
    inline bool create_folder_if_not_exists(const std::string& path)
    {
        boost::filesystem::path _path(path);
        if (!boost::filesystem::exists(_path))
        {
            return boost::filesystem::create_directories(_path);
        }
        else
        {
            return boost::filesystem::is_directory(_path);
        }
    }
}
#endif //KUNGFU_FILESYSTEM_UTIL_H
