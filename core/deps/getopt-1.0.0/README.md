getopt <a href="https://travis-ci.org/r-lyeh/getopt"><img src="https://api.travis-ci.org/r-lyeh/getopt.svg?branch=master" align="right" /></a><a href="LICENSE"><img src="https://img.shields.io/badge/license-zlib/libpng-blue.svg" align="right" /></a>
====

Simple command-line options handler (C++11).

## Features 
- [x] Handy, one line per option.
- [x] Tiny, cross-platform, self-contained, header-only.
- [x] ZLIB/libPNG licensed.

## Showcase
```c++
#> cat sample.cc
#include <iostream>
#include <string>
#include "getopt.hpp"
int main() {
	// - No initialization required: (argc, argv) pair automatically retrieved.
	// - First argument is default option value, then all option indentifiers follow.
    bool help = getarg( false, "-h", "--help", "-?" );
    int version = getarg( 0, "-v", "--version", "--show-version" );
    int depth = getarg( 1, "-d", "--depth", "--max-depth");
    std::string file = getarg( "", "-f", "--file" );
    // [...]
    std::cout << help << ',' << file << ',' << version << std::endl;
}

#> g++ sample.cc && ./a.out --file=/a/b/c --depth=123
0, /a/b/c, 123
```

## Changelog
- v1.0.0 (2016/04/18): Initial version
