Mar 30 2012
- Start of a new thin C++ SQLite wrapper

Apr 2 2012
- The wrapper is functionnal
- Added documentation and examples
- Publication on GitHub

Version 0.1.0 - Apr 4 2012
- Added a Database::exec() methode to execute simple SQL statement
- Added a version number like in sqlite3.h, starting with 0.1.0

Version 0.2.0 - Apr 11 2012
- Added getLastInsertId() and setBusyTimout()
- Added bind() by name methods

Version 0.3.0 - Apr 16 2012
- Added an easy wrapper Database::execAngGet()

Version 0.4.0 - Apr 23 2012
- Added a Database::tableExists() easy to use function

Dec 10 2012
- Added a Statement::exec() method to execute a one-step query with no expected result

Version 0.5.0 - March 9 2013
- Added assert() on errors on destructors
- Added getBytes()
- Added getBlob(), getType() and isInteger/isFloat/isText/isBlob/isNull
- Added bind() for binary blob data

Version 0.5.1 - April 7 2013
- Added Column::getName()

Version 0.6.0 - November 22 2013
- Renamed Column::getName() to Column::getOriginName()
- Added Column::getName()

Version 0.7.0 - January 9 2014
- Added Database::createFunction()
- Added std::string version of existing APIs
- Improved CMake with more build options and Doxygen auto-detection

Version 0.8.0 - February 26 2014
- Database constructor support opening a database with a custom VFS (default to NULL)
- Changed Column::getText() to return empty string "" by default instead of NULL pointer (to handle std::string conversion)

Version 1.0.0 - May 3 2015
- Public headers file moved to include/ dir
- Added support to biicode in CMakeLists.txt
- Added Unit Tests
- Added aBusyTimeoutMs parameter to Database() constructors
- Added Database::getTotalChanges()
- Added Database::getErrorCode()
- Added Statement::clearBindings()
- Added Statement::getColumn(aName)
- Added Statement::getErrorCode()
- Added Statement::getColumnName(aIndex)
- Added Statement::getColumnOriginName(aIndex)

Version 1.1.0 - May 18 2015
- Fixed valgrind error on Database destructor
- Added Database::loadExtension

Version 1.2.0 - September 9 2015
- Fixed build with GCC 5.1.0
- Fixed MSVC release build warning
- Fixed CppDepends warnings
- Updated documentation on installation
- Added Database::getHandle()

Version 1.3.0 - November 1 2015
- Fixed build with Visual Studio 2015
- Further improvements to README
- Added Backup class

Version 1.3.1 - February 10 2016
- Swith Linux/Mac build to the provided SQLite3 C library
- Update SQLite3 from 3.8.8.3 to latest 3.10.2 (2016-01-20)
- Remove warnings
- Remove biicode support (defunct service, servers will shutdown the 16th of February 2016)

Version 2.0.0 - July 25 2016
- Update SQLite3 from 3.10.2 to latest 3.13 (2016-05-18)
- Move #include <sqlite3.h> from headers to .cpp files only using forward declarations
- Add Database::VERSION to reach SQLITE_VERSION without including sqlite3.h in application code
- Add getLibVersion() and getLibVersionNumber() to get runtime version of the library
- Better exception messages when Statements fail PR #84
- Variadic templates for bind() (C++14) PR #85
- Add Statement::bindNoCopy() methods for strings, using SQLITE_STATIC to avoid internal copy by SQLite3 PR #86
- Add Statement::bind() overload for uint32_t, and Column::getUint() and cast operator to uint32_t PR #86
- Use the new SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION from SQLite 3.13 for security reason
- Rename Backup::remainingPageCount()/totalPageCount() to Backup::getRemainingPageCount()/getTotalPageCount()
- Remove Column::errmsg() method : use Database or Statement equivalents
- More unit tests, with code coverage status on the GitHub page
- Do not force MSVC to use static runtime if unit-tests are not build

Version 2.1.0 - July 18 2017
- Update SQLite3 from 3.13 to latest 3.19.3 (2017-06-08)
- Fixed Incompatibility in 3.19.0 (to use older SQLite version set the CMake variable SQLITE_USE_LEGACY_STRUCT) #125
- Fixed link error (inline in cpp) and compiler warnings (unused variable...) #96
- Added ability to open encrypted databases #107
- Added convenience functions for constructing objects from a row #114
- Added CMake install step #118
- Fix warnings #119
- Make cpplint.py Python-3 compatible #120
- Link libssp when targeted #100
- Removed redundant const #102

Version 2.2.0 - Sept 19 2017
- Update SQLite3 from 3.19.3 to latest 3.20.1 (2017-08-24) #143
- Added tryExecuteStep and tryReset #142
- Removed virtual kewords from destructors #140
- Removed misplaced noexcept keyword #139
- Improved Exception class C++ conformance #138
- Fix warnings #134
- Deprecated Statement::IsOk() to Statement::HasRow()

Version 2.3.0 - March 3 2019
- Update SQLite3 from 3.20.1 to latest 3.27.2 (2019-02-25) #183 #187
- Add Statement binding for long int values #147
- Allows long int for bind when used with name #148
- More cmake instructions for linux #151
- Add comparison with sqlite_orm #141
- Fix Statement::bind truncates long integer to 32 bits on x86_64 Linux #155
- Add a move constructor to Database #157
- Added tests for all MSVC compilers available on AppVeyor (2013, 2015, 2017) #169
- Update VariadicBind.h #172
- Better CMake compatibility #170
- Add implicit cast operator to char and short types