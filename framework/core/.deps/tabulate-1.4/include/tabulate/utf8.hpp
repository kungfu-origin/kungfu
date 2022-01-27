
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
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>

#include <clocale>
#include <locale>

#include <cstdlib>
#include <tabulate/termcolor.hpp>
#include <wchar.h>

namespace tabulate {

#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
inline int get_wcswidth(const std::string &string, const std::string &locale,
                        size_t max_column_width) {
  if (string.size() == 0)
    return 0;

  // The behavior of wcswidth() depends on the LC_CTYPE category of the current locale.
  // Set the current locale based on cell properties before computing width
  auto old_locale = std::locale::global(std::locale(locale));

  // Convert from narrow std::string to wide string
  wchar_t *wide_string = new wchar_t[string.size()];
  std::mbstowcs(wide_string, string.c_str(), string.size());

  // Compute display width of wide string
  int result = wcswidth(wide_string, max_column_width);
  delete[] wide_string;

  // Restore old locale
  std::locale::global(old_locale);

  return result;
}
#endif

inline size_t get_sequence_length(const std::string &text, const std::string &locale,
                                  bool is_multi_byte_character_support_enabled) {
  if (!is_multi_byte_character_support_enabled)
    return text.length();

#if defined(_WIN32) || defined(_WIN64)
  return (text.length() - std::count_if(text.begin(), text.end(),
                                        [](char c) -> bool { return (c & 0xC0) == 0x80; }));
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
  auto result = get_wcswidth(text, locale, text.size());
  if (result >= 0)
    return result;
  else
    return (text.length() - std::count_if(text.begin(), text.end(),
                                          [](char c) -> bool { return (c & 0xC0) == 0x80; }));
#endif
}

} // namespace tabulate
