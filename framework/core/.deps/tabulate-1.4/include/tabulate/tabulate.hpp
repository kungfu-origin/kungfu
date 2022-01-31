
/*
  __        ___.         .__          __
_/  |______ \_ |__  __ __|  | _____ _/  |_  ____
\   __\__  \ | __ \|  |  \  | \__  \\   __\/ __ \
 |  |  / __ \| \_\ \  |  /  |__/ __ \|  | \  ___/
 |__| (____  /___  /____/|____(____  /__|  \___  >
           \/    \/                \/          \/
Table Maker for Modern C++
https://github.com/p-ranav/tabulate

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2019 Pranav Srinivas Kumar <pranav.srinivas.kumar@gmail.com>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TABULATE_EXPORT_HPP
#define TABULATE_EXPORT_HPP

// #ifdef _WIN32
//     #ifdef TABULATE_STATIC_LIB
//         #define TABULATE_API
//     #else
//         #ifdef TABULATE_EXPORTS
//             #define TABULATE_API __declspec(dllexport)
//         #else
//             #define TABULATE_API __declspec(dllimport)
//         #endif
//     #endif
// #else
//     #define TABULATE_API
// #endif

// Project version
#define TABULATE_VERSION_MAJOR 1
#define TABULATE_VERSION_MINOR 4
#define TABULATE_VERSION_PATCH 0

// Composing the protocol version string from major, and minor
#define TABULATE_CONCATENATE(A, B) TABULATE_CONCATENATE_IMPL(A, B)
#define TABULATE_CONCATENATE_IMPL(A, B) A##B
#define TABULATE_STRINGIFY(a) TABULATE_STRINGIFY_IMPL(a)
#define TABULATE_STRINGIFY_IMPL(a) #a

#endif
