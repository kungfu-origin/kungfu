
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

namespace tabulate {

class ColumnFormat : public Format {
public:
  explicit ColumnFormat(class Column &column) : column_(column) {}

  ColumnFormat &width(size_t value);
  ColumnFormat &height(size_t value);

  // Padding
  ColumnFormat &padding(size_t value);
  ColumnFormat &padding_left(size_t value);
  ColumnFormat &padding_right(size_t value);
  ColumnFormat &padding_top(size_t value);
  ColumnFormat &padding_bottom(size_t value);

  // Border
  ColumnFormat &border(const std::string &value);
  ColumnFormat &border_color(Color value);
  ColumnFormat &border_background_color(Color value);
  ColumnFormat &border_left(const std::string &value);
  ColumnFormat &border_left_color(Color value);
  ColumnFormat &border_left_background_color(Color value);
  ColumnFormat &border_right(const std::string &value);
  ColumnFormat &border_right_color(Color value);
  ColumnFormat &border_right_background_color(Color value);
  ColumnFormat &border_top(const std::string &value);
  ColumnFormat &border_top_color(Color value);
  ColumnFormat &border_top_background_color(Color value);
  ColumnFormat &border_bottom(const std::string &value);
  ColumnFormat &border_bottom_color(Color value);
  ColumnFormat &border_bottom_background_color(Color value);

  // Corner
  ColumnFormat &corner(const std::string &value);
  ColumnFormat &corner_color(Color value);
  ColumnFormat &corner_background_color(Color value);

  // Column separator
  ColumnFormat &column_separator(const std::string &value);
  ColumnFormat &column_separator_color(Color value);
  ColumnFormat &column_separator_background_color(Color value);

  // Font styling
  ColumnFormat &font_align(FontAlign value);
  ColumnFormat &font_style(const std::vector<FontStyle> &style);
  ColumnFormat &font_color(Color value);
  ColumnFormat &font_background_color(Color value);
  ColumnFormat &color(Color value);
  ColumnFormat &background_color(Color value);

  // Locale
  ColumnFormat &multi_byte_characters(bool value);
  ColumnFormat &locale(const std::string &value);

private:
  std::reference_wrapper<class Column> column_;
};

} // namespace tabulate
