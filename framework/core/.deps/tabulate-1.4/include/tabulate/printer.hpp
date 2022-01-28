
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
#include <tabulate/color.hpp>
#include <tabulate/font_style.hpp>
#include <utility>
#include <vector>

namespace tabulate {

class Printer {
public:
  static std::pair<std::vector<size_t>, std::vector<size_t>>
  compute_cell_dimensions(TableInternal &table);

  static void print_table(std::ostream &stream, TableInternal &table);

  static void print_row_in_cell(std::ostream &stream, TableInternal &table,
                                const std::pair<size_t, size_t> &index,
                                const std::pair<size_t, size_t> &dimension, size_t num_columns,
                                size_t row_index);

  static bool print_cell_border_top(std::ostream &stream, TableInternal &table,
                                    const std::pair<size_t, size_t> &index,
                                    const std::pair<size_t, size_t> &dimension, size_t num_columns);
  static bool print_cell_border_bottom(std::ostream &stream, TableInternal &table,
                                       const std::pair<size_t, size_t> &index,
                                       const std::pair<size_t, size_t> &dimension,
                                       size_t num_columns);

  static void apply_element_style(std::ostream &stream, Color foreground_color,
                                  Color background_color,
                                  const std::vector<FontStyle> &font_style) {
    apply_foreground_color(stream, foreground_color);
    apply_background_color(stream, background_color);
    for (auto &style : font_style)
      apply_font_style(stream, style);
  }

  static void reset_element_style(std::ostream &stream) { stream << termcolor::reset; }

private:
  static void print_content_left_aligned(std::ostream &stream, const std::string &cell_content,
                                         const Format &format, size_t text_with_padding_size,
                                         size_t column_width) {

    // Apply font style
    apply_element_style(stream, *format.font_color_, *format.font_background_color_,
                        *format.font_style_);
    stream << cell_content;
    // Only apply font_style to the font
    // Not the padding. So calling apply_element_style with font_style = {}
    reset_element_style(stream);
    apply_element_style(stream, *format.font_color_, *format.font_background_color_, {});

    if (text_with_padding_size < column_width) {
      for (size_t j = 0; j < (column_width - text_with_padding_size); ++j) {
        stream << " ";
      }
    }
  }

  static void print_content_center_aligned(std::ostream &stream, const std::string &cell_content,
                                           const Format &format, size_t text_with_padding_size,
                                           size_t column_width) {
    auto num_spaces = column_width - text_with_padding_size;
    if (num_spaces % 2 == 0) {
      // Even spacing on either side
      for (size_t j = 0; j < num_spaces / 2; ++j)
        stream << " ";

      // Apply font style
      apply_element_style(stream, *format.font_color_, *format.font_background_color_,
                          *format.font_style_);
      stream << cell_content;
      // Only apply font_style to the font
      // Not the padding. So calling apply_element_style with font_style = {}
      reset_element_style(stream);
      apply_element_style(stream, *format.font_color_, *format.font_background_color_, {});

      for (size_t j = 0; j < num_spaces / 2; ++j)
        stream << " ";
    } else {
      auto num_spaces_before = num_spaces / 2 + 1;
      for (size_t j = 0; j < num_spaces_before; ++j)
        stream << " ";

      // Apply font style
      apply_element_style(stream, *format.font_color_, *format.font_background_color_,
                          *format.font_style_);
      stream << cell_content;
      // Only apply font_style to the font
      // Not the padding. So calling apply_element_style with font_style = {}
      reset_element_style(stream);
      apply_element_style(stream, *format.font_color_, *format.font_background_color_, {});

      for (size_t j = 0; j < num_spaces - num_spaces_before; ++j)
        stream << " ";
    }
  }

  static void print_content_right_aligned(std::ostream &stream, const std::string &cell_content,
                                          const Format &format, size_t text_with_padding_size,
                                          size_t column_width) {
    if (text_with_padding_size < column_width) {
      for (size_t j = 0; j < (column_width - text_with_padding_size); ++j) {
        stream << " ";
      }
    }

    // Apply font style
    apply_element_style(stream, *format.font_color_, *format.font_background_color_,
                        *format.font_style_);
    stream << cell_content;
    // Only apply font_style to the font
    // Not the padding. So calling apply_element_style with font_style = {}
    reset_element_style(stream);
    apply_element_style(stream, *format.font_color_, *format.font_background_color_, {});
  }

  static void apply_font_style(std::ostream &stream, FontStyle style) {
    switch (style) {
    case FontStyle::bold:
      stream << termcolor::bold;
      break;
    case FontStyle::dark:
      stream << termcolor::dark;
      break;
    case FontStyle::italic:
      stream << termcolor::italic;
      break;
    case FontStyle::underline:
      stream << termcolor::underline;
      break;
    case FontStyle::blink:
      stream << termcolor::blink;
      break;
    case FontStyle::reverse:
      stream << termcolor::reverse;
      break;
    case FontStyle::concealed:
      stream << termcolor::concealed;
      break;
    case FontStyle::crossed:
      stream << termcolor::crossed;
      break;
    default:
      break;
    }
  }

  static void apply_foreground_color(std::ostream &stream, Color foreground_color) {
    switch (foreground_color) {
    case Color::grey:
      stream << termcolor::grey;
      break;
    case Color::red:
      stream << termcolor::red;
      break;
    case Color::green:
      stream << termcolor::green;
      break;
    case Color::yellow:
      stream << termcolor::yellow;
      break;
    case Color::blue:
      stream << termcolor::blue;
      break;
    case Color::magenta:
      stream << termcolor::magenta;
      break;
    case Color::cyan:
      stream << termcolor::cyan;
      break;
    case Color::white:
      stream << termcolor::white;
      break;
    case Color::none:
    default:
      break;
    }
  }

  static void apply_background_color(std::ostream &stream, Color background_color) {
    switch (background_color) {
    case Color::grey:
      stream << termcolor::on_grey;
      break;
    case Color::red:
      stream << termcolor::on_red;
      break;
    case Color::green:
      stream << termcolor::on_green;
      break;
    case Color::yellow:
      stream << termcolor::on_yellow;
      break;
    case Color::blue:
      stream << termcolor::on_blue;
      break;
    case Color::magenta:
      stream << termcolor::on_magenta;
      break;
    case Color::cyan:
      stream << termcolor::on_cyan;
      break;
    case Color::white:
      stream << termcolor::on_white;
      break;
    case Color::none:
    default:
      break;
    }
  }
};

} // namespace tabulate
