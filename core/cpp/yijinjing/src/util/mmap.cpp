//
// Created by Keren Dong on 2019-06-10.
//

#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif // _WINDOWS

#include <regex>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/journal/common.h>
#include <kungfu/yijinjing/util/os.h>

using namespace kungfu::yijinjing::journal;

namespace kungfu {

    namespace yijinjing {

        namespace os {

            uintptr_t load_mmap_buffer(const std::string &path, size_t size, bool is_writing, bool lazy)
            {
#ifdef _WINDOWS
                HANDLE dumpFileDescriptor = CreateFileA(path.c_str(),
                    (is_writing) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    (is_writing) ? OPEN_ALWAYS : OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

                if (dumpFileDescriptor == INVALID_HANDLE_VALUE)
                {
                    if (!is_writing) return nullptr;

                    printf("Cannot create/write the file");
                    exit(EXIT_FAILURE);
                }


                HANDLE fileMappingObject = CreateFileMapping(dumpFileDescriptor,
                    NULL,
                    (is_writing) ? PAGE_READWRITE:PAGE_READONLY,
                    0,
                    size,
                    NULL);

                if (fileMappingObject == NULL)
                {
                    int nRet = GetLastError();
                    printf("LoadPageBuffer fail(%s): CreateFileMapping Error = %d, %s\n", is_writing ? "writer" : "reader", nRet, path.c_str());
                    return nullptr;
                    //exit(EXIT_FAILURE);
                }


                void* buffer = MapViewOfFile(fileMappingObject,
                    (is_writing) ? FILE_MAP_ALL_ACCESS:FILE_MAP_READ,
                    0,
                    0,
                    size);

                if (buffer == nullptr)
                {
                    int nRet = GetLastError();
                    throw exception("failed to load page " + path + ", MapViewOfFile Error " + std::to_string(nRet));
                }

                SPDLOG_DEBUG("loaded buffer for page {}", path);
                return reinterpret_cast<uintptr_t>(buffer);
#else
                int fd = open(path.c_str(), (is_writing ? O_RDWR : O_RDONLY) | O_CREAT, (mode_t) 0600);
                if (fd < 0)
                {
                    throw journal_error("failed to open file for page " + path);
                }

                if (/*!lazy &&*/ is_writing)
                {
                    if (lseek(fd, size - 1, SEEK_SET) == -1)
                    {
                        close(fd);
                        throw journal_error("failed to stretch for page " + path);
                    }
                    if (write(fd, "", 1) == -1)
                    {
                        close(fd);
                        throw journal_error("unable to write for page " + path);
                    }
                }

                /**
                 * MAP_FIXED is dup2 for memory mappings, and it's useful in exactly the same situations where dup2 is useful for file descriptors:
                 * when you want to perform a replace operation that atomically reassigns a resource identifier (memory range in the case of MAP_FIXED,
                 * or fd in the case of dup2) to refer to a new resource without the possibility of races where it might get reassigned to something
                 * else if you first released the old resource then attempted to regain it for the new resource.
                 */
                void *buffer = mmap(0, size, (is_writing) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);

                if (buffer == MAP_FAILED)
                {
                    close(fd);
                    throw journal_error("Error mapping file to buffer");
                }

                if (!lazy && madvise(buffer, size, MADV_RANDOM) != 0 && mlock(buffer, size) != 0)
                {
                    munmap(buffer, size);
                    close(fd);
                    throw journal_error("failed to lock memory for page " + path);
                }

                close(fd);
                SPDLOG_DEBUG("loaded buffer for page {}, writing {}, lazy {}", path, is_writing, lazy);
                return reinterpret_cast<uintptr_t>(buffer);
#endif // _WINDOWS
            }

            bool release_mmap_buffer(uintptr_t address, size_t size, bool lazy)
            {
                void *buffer = reinterpret_cast<void *>(address);
#ifdef _WINDOWS
                UnmapViewOfFile(buffer);
#else
                //unlock and unmap
                if (!lazy && munlock(buffer, size) != 0)
                {
                    return false;
                }

                if (munmap(buffer, size) != 0)
                {
                    return false;
                }
#endif // _WINDOWS
                return true;
            }

        }
    }
}
