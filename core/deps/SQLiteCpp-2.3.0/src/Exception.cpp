/**
 * @file    Exception.cpp
 * @ingroup SQLiteCpp
 * @brief   Encapsulation of the error message from SQLite3 on a std::runtime_error.
 *
 * Copyright (c) 2012-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#include <SQLiteCpp/Exception.h>

#include <sqlite3.h>


namespace SQLite
{

Exception::Exception(const char* aErrorMessage) :
    std::runtime_error(aErrorMessage),
    mErrcode(-1), // 0 would be SQLITE_OK, which doesn't make sense
    mExtendedErrcode(-1)
{
}
Exception::Exception(const std::string& aErrorMessage) :
    std::runtime_error(aErrorMessage),
    mErrcode(-1), // 0 would be SQLITE_OK, which doesn't make sense
    mExtendedErrcode(-1)
{
}

Exception::Exception(const char* aErrorMessage, int ret) :
    std::runtime_error(aErrorMessage),
    mErrcode(ret),
    mExtendedErrcode(-1)
{
}

Exception::Exception(const std::string& aErrorMessage, int ret) :
    std::runtime_error(aErrorMessage),
    mErrcode(ret),
    mExtendedErrcode(-1)
{
}

Exception::Exception(sqlite3* apSQLite) :
    std::runtime_error(sqlite3_errmsg(apSQLite)),
    mErrcode(sqlite3_errcode(apSQLite)),
    mExtendedErrcode(sqlite3_extended_errcode(apSQLite))
{
}

Exception::Exception(sqlite3* apSQLite, int ret) :
    std::runtime_error(sqlite3_errmsg(apSQLite)),
    mErrcode(ret),
    mExtendedErrcode(sqlite3_extended_errcode(apSQLite))
{
}

// Return a string, solely based on the error code
const char* Exception::getErrorStr() const noexcept // nothrow
{
    return sqlite3_errstr(mErrcode);
}


}  // namespace SQLite
