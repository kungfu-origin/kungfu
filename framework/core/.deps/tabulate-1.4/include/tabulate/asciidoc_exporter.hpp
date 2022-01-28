
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
#include <optional>
#include <sstream>
#include <string>
#include <tabulate/exporter.hpp>

namespace tabulate {

class AsciiDocExporter : public Exporter {

  static const char new_line = '\n';

public:
  std::string dump(Table &table) override {
    std::stringstream ss;
    ss << add_alignment_header(table);
    ss << new_line;

    const auto rows = table.rows_;
    // iterate content and put text into the table.
    for (size_t row_index = 0; row_index < rows; row_index++) {
      auto &row = table[row_index];

      for (size_t cell_index = 0; cell_index < row.size(); cell_index++) {
        ss << "|";
        ss << add_formatted_cell(row[cell_index]);
      }
      ss << new_line;
      if (row_index == 0) {
        ss << new_line;
      }
    }

    ss << "|===";
    return ss.str();
  }

private:
  std::string add_formatted_cell(Cell &cell) const {
    std::stringstream ss;
    auto format = cell.format();
    std::string cell_string = cell.get_text();

    auto font_style = format.font_style_.value();

    bool format_bold = false;
    bool format_italic = false;
    std::for_each(font_style.begin(), font_style.end(), [&](FontStyle &style) {
      if (style == FontStyle::bold) {
        format_bold = true;
      } else if (style == FontStyle::italic) {
        format_italic = true;
      }
    });

    if (format_bold) {
      ss << '*';
    }
    if (format_italic) {
      ss << '_';
    }

    ss << cell_string;
    if (format_italic) {
      ss << '_';
    }
    if (format_bold) {
      ss << '*';
    }
    return ss.str();
  }

  std::string add_alignment_header(Table &table) {
    std::stringstream ss;
    ss << (R"([cols=")");

    size_t column_count = table[0].size();
    size_t column_index = 0;
    for (auto &cell : table[0]) {
      auto format = cell.format();

      if (format.font_align_.value() == FontAlign::left) {
        ss << '<';
      } else if (format.font_align_.value() == FontAlign::center) {
        ss << '^';
      } else if (format.font_align_.value() == FontAlign::right) {
        ss << '>';
      }

      ++column_index;
      if (column_index != column_count) {
        ss << ",";
      }
    }

    ss << R"("])";
    ss << new_line;
    ss << "|===";

    return ss.str();
  }
};

} // namespace tabulate
