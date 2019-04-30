/**
 * @file    Column.h
 * @ingroup SQLiteCpp
 * @brief   Encapsulation of a Column in a row of the result pointed by the prepared SQLite::Statement.
 *
 * Copyright (c) 2012-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#pragma once

#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Exception.h>

#include <string>
#include <climits> // For INT_MAX


namespace SQLite
{

extern const int INTEGER;   ///< SQLITE_INTEGER
extern const int FLOAT;     ///< SQLITE_FLOAT
extern const int TEXT;      ///< SQLITE_TEXT
extern const int BLOB;      ///< SQLITE_BLOB
extern const int Null;      ///< SQLITE_NULL


/**
 * @brief Encapsulation of a Column in a row of the result pointed by the prepared Statement.
 *
 *  A Column is a particular field of SQLite data in the current row of result
 * of the Statement : it points to a single cell.
 *
 * Its value can be expressed as a text, and, when applicable, as a numeric
 * (integer or floating point) or a binary blob.
 *
 * Thread-safety: a Column object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Column
{
public:
    /**
     * @brief Encapsulation of a Column in a Row of the result.
     *
     * @param[in] aStmtPtr  Shared pointer to the prepared SQLite Statement Object.
     * @param[in] aIndex    Index of the column in the row of result, starting at 0
     */
    Column(Statement::Ptr& aStmtPtr, int aIndex)    noexcept; // nothrow
    /// Simple destructor
    ~Column();

    // default copy constructor and assignment operator are perfectly suited :
    // they copy the Statement::Ptr which in turn increments the reference counter.

    /// Make clang happy by explicitly implementing the copy-constructor:
    Column(const Column & aOther) :
        mStmtPtr(aOther.mStmtPtr),
        mIndex(aOther.mIndex)
    {
    }

    /**
     * @brief Return a pointer to the named assigned to this result column (potentially aliased)
     *
     * @see getOriginName() to get original column name (not aliased)
     */
    const char* getName() const noexcept; // nothrow

#ifdef SQLITE_ENABLE_COLUMN_METADATA
    /**
     * @brief Return a pointer to the table column name that is the origin of this result column
     *
     *  Require definition of the SQLITE_ENABLE_COLUMN_METADATA preprocessor macro :
     * - when building the SQLite library itself (which is the case for the Debian libsqlite3 binary for instance),
     * - and also when compiling this wrapper.
     */
    const char* getOriginName() const noexcept; // nothrow
#endif

    /// Return the integer value of the column.
    int         getInt() const noexcept; // nothrow
    /// Return the 32bits unsigned integer value of the column (note that SQLite3 does not support unsigned 64bits).
    unsigned    getUInt() const noexcept; // nothrow
    /// Return the 64bits integer value of the column (note that SQLite3 does not support unsigned 64bits).
    long long   getInt64() const noexcept; // nothrow
    /// Return the double (64bits float) value of the column
    double      getDouble() const noexcept; // nothrow
    /**
     * @brief Return a pointer to the text value (NULL terminated string) of the column.
     *
     * @warning The value pointed at is only valid while the statement is valid (ie. not finalized),
     *          thus you must copy it before using it beyond its scope (to a std::string for instance).
     */
    const char* getText(const char* apDefaultValue = "") const noexcept; // nothrow
    /**
     * @brief Return a pointer to the binary blob value of the column.
     *
     * @warning The value pointed at is only valid while the statement is valid (ie. not finalized),
     *          thus you must copy it before using it beyond its scope (to a std::string for instance).
     */
    const void* getBlob() const noexcept; // nothrow
    /**
     * @brief Return a std::string for a TEXT or BLOB column.
     *
     * Note this correctly handles strings that contain null bytes.
     */
    std::string getString() const;

    /**
     * @brief Return the type of the value of the column
     *
     * Return either SQLite::INTEGER, SQLite::FLOAT, SQLite::TEXT, SQLite::BLOB, or SQLite::Null.
     *
     * @warning After a type conversion (by a call to a getXxx on a Column of a Yyy type),
     *          the value returned by sqlite3_column_type() is undefined.
     */
    int getType() const noexcept; // nothrow

    /// Test if the column is an integer type value (meaningful only before any conversion)
    inline bool isInteger() const noexcept // nothrow
    {
        return (SQLite::INTEGER == getType());
    }
    /// Test if the column is a floating point type value (meaningful only before any conversion)
    inline bool isFloat() const noexcept // nothrow
    {
        return (SQLite::FLOAT == getType());
    }
    /// Test if the column is a text type value (meaningful only before any conversion)
    inline bool isText() const noexcept // nothrow
    {
        return (SQLite::TEXT == getType());
    }
    /// Test if the column is a binary blob type value (meaningful only before any conversion)
    inline bool isBlob() const noexcept // nothrow
    {
        return (SQLite::BLOB == getType());
    }
    /// Test if the column is NULL (meaningful only before any conversion)
    inline bool isNull() const noexcept // nothrow
    {
        return (SQLite::Null == getType());
    }

    /**
     * @brief Return the number of bytes used by the text (or blob) value of the column
     *
     * Return either :
     * - size in bytes (not in characters) of the string returned by getText() without the '\0' terminator
     * - size in bytes of the string representation of the numerical value (integer or double)
     * - size in bytes of the binary blob returned by getBlob()
     * - 0 for a NULL value
     */
    int getBytes() const noexcept;

    /// Alias returning the number of bytes used by the text (or blob) value of the column
    inline int size() const noexcept
    {
        return getBytes ();
    }

    /// Inline cast operator to char
    inline operator char() const
    {
        return static_cast<char>(getInt());
    }
    /// Inline cast operator to unsigned char
    inline operator unsigned char() const
    {
        return static_cast<unsigned char>(getInt());
    }
    /// Inline cast operator to short
    inline operator short() const
    {
        return static_cast<short>(getInt());
    }
    /// Inline cast operator to unsigned short
    inline operator unsigned short() const
    {
        return static_cast<unsigned short>(getInt());
    }

    /// Inline cast operator to int
    inline operator int() const
    {
        return getInt();
    }
    /// Inline cast operator to 32bits unsigned integer
    inline operator unsigned int() const
    {
        return getUInt();
    }
#if (LONG_MAX == INT_MAX) // 4 bytes "long" type means the data model is ILP32 or LLP64 (Win64 Visual C++ and MinGW)
    /// Inline cast operator to 32bits long
    inline operator long() const
    {
        return getInt();
    }
    /// Inline cast operator to 32bits unsigned long
    inline operator unsigned long() const
    {
        return getUInt();
    }
#else // 8 bytes "long" type means the data model is LP64 (Most Unix-like, Windows when using Cygwin; z/OS)
    /// Inline cast operator to 64bits long when the data model of the system is LP64 (Linux 64 bits...)
    inline operator long() const
    {
        return getInt64();
    }
#endif

    /// Inline cast operator to 64bits integer
    inline operator long long() const
    {
        return getInt64();
    }
    /// Inline cast operator to double
    inline operator double() const
    {
        return getDouble();
    }
    /**
     * @brief Inline cast operator to char*
     *
     * @see getText
     */
    inline operator const char*() const
    {
        return getText();
    }
    /**
     * @brief Inline cast operator to void*
     *
     * @see getBlob
     */
    inline operator const void*() const
    {
        return getBlob();
    }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
    // NOTE : the following is required by GCC and Clang to cast a Column result in a std::string
    // (error: conversion from ‘SQLite::Column’ to non-scalar type ‘std::string {aka std::basic_string<char>}’)
    // and also required for Microsoft Visual Studio 2015 and newer
    // but is not working under Microsoft Visual Studio 2010, 2012 and 2013
    // (error C2440: 'initializing' : cannot convert from 'SQLite::Column' to 'std::basic_string<_Elem,_Traits,_Ax>'
    //  [...] constructor overload resolution was ambiguous)
    // WARNING: without it, trying to access a binary blob with implicit cast to string
    // ends up converting it to a C-style char*, damaging the data by truncating it to the first null character!
    // (see https://github.com/SRombauts/SQLiteCpp/issues/189 Visual Studio 2013: unit test "Column.basis" failing)
    /**
     * @brief Inline cast operator to std::string
     *
     * Handles BLOB or TEXT, which may contain null bytes within
     *
     * @see getString
     */
    inline operator std::string() const
    {
        return getString();
    }
#endif

private:
    Statement::Ptr  mStmtPtr;   ///< Shared Pointer to the prepared SQLite Statement Object
    int             mIndex;     ///< Index of the column in the row of result, starting at 0
};

/**
 * @brief Standard std::ostream text inserter
 *
 * Insert the text value of the Column object, using getText(), into the provided stream.
 *
 * @param[in] aStream   Stream to use
 * @param[in] aColumn   Column object to insert into the provided stream
 *
 * @return  Reference to the stream used
 */
std::ostream& operator<<(std::ostream& aStream, const Column& aColumn);

#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1900)

// Create an instance of T from the first N columns, see declaration in Statement.h for full details
template<typename T, int N>
T Statement::getColumns()
{
    checkRow();
    checkIndex(N - 1);
    return getColumns<T>(std::make_integer_sequence<int, N>{});
}

// Helper function called by getColums<typename T, int N>
template<typename T, const int... Is>
T Statement::getColumns(const std::integer_sequence<int, Is...>)
{
    return T{Column(mStmtPtr, Is)...};
}

#endif

}  // namespace SQLite
