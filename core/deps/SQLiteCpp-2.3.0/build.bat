@REM Copyright (c) 2012-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
@REM
@REM Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
@REM or copy at http://opensource.org/licenses/MIT)
mkdir build
cd build

@REM Generate a Visual Studio solution for latest version found
cmake -DSQLITECPP_BUILD_EXAMPLES=ON -DSQLITECPP_BUILD_TESTS=ON ..
if ERRORLEVEL 1 goto onError

@REM Build default configuration (ie 'Debug')
cmake --build .
if ERRORLEVEL 1 goto onError

@REM Build and run tests
ctest --output-on-failure
if ERRORLEVEL 1 goto onError

cd ..
exit

:onError
@echo An error occured!
cd ..