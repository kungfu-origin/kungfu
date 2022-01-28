
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
#include <tabulate/exporter.hpp>

#if __cplusplus >= 201703L
#include <optional>
using std::optional;
#else
#include <tabulate/optional_lite.hpp>
using nonstd::optional;
#endif

namespace tabulate {

class LatexExporter : public Exporter {

  static const char new_line = '\n';

public:
  class ExportOptions {
  public:
    ExportOptions &indentation(std::size_t value) {
      indentation_ = value;
      return *this;
    }

  private:
    friend class LatexExporter;
    optional<size_t> indentation_;
  };

  ExportOptions &configure() { return options_; }

  std::string dump(Table &table) override {
    std::string result{"\\begin{tabular}"};
    result += new_line;

    result += add_alignment_header(table);
    result += new_line;
    const auto rows = table.rows_;
    // iterate content and put text into the table.
    for (size_t i = 0; i < rows; i++) {
      auto &row = table[i];
      // apply row content indentation
      if (options_.indentation_.has_value()) {
        result += std::string(options_.indentation_.value(), ' ');
      }

      for (size_t j = 0; j < row.size(); j++) {

        result += row[j].get_text();

        // check column position, need "\\" at the end of each row
        if (j < row.size() - 1) {
          result += " & ";
        } else {
          result += " \\\\";
        }
      }
      result += new_line;
    }

    result += "\\end{tabular}";
    return result;
  }

private:
  std::string add_alignment_header(Table &table) {
    std::string result{"{"};

    for (auto &cell : table[0]) {
      auto format = cell.format();
      if (format.font_align_.value() == FontAlign::left) {
        result += 'l';
      } else if (format.font_align_.value() == FontAlign::center) {
        result += 'c';
      } else if (format.font_align_.value() == FontAlign::right) {
        result += 'r';
      }
    }

    result += "}";
    return result;
  }
  ExportOptions options_;
};

} // namespace tabulate
