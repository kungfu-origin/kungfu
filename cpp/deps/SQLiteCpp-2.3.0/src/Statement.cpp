/**
 * @file    Statement.cpp
 * @ingroup SQLiteCpp
 * @brief   A prepared SQLite Statement is a compiled SQL query ready to be executed, pointing to a row of result.
 *
 * Copyright (c) 2012-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#include <SQLiteCpp/Statement.h>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Column.h>
#include <SQLiteCpp/Assertion.h>
#include <SQLiteCpp/Exception.h>

#include <sqlite3.h>

namespace SQLite
{

// Compile and register the SQL query for the provided SQLite Database Connection
Statement::Statement(Database &aDatabase, const char* apQuery) :
    mQuery(apQuery),
    mStmtPtr(aDatabase.mpSQLite, mQuery), // prepare the SQL query, and ref count (needs Database friendship)
    mColumnCount(0),
    mbHasRow(false),
    mbDone(false)
{
    mColumnCount = sqlite3_column_count(mStmtPtr);
}

// Compile and register the SQL query for the provided SQLite Database Connection
Statement::Statement(Database &aDatabase, const std::string& aQuery) :
    mQuery(aQuery),
    mStmtPtr(aDatabase.mpSQLite, mQuery), // prepare the SQL query, and ref count (needs Database friendship)
    mColumnCount(0),
    mbHasRow(false),
    mbDone(false)
{
    mColumnCount = sqlite3_column_count(mStmtPtr);
}


// Finalize and unregister the SQL query from the SQLite Database Connection.
Statement::~Statement()
{
    // the finalization will be done by the destructor of the last shared pointer
}

// Reset the statement to make it ready for a new execution (see also #clearBindings() bellow)
void Statement::reset()
{
    const int ret = tryReset();
    check(ret);
}

int Statement::tryReset() noexcept
{
    mbHasRow = false;
    mbDone = false;
    return sqlite3_reset(mStmtPtr);
}

// Clears away all the bindings of a prepared statement (can be associated with #reset() above).
void Statement::clearBindings()
{
    const int ret = sqlite3_clear_bindings(mStmtPtr);
    check(ret);
}

// Bind an int value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const int aValue)
{
    const int ret = sqlite3_bind_int(mStmtPtr, aIndex, aValue);
    check(ret);
}

// Bind a 32bits unsigned int value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const unsigned aValue)
{
    const int ret = sqlite3_bind_int64(mStmtPtr, aIndex, aValue);
    check(ret);
}

// Bind a 64bits int value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const long long aValue)
{
    const int ret = sqlite3_bind_int64(mStmtPtr, aIndex, aValue);
    check(ret);
}

// Bind a double (64bits float) value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const double aValue)
{
    const int ret = sqlite3_bind_double(mStmtPtr, aIndex, aValue);
    check(ret);
}

// Bind a string value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const std::string& aValue)
{
    const int ret = sqlite3_bind_text(mStmtPtr, aIndex, aValue.c_str(),
                                      static_cast<int>(aValue.size()), SQLITE_TRANSIENT);
    check(ret);
}

// Bind a text value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const char* apValue)
{
    const int ret = sqlite3_bind_text(mStmtPtr, aIndex, apValue, -1, SQLITE_TRANSIENT);
    check(ret);
}

// Bind a binary blob value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex, const void* apValue, const int aSize)
{
    const int ret = sqlite3_bind_blob(mStmtPtr, aIndex, apValue, aSize, SQLITE_TRANSIENT);
    check(ret);
}

// Bind a string value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bindNoCopy(const int aIndex, const std::string& aValue)
{
    const int ret = sqlite3_bind_text(mStmtPtr, aIndex, aValue.c_str(),
                                      static_cast<int>(aValue.size()), SQLITE_STATIC);
    check(ret);
}

// Bind a text value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bindNoCopy(const int aIndex, const char* apValue)
{
    const int ret = sqlite3_bind_text(mStmtPtr, aIndex, apValue, -1, SQLITE_STATIC);
    check(ret);
}

// Bind a binary blob value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bindNoCopy(const int aIndex, const void* apValue, const int aSize)
{
    const int ret = sqlite3_bind_blob(mStmtPtr, aIndex, apValue, aSize, SQLITE_STATIC);
    check(ret);
}

// Bind a NULL value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const int aIndex)
{
    const int ret = sqlite3_bind_null(mStmtPtr, aIndex);
    check(ret);
}


// Bind an int value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const int aValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_int(mStmtPtr, index, aValue);
    check(ret);
}

// Bind a 32bits unsigned int value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const unsigned aValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_int64(mStmtPtr, index, aValue);
    check(ret);
}

// Bind a 64bits int value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const long long aValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_int64(mStmtPtr, index, aValue);
    check(ret);
}

// Bind a double (64bits float) value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const double aValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_double(mStmtPtr, index, aValue);
    check(ret);
}

// Bind a string value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const std::string& aValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_text(mStmtPtr, index, aValue.c_str(),
                                      static_cast<int>(aValue.size()), SQLITE_TRANSIENT);
    check(ret);
}

// Bind a text value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const char* apValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_text(mStmtPtr, index, apValue, -1, SQLITE_TRANSIENT);
    check(ret);
}

// Bind a binary blob value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName, const void* apValue, const int aSize)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_blob(mStmtPtr, index, apValue, aSize, SQLITE_TRANSIENT);
    check(ret);
}

// Bind a string value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bindNoCopy(const char* apName, const std::string& aValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_text(mStmtPtr, index, aValue.c_str(),
                                      static_cast<int>(aValue.size()), SQLITE_STATIC);
    check(ret);
}

// Bind a text value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bindNoCopy(const char* apName, const char* apValue)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_text(mStmtPtr, index, apValue, -1, SQLITE_STATIC);
    check(ret);
}

// Bind a binary blob value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bindNoCopy(const char* apName, const void* apValue, const int aSize)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_blob(mStmtPtr, index, apValue, aSize, SQLITE_STATIC);
    check(ret);
}

// Bind a NULL value to a parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement
void Statement::bind(const char* apName)
{
    const int index = sqlite3_bind_parameter_index(mStmtPtr, apName);
    const int ret = sqlite3_bind_null(mStmtPtr, index);
    check(ret);
}


// Execute a step of the query to fetch one row of results
bool Statement::executeStep()
{
    const int ret = tryExecuteStep();
    if ((SQLITE_ROW != ret) && (SQLITE_DONE != ret)) // on row or no (more) row ready, else it's a problem
    {
        throw SQLite::Exception(mStmtPtr, ret);
    }

    return mbHasRow; // true only if one row is accessible by getColumn(N)
}

// Execute a one-step query with no expected result
int Statement::exec()
{
    const int ret = tryExecuteStep();
    if (SQLITE_DONE != ret) // the statement has finished executing successfully
    {
        if (SQLITE_ROW == ret)
        {
            throw SQLite::Exception("exec() does not expect results. Use executeStep.");
        }
        else
        {
            throw SQLite::Exception(mStmtPtr, ret);
        }
    }

    // Return the number of rows modified by those SQL statements (INSERT, UPDATE or DELETE)
    return sqlite3_changes(mStmtPtr);
}

int Statement::tryExecuteStep() noexcept
{
    if (false == mbDone)
    {
        const int ret = sqlite3_step(mStmtPtr);
        if (SQLITE_ROW == ret) // one row is ready : call getColumn(N) to access it
        {
            mbHasRow = true;
        }
        else if (SQLITE_DONE == ret) // no (more) row ready : the query has finished executing
        {
            mbHasRow = false;
            mbDone = true;
        }
        else
        {
            mbHasRow = false;
            mbDone = false;
        }

        return ret;
    }
    else
    {
        // Statement needs to be reseted !
        return SQLITE_MISUSE;
    }
}


// Return a copy of the column data specified by its index starting at 0
// (use the Column copy-constructor)
Column Statement::getColumn(const int aIndex)
{
    checkRow();
    checkIndex(aIndex);

    // Share the Statement Object handle with the new Column created
    return Column(mStmtPtr, aIndex);
}

// Return a copy of the column data specified by its column name starting at 0
// (use the Column copy-constructor)
Column  Statement::getColumn(const char* apName)
{
    checkRow();
    const int index = getColumnIndex(apName);

    // Share the Statement Object handle with the new Column created
    return Column(mStmtPtr, index);
}

// Test if the column is NULL
bool Statement::isColumnNull(const int aIndex) const
{
    checkRow();
    checkIndex(aIndex);
    return (SQLITE_NULL == sqlite3_column_type(mStmtPtr, aIndex));
}

bool Statement::isColumnNull(const char* apName) const
{
    checkRow();
    const int index = getColumnIndex(apName);
    return (SQLITE_NULL == sqlite3_column_type(mStmtPtr, index));
}

// Return the named assigned to the specified result column (potentially aliased)
const char* Statement::getColumnName(const int aIndex) const
{
    checkIndex(aIndex);
    return sqlite3_column_name(mStmtPtr, aIndex);
}

#ifdef SQLITE_ENABLE_COLUMN_METADATA
// Return the named assigned to the specified result column (potentially aliased)
const char* Statement::getColumnOriginName(const int aIndex) const
{
    checkIndex(aIndex);
    return sqlite3_column_origin_name(mStmtPtr, aIndex);
}
#endif

// Return the index of the specified (potentially aliased) column name
int Statement::getColumnIndex(const char* apName) const
{
    // Build the map of column index by name on first call
    if (mColumnNames.empty())
    {
        for (int i = 0; i < mColumnCount; ++i)
        {
            const char* pName = sqlite3_column_name(mStmtPtr, i);
            mColumnNames[pName] = i;
        }
    }

    const TColumnNames::const_iterator iIndex = mColumnNames.find(apName);
    if (iIndex == mColumnNames.end())
    {
        throw SQLite::Exception("Unknown column name.");
    }

    return (*iIndex).second;
}

// Return the numeric result code for the most recent failed API call (if any).
int Statement::getErrorCode() const noexcept // nothrow
{
    return sqlite3_errcode(mStmtPtr);
}
// Return the extended numeric result code for the most recent failed API call (if any).
int Statement::getExtendedErrorCode() const noexcept // nothrow
{
    return sqlite3_extended_errcode(mStmtPtr);
}
// Return UTF-8 encoded English language explanation of the most recent failed API call (if any).
const char* Statement::getErrorMsg() const noexcept // nothrow
{
    return sqlite3_errmsg(mStmtPtr);
}

////////////////////////////////////////////////////////////////////////////////
// Internal class : shared pointer to the sqlite3_stmt SQLite Statement Object
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Prepare the statement and initialize its reference counter
 *
 * @param[in] apSQLite  The sqlite3 database connexion
 * @param[in] aQuery    The SQL query string to prepare
 */
Statement::Ptr::Ptr(sqlite3* apSQLite, std::string& aQuery) :
    mpSQLite(apSQLite),
    mpStmt(NULL),
    mpRefCount(NULL)
{
    const int ret = sqlite3_prepare_v2(apSQLite, aQuery.c_str(), static_cast<int>(aQuery.size()), &mpStmt, NULL);
    if (SQLITE_OK != ret)
    {
        throw SQLite::Exception(apSQLite, ret);
    }
    // Initialize the reference counter of the sqlite3_stmt :
    // used to share the mStmtPtr between Statement and Column objects;
    // This is needed to enable Column objects to live longer than the Statement objet it refers to.
    mpRefCount = new unsigned int(1);  // NOLINT(readability/casting)
}

/**
 * @brief Copy constructor increments the ref counter
 *
 * @param[in] aPtr Pointer to copy
 */
Statement::Ptr::Ptr(const Statement::Ptr& aPtr) :
    mpSQLite(aPtr.mpSQLite),
    mpStmt(aPtr.mpStmt),
    mpRefCount(aPtr.mpRefCount)
{
    assert(NULL != mpRefCount);
    assert(0 != *mpRefCount);

    // Increment the reference counter of the sqlite3_stmt,
    // asking not to finalize the sqlite3_stmt during the lifetime of the new objet
    ++(*mpRefCount);
}

/**
 * @brief Decrement the ref counter and finalize the sqlite3_stmt when it reaches 0
 */
Statement::Ptr::~Ptr()
{
    assert(NULL != mpRefCount);
    assert(0 != *mpRefCount);

    // Decrement and check the reference counter of the sqlite3_stmt
    --(*mpRefCount);
    if (0 == *mpRefCount)
    {
        // If count reaches zero, finalize the sqlite3_stmt, as no Statement nor Column objet use it anymore.
        // No need to check the return code, as it is the same as the last statement evaluation.
        sqlite3_finalize(mpStmt);

        // and delete the reference counter
        delete mpRefCount;
        mpRefCount = NULL;
        mpStmt = NULL;
    }
    // else, the finalization will be done later, by the last object
}


}  // namespace SQLite
