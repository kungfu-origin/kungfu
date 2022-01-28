<p align="center">
  <img height="50" src="img/logo.jpg"/>  
</p>

<p align="center">
  <a href="https://travis-ci.com/p-ranav/tabulate">
    <img src="https://travis-ci.com/p-ranav/tabulate.svg?token=haZMYySrhmkE9vsJse61&branch=master" alt="ci status"/>
  </a>
  <a href="https://en.wikipedia.org/wiki/C%2B%2B11">
    <img src="https://img.shields.io/badge/C%2B%2B-11-blue.svg" alt="standard"/>
  </a>
  <a href="https://github.com/p-ranav/tabulate/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="license"/>
  </a>
  <img src="https://img.shields.io/badge/version-1.4-blue.svg?cacheSeconds=2592000" alt="version"/>
</p>

<p align="center">
  <img src="img/summary.png"/>  
  <p align="center">
   Source for the above image can be found
    <a href="https://github.com/p-ranav/tabulate/blob/master/samples/summary.cpp">
      here
    </a>
  </p>
</p>

## Table of Contents

*   [Quick Start](#quick-start)
*   [Formatting Options](#formatting-options)
    *   [Style Inheritance Model](#style-inheritance-model)
    *   [Word Wrapping](#word-wrapping)
    *   [Font Alignment](#font-alignment)
    *   [Font Styles](#font-styles)
    *   [Cell Colors](#cell-colors)
    *   [Borders and Corners](#borders-and-corners)
    *   [Range-based Iteration](#range-based-iteration)
    *   [Nested Tables](#nested-tables)
    *   [UTF-8 Support](#utf-8-support)
*   [Exporters](#exporters)
    *   [Markdown](#markdown)
    *   [AsciiDoc](#asciidoc)
*   [Building Samples](#building-samples)
*   [Generating Single Header](#generating-single-header)
*   [Contributing](#contributing)
*   [License](#license)

## Quick Start

`tabulate` is a header-only library. Just add `include/` to your `include_directories` and you should be good to go. A single header file version is also available in `single_include/`.

**NOTE** Tabulate supports `>=C++11`. The rest of this README, however, assumes `C++17` support. 

Create a `Table` object and call `Table.add_rows` to add rows to your table.

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {

  Table universal_constants;

  universal_constants.add_row({"Quantity", "Value"});
  universal_constants.add_row({"Characteristic impedance of vacuum", "376.730 313 461... Ω"});
  universal_constants.add_row({"Electric constant (permittivity of free space)", "8.854 187 817... × 10⁻¹²F·m⁻¹"});
  universal_constants.add_row({"Magnetic constant (permeability of free space)", "4π × 10⁻⁷ N·A⁻² = 1.2566 370 614... × 10⁻⁶ N·A⁻²"});
  universal_constants.add_row({"Gravitational constant (Newtonian constant of gravitation)", "6.6742(10) × 10⁻¹¹m³·kg⁻¹·s⁻²"});
  universal_constants.add_row({"Planck's constant", "6.626 0693(11) × 10⁻³⁴ J·s"});
  universal_constants.add_row({"Dirac's constant", "1.054 571 68(18) × 10⁻³⁴ J·s"});
  universal_constants.add_row({"Speed of light in vacuum", "299 792 458 m·s⁻¹"});
```

You can format this table using `Table.format()` which returns a `Format` object. Using a fluent interface, format properties of the table, e.g., borders, font styles, colors etc.

```cpp
  universal_constants.format()
    .font_style({FontStyle::bold})
    .border_top(" ")
    .border_bottom(" ")
    .border_left(" ")
    .border_right(" ")
    .corner(" ");
```

You can access rows in the table using `Table[row_index]`. This will return a `Row` object on which you can similarly call `Row.format()` to format properties of all the cells in that row.

Now, let's format the header of the table. The following code changes the font background of the header row to `red`, aligns the cell contents to `center` and applies a padding to the top and bottom of the row.

```cpp
  universal_constants[0].format()
    .padding_top(1)
    .padding_bottom(1)
    .font_align(FontAlign::center)
    .font_style({FontStyle::underline})
    .font_background_color(Color::red);
```

Calling `Table.column(index)` will return a `Column` object. Similar to rows, you can use `Column.format()` to format all the cells in that column.

Now, let's change the font color of the second column to yellow:

```cpp
  universal_constants.column(1).format()
    .font_color(Color::yellow);
```

You can access cells by indexing twice from a table: From a row using `Table[row_index][col_index]` or from a column using `Table.column(col_index)[cell_index]`. Just like rows, columns, and tables, you can use `Cell.format()` to format individual cells

```cpp
  universal_constants[0][1].format()
    .font_background_color(Color::blue)
    .font_color(Color::white);
}
```

Print the table using the stream `operator<<` like so:

```cpp
  std::cout << universal_constants << std::endl;
``` 

You could also use `Table.print(stream)` to print the table, e.g., `universal_constants.print(std::cout)`. 

<p align="center">
  <img src="img/universal_constants.png"/>  
</p>

## Formatting Options

### Style Inheritance Model

Formatting in `tabulate` follows a simple style-inheritance model. When rendering each cell:
1.    Apply cell formatting if specified
2.    If no cell formatting is specified, apply its parent row formatting
3.    If no row formatting is specified, apply its parent table formatting
4.    If no table formatting is specified, apply the default table formatting

This enables overriding the formatting for a particular cell even though row or table formatting is specified, e.g., when an entire row is colored `yellow` but you want a specific cell to be colored `red`.

### Word Wrapping

`tabulate` supports automatic word-wrapping when printing cells. 

Although word-wrapping is automatic, there is a simple override. Automatic word-wrapping is used only if the cell contents do not have any embedded newline `\n` characters. So, you can embed newline characters in the cell contents and enforce the word-wrapping manually. 

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table table;

  table.add_row({"This paragraph contains a veryveryveryveryveryverylong word. The long word will "
                 "break and word wrap to the next line.",
                 "This paragraph \nhas embedded '\\n' \ncharacters and\n will break\n exactly "
                 "where\n you want it\n to\n break."});

  table[0][0].format().width(20);
  table[0][1].format().width(50);

  std::cout << table << std::endl;
}
```

*  The above table has 1 row and 2 columns. 
*  The first cell has automatic word-wrapping. 
*  The second cell uses the embedded newline characters in the cell contents - even though the second column has plenty of space (50 characters width), it uses user-provided newline characters to break into new lines and enforce the cell style.
*  **NOTE**: Whether word-wrapping is automatic or not, `tabulate` performs a trim operation on each line of each cell to remove whitespace characters from either side of line.

<p align="center">
  <img src="img/word_wrapping.png"/>  
</p>

**NOTE**: Both columns in the above table are left-aligned by default. This, however, can be easily changed.

### Font Alignment

`tabulate` supports three font alignment settings: `left`, `center`, and `right`. By default, all table content is left-aligned. To align cells, use `.format().font_align(alignment)`. 

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table movies;
  movies.add_row({"S/N", "Movie Name", "Director", "Estimated Budget", "Release Date"});
  movies.add_row({"tt1979376", "Toy Story 4", "Josh Cooley", "$200,000,000", "21 June 2019"});
  movies.add_row({"tt3263904", "Sully", "Clint Eastwood", "$60,000,000", "9 September 2016"});
  movies.add_row({"tt1535109", "Captain Phillips", "Paul Greengrass", "$55,000,000", " 11 October 2013"});

  // center align 'Director' column
  movies.column(2).format()
    .font_align(FontAlign::center);

  // right align 'Estimated Budget' column
  movies.column(3).format()
    .font_align(FontAlign::right);

  // right align 'Release Date' column
  movies.column(4).format()
    .font_align(FontAlign::right);

  // center-align and color header cells
  for (size_t i = 0; i < 5; ++i) {
    movies[0][i].format()
      .font_color(Color::yellow)
      .font_align(FontAlign::center)
      .font_style({FontStyle::bold});
  }

  std::cout << movies << std::endl;
}
```

<p align="center">
  <img src="img/movies.png"/>  
</p>

### Font Styles

`tabulate` supports 8 font styles: `bold`, `dark`, `italic`, `underline`, `blink`, `reverse`, `concealed`, `crossed`. Depending on the terminal (or terminal settings), some of these might not work. 

To apply a font style, simply call `.format().font_style({...})`. The `font_style` method takes a vector of font styles. This allows to apply multiple font styles to a cell, e.g., ***bold and italic***.

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table styled_table;
  styled_table.add_row({"Bold", "Italic", "Bold & Italic", "Blinking"});
  styled_table.add_row({"Underline", "Crossed", "Dark", "Bold, Italic & Underlined"});

  styled_table[0][0].format()
    .font_style({FontStyle::bold});

  styled_table[0][1].format()
    .font_style({FontStyle::italic});

  styled_table[0][2].format()
    .font_style({FontStyle::bold, FontStyle::italic});

  styled_table[0][3].format()
    .font_style({FontStyle::blink});

  styled_table[1][0].format()
    .font_style({FontStyle::underline});

  styled_table[1][1].format()
    .font_style({FontStyle::crossed});

  styled_table[1][2].format()
    .font_style({FontStyle::dark});


  styled_table[1][3].format()
    .font_style({FontStyle::bold, FontStyle::italic, FontStyle::underline});

  std::cout << styled_table << std::endl;

}
```

<p align="center">
  <img src="img/font_styles.png"/>  
</p>

**NOTE**: Font styles are applied to the entire cell. Unlike HTML, you cannot currently apply styles to specific words in a cell.

### Cell Colors

There are a number of methods in the `Format` object to color cells - foreground and background - for font, borders, corners, and column separators. Thanks to [termcolor](https://github.com/ikalnytskyi/termcolor), `tabulate` supports 8 colors: `grey`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, and `white`. The look of these colors vary depending on your terminal.

For font, border, and corners, you can call `.format().<element>_color(value)` to set the foreground color and `.format().<element>_background_color(value)` to set the background color. Here's an example:

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table colors;

  colors.add_row({"Font Color is Red", "Font Color is Blue", "Font Color is Green"});
  colors.add_row({"Everything is Red", "Everything is Blue", "Everything is Green"});
  colors.add_row({"Font Background is Red", "Font Background is Blue", "Font Background is Green"});

  colors[0][0].format()
    .font_color(Color::red)
    .font_style({FontStyle::bold});
  colors[0][1].format()
    .font_color(Color::blue)
    .font_style({FontStyle::bold});
  colors[0][2].format()
    .font_color(Color::green)
    .font_style({FontStyle::bold});

  colors[1][0].format()
    .border_left_color(Color::red)
    .border_left_background_color(Color::red)
    .font_background_color(Color::red)
    .font_color(Color::red);

  colors[1][1].format()
    .border_left_color(Color::blue)
    .border_left_background_color(Color::blue)
    .font_background_color(Color::blue)
    .font_color(Color::blue);

  colors[1][2].format()
    .border_left_color(Color::green)
    .border_left_background_color(Color::green)
    .font_background_color(Color::green)
    .font_color(Color::green)
    .border_right_color(Color::green)
    .border_right_background_color(Color::green);

  colors[2][0].format()
    .font_background_color(Color::red)
    .font_style({FontStyle::bold});
  colors[2][1].format()
    .font_background_color(Color::blue)
    .font_style({FontStyle::bold});
  colors[2][2].format()
    .font_background_color(Color::green)
    .font_style({FontStyle::bold});

  std::cout << colors << std::endl;
}
```

<p align="center">
  <img src="img/colors.png"/>  
</p>

### Borders and Corners

`tabulate` allows for fine control over borders and corners. For each border and corner, you can set the text, color, and background color.

**NOTE**: You can use `.corner(..)`, `.corner_color(..)`, and `.corner_background_color(..)` to set a common style for all corners. Similarly, you can use `.border(..)`, `.border_color(..)` and `.border_background_color(..)` to set a common style for all borders.

**NOTE**: Note the use of `.format().multi_byte_characters(true)`. Use this when you know your table has multi-byte characters. This is an opt-in because the calculation of column width when dealing with multi-byte characters is more involved and you don't want to pay the performance penalty unless you need it. Just like any other format setting, you can set this at the table-level, row-level, or on a per-cell basis. 

Here's an example where each border and corner is individually styled:

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table table;

  table.add_row({"ᛏᚺᛁᛊ ᛁᛊ ᚨ ᛊᛏᛟᚱy ᛟᚠᚨ ᛒᛖᚨᚱ ᚨᚾᛞ\n"
                 "ᚨ ᚹᛟᛚᚠ, ᚹᚺᛟ ᚹᚨᚾᛞᛖᚱᛖᛞ ᛏᚺᛖ\n"
                 "ᚱᛖᚨᛚᛗᛊ ᚾᛁᚾᛖ ᛏᛟ ᚠᚢᛚᚠᛁᛚᛚ ᚨ ᛈᚱᛟᛗᛁᛊᛖ\n"
                 "ᛏᛟ ᛟᚾᛖ ᛒᛖᚠᛟᚱᛖ; ᛏᚺᛖy ᚹᚨᛚᚲ ᛏᚺᛖ\n"
                 "ᛏᚹᛁᛚᛁᚷᚺᛏ ᛈᚨᛏᚺ, ᛞᛖᛊᛏᛁᚾᛖᛞ ᛏᛟ\n"
                 "ᛞᛁᛊcᛟᚹᛖᚱ ᛏᚺᛖ ᛏᚱᚢᛏᚺ\nᛏᚺᚨᛏ ᛁᛊ ᛏᛟ cᛟᛗᛖ."});

  table.format()
      .multi_byte_characters(true)
      // Font styling
      .font_style({FontStyle::bold, FontStyle::dark})
      .font_align(FontAlign::center)
      .font_color(Color::red)
      .font_background_color(Color::yellow)
      // Corners
      .corner_top_left("ᛰ")
      .corner_top_right("ᛯ")
      .corner_bottom_left("ᛮ")
      .corner_bottom_right("ᛸ")
      .corner_top_left_color(Color::cyan)
      .corner_top_right_color(Color::yellow)
      .corner_bottom_left_color(Color::green)
      .corner_bottom_right_color(Color::red)
      // Borders
      .border_top("ᛜ")
      .border_bottom("ᛜ")
      .border_left("ᚿ")
      .border_right("ᛆ")
      .border_left_color(Color::yellow)
      .border_right_color(Color::green)
      .border_top_color(Color::cyan)
      .border_bottom_color(Color::red);

  std::cout << table << std::endl;
}
```

<p align="center">
  <img src="img/runic.png"/>  
</p>

### Range-based Iteration

Hand-picking and formatting cells using `operator[]` gets tedious very quickly. To ease this, `tabulate` supports range-based iteration on tables, rows, and columns. Quickly iterate over rows and columns to format cells.

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table table;

  table.add_row({"Company", "Contact", "Country"});
  table.add_row({"Alfreds Futterkiste", "Maria Anders", "Germany"});
  table.add_row({"Centro comercial Moctezuma", "Francisco Chang", "Mexico"});
  table.add_row({"Ernst Handel", "Roland Mendel", "Austria"});
  table.add_row({"Island Trading", "Helen Bennett", "UK"});
  table.add_row({"Laughing Bacchus Winecellars", "Yoshi Tannamuri", "Canada"});
  table.add_row({"Magazzini Alimentari Riuniti", "Giovanni Rovelli", "Italy"});

  // Set width of cells in each column
  table.column(0).format().width(40);
  table.column(1).format().width(30);
  table.column(2).format().width(30);

  // Iterate over cells in the first row
  for (auto& cell : table[0]) {
    cell.format()
      .font_style({FontStyle::underline})
      .font_align(FontAlign::center);
  }

  // Iterator over cells in the first column
  for (auto& cell : table.column(0)) {
    if (cell.get_text() != "Company") {
      cell.format()
        .font_align(FontAlign::right);
    }
  }

  // Iterate over rows in the table
  size_t index = 0;
  for (auto& row : table) {
    row.format()
      .font_style({FontStyle::bold});

    // Set blue background color for alternate rows
    if (index > 0 && index % 2 == 0) {
      for (auto& cell : row) {
        cell.format()
          .font_background_color(Color::blue);
      }      
    }
    index += 1;
  }

  std::cout << table << std::endl;
}
```

<p align="center">
  <img src="img/iterators.png"/>  
</p>

### Nested Tables

`Table.add_row(...)` takes either a `std::string` or a `tabulate::Table`. This can be used to nest tables within tables. Here's an example program that prints a UML class diagram using `tabulate`. Note the use of font alignment, style, and width settings to generate a diagram that looks centered and great.

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table class_diagram;

  // Global styling
  class_diagram.format().font_style({FontStyle::bold}).font_align(FontAlign::center).width(60);

  // Animal class
  Table animal;
  animal.add_row({"Animal"});
  animal[0].format().font_align(FontAlign::center);

  // Animal properties nested table
  Table animal_properties;
  animal_properties.format().width(20);
  animal_properties.add_row({"+age: Int"});
  animal_properties.add_row({"+gender: String"});
  animal_properties[1].format().hide_border_top();

  // Animal methods nested table
  Table animal_methods;
  animal_methods.format().width(20);
  animal_methods.add_row({"+isMammal()"});
  animal_methods.add_row({"+mate()"});
  animal_methods[1].format().hide_border_top();

  animal.add_row({animal_properties});
  animal.add_row({animal_methods});
  animal[2].format().hide_border_top();

  class_diagram.add_row({animal});

  // Add rows in the class diagram for the up-facing arrow
  // THanks to center alignment, these will align just fine
  class_diagram.add_row({"▲"});
  class_diagram[1][0].format().hide_border_top().multi_byte_characters(true); // ▲ is multi-byte
  
  class_diagram.add_row({"|"});
  class_diagram[2].format().hide_border_top();
  class_diagram.add_row({"|"});
  class_diagram[3].format().hide_border_top();

  // Duck class
  Table duck;
  duck.add_row({"Duck"});
  duck[0].format().font_align(FontAlign::center);

  // Duck proeperties nested table
  Table duck_properties;
  duck_properties.format().width(40);
  duck_properties.add_row({"+beakColor: String = \"yellow\""});

  // Duck methods nested table
  Table duck_methods;
  duck_methods.format().width(40);
  duck_methods.add_row({"+swim()"});
  duck_methods.add_row({"+quack()"});
  duck_methods[1].format().hide_border_top();

  duck.add_row({duck_properties});
  duck.add_row({duck_methods});
  duck[2].format().hide_border_top();

  class_diagram.add_row({duck});
  class_diagram[4].format().hide_border_top();

  std::cout << class_diagram << std::endl;
}
```

<p align="center">
  <img height="600" src="img/class_diagram.png"/>  
</p>

### UTF-8 Support

In \*nix, `wcswidth` is used to compute the display width of multi-byte characters. Column alignment works well when your system supports the necessary locale, e.g., I've noticed on MacOS 10 there is no Arabic locale (searched with `locale -a`) and this ends up causing alignment issues when using Arabic text, e.g., `"ٲنَا بحِبَّك (Ana bahebak)"` in tables.

The following table prints the phrase `I love you` in different languages. Note the use of `.format().multi_byte_characters(true)` for the second column. Remember to do this when dealing with multi-byte characters. 

```cpp
#include <tabulate/table.hpp>
using namespace tabulate;

int main() {
  Table table;

  table.format().corner("♥")
    .font_style({FontStyle::bold})
    .corner_color(Color::magenta)
    .border_color(Color::magenta);

  table.add_row({"English", "I love you"});
  table.add_row({"French", "Je t’aime"});
  table.add_row({"Spanish", "Te amo"});
  table.add_row({"German", "Ich liebe Dich"});
  table.add_row({"Mandarin Chinese", "我爱你"});
  table.add_row({"Japanese", "愛してる"});
  table.add_row({"Korean", "사랑해 (Saranghae)"});
  table.add_row({"Greek", "Σ΄αγαπώ (Se agapo)"});
  table.add_row({"Italian", "Ti amo"});
  table.add_row({"Russian", "Я тебя люблю (Ya tebya liubliu)"});
  table.add_row({"Hebrew", "אני אוהב אותך (Ani ohev otakh)"});

  // Column 1 is using mult-byte characters
  table.column(1).format()
    .multi_byte_characters(true);

  std::cout << table << std::endl;
}
```

<p align="center">
  <img height="600" src="img/unicode.png"/>  
</p>

You can explicitly set the locale for a cell using `.format().locale(value)`. Note that the locale string is system-specific. So, the following code might throw `std::runtime_error locale::facet::_S_create_c_locale name not valid` on your system. 

```cpp
  // Set English-US locale for first column
  table.column(0).format().locale("en_US.UTF-8");
  table[0][1].format().locale("en_US.UTF-8");

  // Set locale for individual cells
  table[1][1].format().locale("fr_FR.UTF-8");  // French
  table[2][1].format().locale("es_ES.UTF-8");  // Spanish
  table[3][1].format().locale("de_DE.UTF-8");  // German
  table[4][1].format().locale("zh_CN.UTF-8");  // Chinese
  table[5][1].format().locale("ja_JP.UTF-8");  // Japanese
  table[6][1].format().locale("ko_KR.UTF-8");  // Korean
  table[7][1].format().locale("el_GR.UTF-8");  // Greek
  table[8][1].format().locale("it_IT.UTF-8");  // Italian
  table[9][1].format().locale("ru_RU.UTF-8");  // Russian
  table[10][1].format().locale("he_IL.UTF-8"); // Hebrew
```

## Exporters

### Markdown

Tables can be exported to GitHub-flavored markdown using a `MarkdownExporter`. Simply create an exporter object and call `exporter.dump(table)` to generate a Markdown-formatted `std::string`. 

```cpp
#include <tabulate/markdown_exporter.hpp>
using namespace tabulate;

int main() {
  Table movies;
  movies.add_row({"S/N", "Movie Name", "Director", "Estimated Budget", "Release Date"});  
  movies.add_row({"tt1979376", "Toy Story 4", "Josh Cooley", "$200,000,000", "21 June 2019"});
  movies.add_row({"tt3263904", "Sully", "Clint Eastwood", "$60,000,000", "9 September 2016"});
  movies.add_row(
      {"tt1535109", "Captain Phillips", "Paul Greengrass", "$55,000,000", " 11 October 2013"});

  // center align 'Director' column
  movies.column(2).format().font_align(FontAlign::center);

  // right align 'Estimated Budget' column
  movies.column(3).format().font_align(FontAlign::right);

  // right align 'Release Date' column
  movies.column(4).format().font_align(FontAlign::right);

  // Color header cells
  for (size_t i = 0; i < 5; ++i) {
    movies[0][i].format().font_color(Color::yellow).font_style({FontStyle::bold});
  }

  // Export to Markdown
  MarkdownExporter exporter;
  auto markdown = exporter.dump(movies);

  // tabulate::table
  std::cout << movies << "\n\n";

  // Exported Markdown
  std::cout << markdown << std::endl;
}
```

<p align="center">
  <img src="img/markdown_export.png"/>  
</p>

The above table renders in Markdown like below.

**NOTE**: Unlike `tabulate`, you cannot align individual cells in Markdown. Alignment is on a per-column basis. Markdown allows a second header row where such column-wise alignment can be specified. The `MarkdownExporter` uses the formatting of the header cells in the original `tabulate::Table` to decide how to align each column. As per the Markdown spec, columns are left-aligned by default.

| S/N       | Movie Name       |     Director    | Estimated Budget |     Release Date |
| :----     | :----            |      :---:      |            ----: |            ----: |
| tt1979376 | Toy Story 4      |   Josh Cooley   |     $200,000,000 |     21 June 2019 |
| tt3263904 | Sully            |  Clint Eastwood |      $60,000,000 | 9 September 2016 |
| tt1535109 | Captain Phillips | Paul Greengrass |      $55,000,000 |  11 October 2013 |

### AsciiDoc

Tabulate can export tables as AsciiDoc using an `AsciiDocExporter`. 

```cpp
#include <tabulate/asciidoc_exporter.hpp>
using namespace tabulate;

int main() {
  Table movies;
  movies.add_row({"S/N", "Movie Name", "Director", "Estimated Budget", "Release Date"});
  movies.add_row({"tt1979376", "Toy Story 4", "Josh Cooley", "$200,000,000", "21 June 2019"});
  movies.add_row({"tt3263904", "Sully", "Clint Eastwood", "$60,000,000", "9 September 2016"});
  movies.add_row(
      {"tt1535109", "Captain Phillips", "Paul Greengrass", "$55,000,000", " 11 October 2013"});

  // center align 'Director' column
  movies.column(2).format().font_align(FontAlign::center);

  // right align 'Estimated Budget' column
  movies.column(3).format().font_align(FontAlign::right);

  // right align 'Release Date' column
  movies.column(4).format().font_align(FontAlign::right);

  movies[1][2].format().font_style({FontStyle::bold, FontStyle::italic});
  movies[2][1].format().font_style({FontStyle::italic});

  // Color header cells
  for (size_t i = 0; i < 5; ++i) {
    movies[0][i]
        .format()
        .font_color(Color::white)
        .font_style({FontStyle::bold})
        .background_color(Color::blue);
  }

  AsciiDocExporter exporter;
  auto asciidoc = exporter.dump(movies);

  // tabulate::table
  std::cout << movies << "\n\n";

  // Exported AsciiDoc
  std::cout << asciidoc << std::endl;
}
```
Below is the export of the example above:

```
[cols="<,<,^,>,>"]
|===
|*S/N*|*Movie Name*|*Director*|*Estimated Budget*|*Release Date*

|tt1979376|Toy Story 4|*_Josh Cooley_*|$200,000,000|21 June 2019
|tt3263904|_Sully_|Clint Eastwood|$60,000,000|9 September 2016
|tt1535109|Captain Phillips|Paul Greengrass|$55,000,000| 11 October 2013
|===
```
The rendered output you can see here: http://tpcg.io/pbbfU3ks

**NOTE** Alignment is only supported per column. The font styles `FontStyle::bold` and `FontStyle::italic` can be used for each cell, also in combination.

## Building Samples

There are a number of samples in the `samples/` directory, e.g., [Mario](https://github.com/p-ranav/tabulate/blob/master/samples/mario.cpp). You can build these samples by running the following commands.

```bash
mkdir build
cd build
cmake -DSAMPLES=ON -DUSE_CPP17=ON ..
make
./samples/mario
```

Note the `USE_CPP17` variable. `Tabulate` uses `std::variant` and `std::optional`. If you do not have `C++17` compiler support for these data structures, build without this flag. `Tabulate` will then use [variant-lite](https://github.com/martinmoene/variant-lite) and [optional-lite](https://github.com/martinmoene/optional-lite).

<p align="center">
  <img width="400" src="img/mario.png"/>  
</p>

## Generating Single Header

```bash
python3 utils/amalgamate/amalgamate.py -c single_include.json -s .
```

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
