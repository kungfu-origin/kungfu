/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * Page Utility functions.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * for memory access, yjj-format file name, page number, etc.
 */

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

#include <cstdio>
#include <sstream>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include <kungfu/log/spdlog_config.h>
#include <kungfu/yijinjing/journal/page.h>

USING_YJJ_NAMESPACE

string PageUtil::GenPageFileName(const string &jname, short pageNum)
{
    return JOURNAL_PREFIX + "." + jname + "." + std::to_string(pageNum) + "." + JOURNAL_SUFFIX;
}

string PageUtil::GenPageFullPath(const string& dir, const string& jname, short pageNum)
{
    std::stringstream ss;
    ss << dir << "/" << GenPageFileName(jname, pageNum);
    return ss.str();
}

string PageUtil::GetPageFileNamePattern(const string &jname)
{
    return JOURNAL_PREFIX + "\\." + jname + "\\.[0-9]+\\." + JOURNAL_SUFFIX;
}

short PageUtil::ExtractPageNum(const string &filename, const string &jname)
{
    string numStr = filename.substr(JOURNAL_PREFIX.length() + jname.length() + 2, filename.length() - JOURNAL_SUFFIX.length());
    return (short) atoi(numStr.c_str());
}

vector<short> PageUtil::GetPageNums(const string& dir, const string& jname)
{
    string namePattern = GetPageFileNamePattern(jname);
    boost::filesystem::path p(dir);
    boost::regex pattern(namePattern);
    vector<short> res;
    for (auto &file : boost::filesystem::directory_iterator(p)) {
        string filename = file.path().filename().string();
        if (boost::regex_match(filename.begin(), filename.end(), pattern))
            res.push_back(PageUtil::ExtractPageNum(filename, jname));
    }
    std::sort(res.begin(), res.end());
    return res;
}

short PageUtil::GetPageNumWithTime(const string& dir, const string& jname, int64_t time)
{
    vector<short> pageNums = GetPageNums(dir, jname);
    for (int idx = pageNums.size() - 1; idx >= 0; idx--)
    {
        PageHeader header = GetPageHeader(dir, jname, pageNums[idx]);
        if (header.start_nano < time)
            return pageNums[idx];
    }
    return 1;
}

PageHeader PageUtil::GetPageHeader(const string &dir, const string &jname, short pageNum)
{
    PageHeader header;
    string path = PageUtil::GenPageFullPath(dir, jname, pageNum);
    FILE* pfile = fopen(path.c_str(), "rb");
    if (pfile == nullptr)
        perror("cannot open file in PageUtil::GetPageNumWithTime");
    size_t length = fread(&header, 1, sizeof(PageHeader), pfile);
    if (length != sizeof(PageHeader))
        perror("cannot get page header in PageUtil::GetPageNumWithTime");
    fclose(pfile);
    return header;
}

/*
 * memory manipulation (no service)
 */

void* PageUtil::LoadPageBuffer(const string& path, int size, bool isWriting, bool quickMode)
{
    boost::filesystem::path page_path = path;
    boost::filesystem::path page_folder_path = page_path.parent_path();
    if(!boost::filesystem::exists(page_folder_path)) {
        boost::filesystem::create_directories(page_folder_path);
    }
#ifdef _WINDOWS
    HANDLE dumpFileDescriptor = CreateFileA(path.c_str(),
		(isWriting) ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		(isWriting) ? OPEN_ALWAYS : OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (dumpFileDescriptor == INVALID_HANDLE_VALUE)
	{
		if (!isWriting) return nullptr;

		printf("Cannot create/write the file");
		exit(EXIT_FAILURE);
	}


	HANDLE fileMappingObject = CreateFileMapping(dumpFileDescriptor,
		NULL,
		(isWriting) ? PAGE_READWRITE:PAGE_READONLY,
		0,
		size,
		NULL);

	if (fileMappingObject == NULL)
	{
		int nRet = GetLastError();
		printf("LoadPageBuffer fail(%s): CreateFileMapping Error = %d, %s\n", isWriting ? "writer" : "reader", nRet, path.c_str());
		return nullptr;
		//exit(EXIT_FAILURE);
	}


	void* buffer = MapViewOfFile(fileMappingObject,
		(isWriting) ? FILE_MAP_ALL_ACCESS:FILE_MAP_READ,
		0,
		0,
		size);

	if (buffer == nullptr)
	{
		int nRet = GetLastError();
		printf("LoadPageBuffer fail(%s): MapViewOfFile Error = %d, %s\n", isWriting ? "writer" : "reader", nRet, path.c_str());
		return nullptr;
	}

    return buffer;
#else
    int fd = open(path.c_str(), (isWriting) ? (O_RDWR | O_CREAT) : O_RDONLY, (mode_t)0600);

    if (fd < 0)
    {
        if (!isWriting)
            return nullptr;
        perror("Cannot create/write the file");
        exit(EXIT_FAILURE);
    }

    if (/*!quickMode &&*/ isWriting)
    {
        if (lseek(fd, size-1, SEEK_SET) == -1)
        {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        if (write(fd, "", 1) == -1)
        {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
    }

    void* buffer = mmap(0, size, (isWriting) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);

    if (buffer == MAP_FAILED)
    {
        close(fd);
        perror("Error mapping file to buffer");
        exit(EXIT_FAILURE);
    }

    if (!quickMode && madvise(buffer, size, MADV_RANDOM) != 0 && mlock(buffer, size) != 0)
    {
        munmap(buffer, size);
        close(fd);
        perror("Error in madvise or mlock");
        exit(EXIT_FAILURE);
    }

    close(fd);
    SPDLOG_TRACE("loaded buffer for [{}] {}", quickMode, path);
    return buffer;
#endif // _WINDOWS
}

void PageUtil::ReleasePageBuffer(void *buffer, int size, bool quickMode)
{
#ifdef _WINDOWS
    UnmapViewOfFile(buffer);
#else
    //unlock and unmap
    if (!quickMode && munlock(buffer, size) != 0)
    {
        perror("ERROR in munlock");
        exit(EXIT_FAILURE);
    }

    if(munmap(buffer, size)!=0)
    {
        perror("ERROR in munmap");
        exit(EXIT_FAILURE);
    }
#endif // _WINDOWS
}

bool PageUtil::FileExists(const string& filename)
{
#ifdef _WINDOWS
    HANDLE dumpFileDescriptor = CreateFileA(filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (dumpFileDescriptor == INVALID_HANDLE_VALUE) return false;
	else return true;
#else
    int fd = open(filename.c_str(), O_RDONLY, (mode_t)0600);
    if (fd >= 0)
    {
        close(fd);
        return true;
    }
    return false;
#endif // _WINDOWS
}