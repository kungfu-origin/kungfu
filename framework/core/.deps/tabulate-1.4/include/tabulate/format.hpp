
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
#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>
#include <tabulate/color.hpp>
#include <tabulate/font_align.hpp>
#include <tabulate/font_style.hpp>
#include <tabulate/utf8.hpp>

#if __cplusplus >= 201703L
#include <optional>
using std::optional;
#else
#include <tabulate/optional_lite.hpp>
using nonstd::optional;
#endif

#include <vector>

namespace tabulate {

class Format {
public:
  Format &width(size_t value) {
    width_ = value;
    return *this;
  }

  Format &height(size_t value) {
    height_ = value;
    return *this;
  }

  Format &padding(size_t value) {
    padding_left_ = value;
    padding_right_ = value;
    padding_top_ = value;
    padding_bottom_ = value;
    return *this;
  }

  Format &padding_left(size_t value) {
    padding_left_ = value;
    return *this;
  }

  Format &padding_right(size_t value) {
    padding_right_ = value;
    return *this;
  }

  Format &padding_top(size_t value) {
    padding_top_ = value;
    return *this;
  }

  Format &padding_bottom(size_t value) {
    padding_bottom_ = value;
    return *this;
  }

  Format &border(const std::string &value) {
    border_left_ = value;
    border_right_ = value;
    border_top_ = value;
    border_bottom_ = value;
    return *this;
  }

  Format &border_color(Color value) {
    border_left_color_ = value;
    border_right_color_ = value;
    border_top_color_ = value;
    border_bottom_color_ = value;
    return *this;
  }

  Format &border_background_color(Color value) {
    border_left_background_color_ = value;
    border_right_background_color_ = value;
    border_top_background_color_ = value;
    border_bottom_background_color_ = value;
    return *this;
  }

  Format &border_left(const std::string &value) {
    border_left_ = value;
    return *this;
  }

  Format &border_left_color(Color value) {
    border_left_color_ = value;
    return *this;
  }

  Format &border_left_background_color(Color value) {
    border_left_background_color_ = value;
    return *this;
  }

  Format &border_right(const std::string &value) {
    border_right_ = value;
    return *this;
  }

  Format &border_right_color(Color value) {
    border_right_color_ = value;
    return *this;
  }

  Format &border_right_background_color(Color value) {
    border_right_background_color_ = value;
    return *this;
  }

  Format &border_top(const std::string &value) {
    border_top_ = value;
    return *this;
  }

  Format &border_top_color(Color value) {
    border_top_color_ = value;
    return *this;
  }

  Format &border_top_background_color(Color value) {
    border_top_background_color_ = value;
    return *this;
  }

  Format &border_bottom(const std::string &value) {
    border_bottom_ = value;
    return *this;
  }

  Format &border_bottom_color(Color value) {
    border_bottom_color_ = value;
    return *this;
  }

  Format &border_bottom_background_color(Color value) {
    border_bottom_background_color_ = value;
    return *this;
  }

  Format &show_border() {
    show_border_top_ = true;
    show_border_bottom_ = true;
    show_border_left_ = true;
    show_border_right_ = true;
    return *this;
  }

  Format &hide_border() {
    show_border_top_ = false;
    show_border_bottom_ = false;
    show_border_left_ = false;
    show_border_right_ = false;
    return *this;
  }

  Format &show_border_top() {
    show_border_top_ = true;
    return *this;
  }

  Format &hide_border_top() {
    show_border_top_ = false;
    return *this;
  }

  Format &show_border_bottom() {
    show_border_bottom_ = true;
    return *this;
  }

  Format &hide_border_bottom() {
    show_border_bottom_ = false;
    return *this;
  }

  Format &show_border_left() {
    show_border_left_ = true;
    return *this;
  }

  Format &hide_border_left() {
    show_border_left_ = false;
    return *this;
  }

  Format &show_border_right() {
    show_border_right_ = true;
    return *this;
  }

  Format &hide_border_right() {
    show_border_right_ = false;
    return *this;
  }

  Format &corner(const std::string &value) {
    corner_top_left_ = value;
    corner_top_right_ = value;
    corner_bottom_left_ = value;
    corner_bottom_right_ = value;
    return *this;
  }

  Format &corner_color(Color value) {
    corner_top_left_color_ = value;
    corner_top_right_color_ = value;
    corner_bottom_left_color_ = value;
    corner_bottom_right_color_ = value;
    return *this;
  }

  Format &corner_background_color(Color value) {
    corner_top_left_background_color_ = value;
    corner_top_right_background_color_ = value;
    corner_bottom_left_background_color_ = value;
    corner_bottom_right_background_color_ = value;
    return *this;
  }

  Format &corner_top_left(const std::string &value) {
    corner_top_left_ = value;
    return *this;
  }

  Format &corner_top_left_color(Color value) {
    corner_top_left_color_ = value;
    return *this;
  }

  Format &corner_top_left_background_color(Color value) {
    corner_top_left_background_color_ = value;
    return *this;
  }

  Format &corner_top_right(const std::string &value) {
    corner_top_right_ = value;
    return *this;
  }

  Format &corner_top_right_color(Color value) {
    corner_top_right_color_ = value;
    return *this;
  }

  Format &corner_top_right_background_color(Color value) {
    corner_top_right_background_color_ = value;
    return *this;
  }

  Format &corner_bottom_left(const std::string &value) {
    corner_bottom_left_ = value;
    return *this;
  }

  Format &corner_bottom_left_color(Color value) {
    corner_bottom_left_color_ = value;
    return *this;
  }

  Format &corner_bottom_left_background_color(Color value) {
    corner_bottom_left_background_color_ = value;
    return *this;
  }

  Format &corner_bottom_right(const std::string &value) {
    corner_bottom_right_ = value;
    return *this;
  }

  Format &corner_bottom_right_color(Color value) {
    corner_bottom_right_color_ = value;
    return *this;
  }

  Format &corner_bottom_right_background_color(Color value) {
    corner_bottom_right_background_color_ = value;
    return *this;
  }

  Format &column_separator(const std::string &value) {
    column_separator_ = value;
    return *this;
  }

  Format &column_separator_color(Color value) {
    column_separator_color_ = value;
    return *this;
  }

  Format &column_separator_background_color(Color value) {
    column_separator_background_color_ = value;
    return *this;
  }

  Format &font_align(FontAlign value) {
    font_align_ = value;
    return *this;
  }

  Format &font_style(const std::vector<FontStyle> &style) {
    if (font_style_.has_value()) {
      for (auto &s : style)
        font_style_->push_back(s);
    } else {
      font_style_ = style;
    }
    return *this;
  }

  Format &font_color(Color value) {
    font_color_ = value;
    return *this;
  }

  Format &font_background_color(Color value) {
    font_background_color_ = value;
    return *this;
  }

  Format &color(Color value) {
    font_color(value);
    border_color(value);
    corner_color(value);
    return *this;
  }

  Format &background_color(Color value) {
    font_background_color(value);
    border_background_color(value);
    corner_background_color(value);
    return *this;
  }

  Format &multi_byte_characters(bool value) {
    multi_byte_characters_ = value;
    return *this;
  }

  Format &locale(const std::string &value) {
    locale_ = value;
    return *this;
  }

  // Apply word wrap
  // Given an input string and a line length, this will insert \n
  // in strategic places in input string and apply word wrapping
  static std::string word_wrap(const std::string &str, size_t width, const std::string &locale,
                               bool is_multi_byte_character_support_enabled) {
    std::vector<std::string> words = explode_string(str, {" ", "-", "\t"});
    size_t current_line_length = 0;
    std::string result;

    for (size_t i = 0; i < words.size(); ++i) {
      std::string word = words[i];
      // If adding the new word to the current line would be too long,
      // then put it on a new line (and split it up if it's too long).
      if (current_line_length +
              get_sequence_length(word, locale, is_multi_byte_character_support_enabled) >
          width) {
        // Only move down to a new line if we have text on the current line.
        // Avoids situation where wrapped whitespace causes emptylines in text.
        if (current_line_length > 0) {
          result += '\n';
          current_line_length = 0;
        }

        // If the current word is too long to fit on a line even on it's own then
        // split the word up.
        while (get_sequence_length(word, locale, is_multi_byte_character_support_enabled) > width) {
          result += word.substr(0, width - 1) + "-";
          word = word.substr(width - 1);
          result += '\n';
        }

        // Remove leading whitespace from the word so the new line starts flush to the left.
        word = trim_left(word);
      }
      result += word;
      current_line_length +=
          get_sequence_length(word, locale, is_multi_byte_character_support_enabled);
    }
    return result;
  }

  static std::vector<std::string> split_lines(const std::string &text, const std::string &delimiter,
                                              const std::string &locale,
                                              bool is_multi_byte_character_support_enabled) {
    std::vector<std::string> result{};
    std::string input = text;
    size_t pos = 0;
    std::string token;
    while ((pos = input.find(delimiter)) != std::string::npos) {
      token = input.substr(0, pos);
      result.push_back(token);
      input.erase(0, pos + delimiter.length());
    }
    if (get_sequence_length(input, locale, is_multi_byte_character_support_enabled))
      result.push_back(input);
    return result;
  };

  // Merge two formats
  // first has higher precedence
  // e.g., first = cell-level formatting and
  // second = row-level formatting
  // Result has attributes of both with cell-level
  // formatting taking precedence
  static Format merge(Format first, Format second) {
    Format result;

    // Width and height
    if (first.width_.has_value())
      result.width_ = first.width_;
    else
      result.width_ = second.width_;

    if (first.height_.has_value())
      result.height_ = first.height_;
    else
      result.height_ = second.height_;

    // Font styling
    if (first.font_align_.has_value())
      result.font_align_ = first.font_align_;
    else
      result.font_align_ = second.font_align_;

    if (first.font_style_.has_value()) {
      // Merge font styles using std::set_union
      std::vector<FontStyle> merged_font_style(first.font_style_->size() +
                                               second.font_style_->size());
#if defined(_WIN32) || defined(_WIN64)
      // Fixes error in Windows - Sequence not ordered
      std::sort(first.font_style_->begin(), first.font_style_->end());
      std::sort(second.font_style_->begin(), second.font_style_->end());
#endif
      std::set_union(first.font_style_->begin(), first.font_style_->end(),
                     second.font_style_->begin(), second.font_style_->end(),
                     merged_font_style.begin());
      result.font_style_ = merged_font_style;
    } else
      result.font_style_ = second.font_style_;

    if (first.font_color_.has_value())
      result.font_color_ = first.font_color_;
    else
      result.font_color_ = second.font_color_;

    if (first.font_background_color_.has_value())
      result.font_background_color_ = first.font_background_color_;
    else
      result.font_background_color_ = second.font_background_color_;

    // Padding
    if (first.padding_left_.has_value())
      result.padding_left_ = first.padding_left_;
    else
      result.padding_left_ = second.padding_left_;

    if (first.padding_top_.has_value())
      result.padding_top_ = first.padding_top_;
    else
      result.padding_top_ = second.padding_top_;

    if (first.padding_right_.has_value())
      result.padding_right_ = first.padding_right_;
    else
      result.padding_right_ = second.padding_right_;

    if (first.padding_bottom_.has_value())
      result.padding_bottom_ = first.padding_bottom_;
    else
      result.padding_bottom_ = second.padding_bottom_;

    // Border
    if (first.border_left_.has_value())
      result.border_left_ = first.border_left_;
    else
      result.border_left_ = second.border_left_;

    if (first.border_left_color_.has_value())
      result.border_left_color_ = first.border_left_color_;
    else
      result.border_left_color_ = second.border_left_color_;

    if (first.border_left_background_color_.has_value())
      result.border_left_background_color_ = first.border_left_background_color_;
    else
      result.border_left_background_color_ = second.border_left_background_color_;

    if (first.border_top_.has_value())
      result.border_top_ = first.border_top_;
    else
      result.border_top_ = second.border_top_;

    if (first.border_top_color_.has_value())
      result.border_top_color_ = first.border_top_color_;
    else
      result.border_top_color_ = second.border_top_color_;

    if (first.border_top_background_color_.has_value())
      result.border_top_background_color_ = first.border_top_background_color_;
    else
      result.border_top_background_color_ = second.border_top_background_color_;

    if (first.border_bottom_.has_value())
      result.border_bottom_ = first.border_bottom_;
    else
      result.border_bottom_ = second.border_bottom_;

    if (first.border_bottom_color_.has_value())
      result.border_bottom_color_ = first.border_bottom_color_;
    else
      result.border_bottom_color_ = second.border_bottom_color_;

    if (first.border_bottom_background_color_.has_value())
      result.border_bottom_background_color_ = first.border_bottom_background_color_;
    else
      result.border_bottom_background_color_ = second.border_bottom_background_color_;

    if (first.border_right_.has_value())
      result.border_right_ = first.border_right_;
    else
      result.border_right_ = second.border_right_;

    if (first.border_right_color_.has_value())
      result.border_right_color_ = first.border_right_color_;
    else
      result.border_right_color_ = second.border_right_color_;

    if (first.border_right_background_color_.has_value())
      result.border_right_background_color_ = first.border_right_background_color_;
    else
      result.border_right_background_color_ = second.border_right_background_color_;

    if (first.show_border_top_.has_value())
      result.show_border_top_ = first.show_border_top_;
    else
      result.show_border_top_ = second.show_border_top_;

    if (first.show_border_bottom_.has_value())
      result.show_border_bottom_ = first.show_border_bottom_;
    else
      result.show_border_bottom_ = second.show_border_bottom_;

    if (first.show_border_left_.has_value())
      result.show_border_left_ = first.show_border_left_;
    else
      result.show_border_left_ = second.show_border_left_;

    if (first.show_border_right_.has_value())
      result.show_border_right_ = first.show_border_right_;
    else
      result.show_border_right_ = second.show_border_right_;

    // Corner
    if (first.corner_top_left_.has_value())
      result.corner_top_left_ = first.corner_top_left_;
    else
      result.corner_top_left_ = second.corner_top_left_;

    if (first.corner_top_left_color_.has_value())
      result.corner_top_left_color_ = first.corner_top_left_color_;
    else
      result.corner_top_left_color_ = second.corner_top_left_color_;

    if (first.corner_top_left_background_color_.has_value())
      result.corner_top_left_background_color_ = first.corner_top_left_background_color_;
    else
      result.corner_top_left_background_color_ = second.corner_top_left_background_color_;

    if (first.corner_top_right_.has_value())
      result.corner_top_right_ = first.corner_top_right_;
    else
      result.corner_top_right_ = second.corner_top_right_;

    if (first.corner_top_right_color_.has_value())
      result.corner_top_right_color_ = first.corner_top_right_color_;
    else
      result.corner_top_right_color_ = second.corner_top_right_color_;

    if (first.corner_top_right_background_color_.has_value())
      result.corner_top_right_background_color_ = first.corner_top_right_background_color_;
    else
      result.corner_top_right_background_color_ = second.corner_top_right_background_color_;

    if (first.corner_bottom_left_.has_value())
      result.corner_bottom_left_ = first.corner_bottom_left_;
    else
      result.corner_bottom_left_ = second.corner_bottom_left_;

    if (first.corner_bottom_left_color_.has_value())
      result.corner_bottom_left_color_ = first.corner_bottom_left_color_;
    else
      result.corner_bottom_left_color_ = second.corner_bottom_left_color_;

    if (first.corner_bottom_left_background_color_.has_value())
      result.corner_bottom_left_background_color_ = first.corner_bottom_left_background_color_;
    else
      result.corner_bottom_left_background_color_ = second.corner_bottom_left_background_color_;

    if (first.corner_bottom_right_.has_value())
      result.corner_bottom_right_ = first.corner_bottom_right_;
    else
      result.corner_bottom_right_ = second.corner_bottom_right_;

    if (first.corner_bottom_right_color_.has_value())
      result.corner_bottom_right_color_ = first.corner_bottom_right_color_;
    else
      result.corner_bottom_right_color_ = second.corner_bottom_right_color_;

    if (first.corner_bottom_right_background_color_.has_value())
      result.corner_bottom_right_background_color_ = first.corner_bottom_right_background_color_;
    else
      result.corner_bottom_right_background_color_ = second.corner_bottom_right_background_color_;

    // Column separator
    if (first.column_separator_.has_value())
      result.column_separator_ = first.column_separator_;
    else
      result.column_separator_ = second.column_separator_;

    if (first.column_separator_color_.has_value())
      result.column_separator_color_ = first.column_separator_color_;
    else
      result.column_separator_color_ = second.column_separator_color_;

    if (first.column_separator_background_color_.has_value())
      result.column_separator_background_color_ = first.column_separator_background_color_;
    else
      result.column_separator_background_color_ = second.column_separator_background_color_;

    // Internationlization
    if (first.multi_byte_characters_.has_value())
      result.multi_byte_characters_ = first.multi_byte_characters_;
    else
      result.multi_byte_characters_ = second.multi_byte_characters_;

    if (first.locale_.has_value())
      result.locale_ = first.locale_;
    else
      result.locale_ = second.locale_;

    return result;
  }

private:
  friend class Cell;
  friend class Row;
  friend class Column;
  friend class TableInternal;
  friend class Printer;
  friend class MarkdownExporter;
  friend class LatexExporter;
  friend class AsciiDocExporter;

  void set_defaults() {
    // NOTE: width and height are not set here
    font_align_ = FontAlign::left;
    font_style_ = std::vector<FontStyle>{};
    font_color_ = font_background_color_ = Color::none;
    padding_left_ = padding_right_ = 1;
    padding_top_ = padding_bottom_ = 0;
    border_top_ = border_bottom_ = "-";
    border_left_ = border_right_ = "|";
    show_border_left_ = show_border_right_ = show_border_top_ = show_border_bottom_ = true;
    border_top_color_ = border_top_background_color_ = border_bottom_color_ =
        border_bottom_background_color_ = border_left_color_ = border_left_background_color_ =
            border_right_color_ = border_right_background_color_ = Color::none;
    corner_top_left_ = corner_top_right_ = corner_bottom_left_ = corner_bottom_right_ = "+";
    corner_top_left_color_ = corner_top_left_background_color_ = corner_top_right_color_ =
        corner_top_right_background_color_ = corner_bottom_left_color_ =
            corner_bottom_left_background_color_ = corner_bottom_right_color_ =
                corner_bottom_right_background_color_ = Color::none;
    column_separator_ = "|";
    column_separator_color_ = column_separator_background_color_ = Color::none;
    multi_byte_characters_ = false;
    locale_ = "";
  }

  // Helper methods for word wrapping:

  // trim white spaces from the left end of an input string
  static std::string trim_left(const std::string &input_string) {
    std::string result = input_string;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(),
                                              [](int ch) { return !std::isspace(ch); }));
    return result;
  }

  // trim white spaces from right end of an input string
  static std::string trim_right(const std::string &input_string) {
    std::string result = input_string;
    result.erase(
        std::find_if(result.rbegin(), result.rend(), [](int ch) { return !std::isspace(ch); })
            .base(),
        result.end());
    return result;
  }

  // trim white spaces from either end of an input string
  static std::string trim(const std::string &input_string) {
    return trim_left(trim_right(input_string));
  }

  static size_t index_of_any(const std::string &input, size_t start_index,
                             const std::vector<std::string> &split_characters) {
    std::vector<size_t> indices{};
    for (auto &c : split_characters) {
      auto index = input.find(c, start_index);
      if (index != std::string::npos)
        indices.push_back(index);
    }
    if (indices.size() > 0)
      return *std::min_element(indices.begin(), indices.end());
    else
      return std::string::npos;
  }

  static std::vector<std::string> explode_string(const std::string &input,
                                                 const std::vector<std::string> &split_characters) {
    std::vector<std::string> result{};
    size_t start_index{0};
    while (true) {
      auto index = index_of_any(input, start_index, split_characters);

      if (index == std::string::npos) {
        result.push_back(input.substr(start_index));
        return result;
      }

      std::string word = input.substr(start_index, index - start_index);
      char next_character = input.substr(index, 1)[0];
      // Unlike whitespace, dashes and the like should stick to the word occurring before it.
      if (isspace(next_character)) {
        result.push_back(word);
        result.push_back(std::string(1, next_character));
      } else {
        result.push_back(word + next_character);
      }
      start_index = index + 1;
    }

    return result;
  }

  // Element width and height
  optional<size_t> width_{};
  optional<size_t> height_{};

  // Font styling
  optional<FontAlign> font_align_{};
  optional<std::vector<FontStyle>> font_style_{};
  optional<Color> font_color_{};
  optional<Color> font_background_color_{};

  // Element padding
  optional<size_t> padding_left_{};
  optional<size_t> padding_top_{};
  optional<size_t> padding_right_{};
  optional<size_t> padding_bottom_{};

  // Element border
  optional<bool> show_border_top_{};
  optional<std::string> border_top_{};
  optional<Color> border_top_color_{};
  optional<Color> border_top_background_color_{};

  optional<bool> show_border_bottom_{};
  optional<std::string> border_bottom_{};
  optional<Color> border_bottom_color_{};
  optional<Color> border_bottom_background_color_{};

  optional<bool> show_border_left_{};
  optional<std::string> border_left_{};
  optional<Color> border_left_color_{};
  optional<Color> border_left_background_color_{};

  optional<bool> show_border_right_{};
  optional<std::string> border_right_{};
  optional<Color> border_right_color_{};
  optional<Color> border_right_background_color_{};

  // Element corner
  optional<std::string> corner_top_left_{};
  optional<Color> corner_top_left_color_{};
  optional<Color> corner_top_left_background_color_{};

  optional<std::string> corner_top_right_{};
  optional<Color> corner_top_right_color_{};
  optional<Color> corner_top_right_background_color_{};

  optional<std::string> corner_bottom_left_{};
  optional<Color> corner_bottom_left_color_{};
  optional<Color> corner_bottom_left_background_color_{};

  optional<std::string> corner_bottom_right_{};
  optional<Color> corner_bottom_right_color_{};
  optional<Color> corner_bottom_right_background_color_{};

  // Element column separator
  optional<std::string> column_separator_{};
  optional<Color> column_separator_color_{};
  optional<Color> column_separator_background_color_{};

  // Internationalization
  optional<bool> multi_byte_characters_{};
  optional<std::string> locale_{};
};

} // namespace tabulate
