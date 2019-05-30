/**
 * @file    SQLiteCpp.h
 * @ingroup SQLiteCpp
 * @brief   SQLiteC++ is a smart and simple C++ SQLite3 wrapper. This file is only "easy include" for other files.
 *
 * Include this main header file in your project to gain access to all functionality provided by the wrapper.
 *
 * Copyright (c) 2012-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
/**
 * @defgroup SQLiteCpp SQLiteC++
 * @brief    SQLiteC++ is a smart and simple C++ SQLite3 wrapper. This file is only "easy include" for other files.
 */
#pragma once


// Include useful headers of SQLiteC++
#include <SQLiteCpp/Assertion.h>
#include <SQLiteCpp/Exception.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Column.h>
#include <SQLiteCpp/Transaction.h>


/**
 * @brief Version numbers for SQLiteC++ are provided in the same way as sqlite3.h
 *
 * The [SQLITECPP_VERSION] C preprocessor macro in the SQLiteC++.h header
 * evaluates to a string literal that is the SQLite version in the
 * format "X.Y.Z" where X is the major version number
 * and Y is the minor version number and Z is the release number.
 *
 * The [SQLITECPP_VERSION_NUMBER] C preprocessor macro resolves to an integer
 * with the value (X*1000000 + Y*1000 + Z) where X, Y, and Z are the same
 * numbers used in [SQLITECPP_VERSION].
 */
#define SQLITECPP_VERSION           "2.03.00"   // 2.3.0
#define SQLITECPP_VERSION_NUMBER     2003000    // 2.3.0
