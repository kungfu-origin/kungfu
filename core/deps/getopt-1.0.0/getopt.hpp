// Simple getopt replacement class (C++11). 
// - rlyeh, zlib/libpng licensed.

// Two APIs provided:
//
// 1) Simple functional api `getarg(...)`. 
//    - No initialization required: (argc, argv) pair automatically retrieved.
//    - First argument is default option value, then all option indentifiers follow.
//
// int main() {
//     bool help = getarg( false, "-h", "--help", "-?" );
//     int version = getarg( 0, "-v", "--version", "--show-version" );
//     int depth = getarg( 1, "-d", "--depth", "--max-depth");
//     std::string file = getarg( "", "-f", "--file" );
//     [...]
// }
//
// 2) Simple OOP map-based api `getopt class`. Initialization (argc, argv) pair required.
//
//    This getopt class is a std::map replacement where key/value are std::string types.
//    Given invokation './app.out --user=me --pass=123 -h' this class delivers not only:
//    map[0] = "./app.out", map[1] = "--user=me", map[2]="--pass=123", map[3]='-h'
//    but also, map["--user"]="me", map["--pass"]="123" and also, map["-h"]=true
//
//    Additional API:
//    - .cmdline() for a print app invokation string
//    - .str() for pretty map printing
//    - .size() number of arguments (equivalent to argc), rather than std::map.size()
//
// int main( int argc, const char **argv ) {
//     getopt args( argc, argv );
//     if( args.has("-h") || args.has("--help") || args.has("-?") || args.size() == 1 ) {
//         std::cout << args["0"] << " [-?|-h|--help] [-v|--version] [--depth=number]" << std::endl;
//         return 0;
//     }
//     if( args.has("-v") || args.has("--version") ) {
//         std::cout << args["0"] << " sample v1.0.0. Compiled on " << __DATE__ << std::endl;
//     }
//     if( args.has("--depth") ) {
//         int depth = atoi( args["--depth"].c_str() );
//         std::cout << "depth set to " << depth << std::endl;
//     }
//     [...]
// }

#pragma once
#include <map>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#else
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#endif

#define GETOPT_VERSION "1.0.0" // (2016/04/18) Initial version

namespace getopt_utils
{
    // string conversion

    template< typename T >
    inline T as( const std::string &self ) {
        T t;
        return (std::istringstream(self) >> t) ? t :
               (T)(self.size() && (self != "0") && (self != "false"));
    }

    template<>
    inline char as( const std::string &self ) {
        return self.size() == 1 ? (char)(self[0]) : (char)(as<int>(self));
    }
    template<>
    inline signed char as( const std::string &self ) {
        return self.size() == 1 ? (signed char)(self[0]) : (signed char)(as<int>(self));
    }
    template<>
    inline unsigned char as( const std::string &self ) {
        return self.size() == 1 ? (unsigned char)(self[0]) : (unsigned char)(as<int>(self));
    }

    template<>
    inline const char *as( const std::string &self ) {
        return self.c_str();
    }
    template<>
    inline std::string as( const std::string &self ) {
        return self;
    }

    // token split

    inline size_t split( std::vector<std::string> &tokens, const std::string &self, const std::string &delimiters ) {
        std::string str;
        tokens.clear();
        for( auto &ch : self ) {
            if( delimiters.find_first_of( ch ) != std::string::npos ) {
                if( str.size() ) tokens.push_back( str ), str = "";
                tokens.push_back( std::string() + ch );
            } else str += ch;
        }
        return str.empty() ? tokens.size() : ( tokens.push_back( str ), tokens.size() );
    };

    // portable cmdline 

    inline std::vector<std::string> cmdline() {
        std::vector<std::string> args;
        std::string arg;
    #       ifdef _WIN32
            int argv;
            auto *list = CommandLineToArgvW( GetCommandLineW(), &argv );
            if( list ) {
                for( int i = 0; i < argv; ++i ) {
                    std::wstring ws( list[i] );
                    args.push_back( std::string( ws.begin(), ws.end() ) );
                }
                LocalFree(list);
            }
    #       else
            pid_t pid = getpid();

            char fname[32] = {};
            sprintf(fname, "/proc/%d/cmdline", pid);
            std::ifstream ifs(fname);
            if( ifs.good() ) {
                std::stringstream ss;
                ifs >> ss.rdbuf();
                arg = ss.str();
            }
            for( auto end = arg.size(), i = end - end; i < end; ++i ) {
                auto st = i;
                while (i < arg.size() && arg[i] != '\0') ++i;
                args.push_back( arg.substr(st, i - st) );
            }
    #       endif
        return args;
    }
}

// main map class; explicit initialization

struct getopt : public std::map< std::string, std::string >
{
    using super = std::map< std::string, std::string >;

    getopt( int argc, const char **argv ) : super() {
        // reconstruct vector
        std::vector<std::string> args( argc, std::string() );
        for( int i = 0; i < argc; ++i ) {
            args[ i ] = argv[ i ];
        }
        // create key=value and key= args as well
        for( auto &it : args ) {
            std::vector<std::string> tokens;
            auto size = getopt_utils::split( tokens, it, "=" );

            if( size == 3 && tokens[1] == "=" )
                (*this)[ tokens[0] ] = tokens[2];
            else
            if( size == 2 && tokens[1] == "=" )
                (*this)[ tokens[0] ] = true;
            else
            if( size == 1 && tokens[0] != argv[0] )
                (*this)[ tokens[0] ] = true;
        }
        // recreate args
        while( argc-- ) {
            (*this)[ std::to_string(argc) ] = std::string( argv[argc] );
        }
    }

    getopt( const std::vector<std::string> &args ) : super() {
        std::vector<const char *> argv;
        for( auto &it : args ) {
            argv.push_back( it.c_str() );
        }
        *this = getopt( argv.size(), argv.data() );
    }

    size_t size() const {
        unsigned i = 0;
        while( has(std::to_string(i)) ) ++i;
        return i;
    }

    bool has( const std::string &op ) const {
        return this->find(op) != this->end();
    }

    std::string str() const {
        std::stringstream ss;
        std::string sep;
        for( auto &it : *this ) {
            ss << sep << it.first << "=" << it.second;
            sep = ',';
        }
        return ss.str();
    }

    std::string cmdline() const {
        std::stringstream cmd;
        std::string sep;
        // concatenate args
        for( auto end = size(), arg = end - end; arg < end; ++arg ) {
            cmd << sep << this->find(std::to_string(arg))->second;
            sep = ' ';
        }
        return cmd.str();
    }
};

// variadic syntax sugars {

template< typename T >
inline T getarg( const T &defaults, const char *argv ) {
    static struct getopt map( getopt_utils::cmdline() );
    return map.has( argv ) ? getopt_utils::as<T>(map[ argv ]) : defaults;
}

template< typename T, typename... Args >
inline T getarg( const T &defaults, const char *arg0, Args... argv ) {
    T t = getarg<T>( defaults, arg0 );
    return t == defaults ? getarg<T>( defaults, argv... ) : t;
}

inline const char * getarg( const char *defaults, const char *argv ) {
    static struct getopt map( getopt_utils::cmdline() );
    return map.has( argv ) ? getopt_utils::as<const char *>(map[ argv ]) : defaults;
}

template< typename... Args >
inline const char * getarg( const char *defaults, const char *arg0, Args... argv ) {
    const char *t = getarg( defaults, arg0 );
    return t == defaults ? getarg( defaults, argv... ) : t;
}

// }


#ifdef GETOPT_BUILD_DEMO
#include <iostream>
#include <stdlib.h>

int main( int argc, const char **argv ) {

    auto show_help = [&]() {
        std::cout << argv[0] << " [-h|--help|-?] [-f=path|--file=path] [-v|--version] [-d=number|--depth=number|--max-depth=number]" << std::endl;
        exit(0);
    };

    // Simple functional api. No initialization required.

    bool help = getarg( false, "-h", "--help", "-?" );
    int version = getarg( 0, "-v", "--version", "--show-version" );
    int depth = getarg( 0, "-d", "--depth", "--max-depth");
    std::string file = getarg( "", "-f", "--file" );

    if( help || argc <= 1 ) {
        show_help();
    }

    if( version ) {
        std::cout << argv[0] << " demo v1.0.0. Compiled on " << __DATE__ << std::endl;
    }

    if( depth ) {
        std::cout << "provided depth: " << depth << std::endl;
    }

    if( !file.empty() ) {
        std::cout << "provided file: " << file << std::endl;
    }

    // OOP map-based api. Explicit (argc, argv) initialization required.

    struct getopt args( argc, argv );

    if( args.has("-h") || args.has("--help") || args.has("-?") || args.size() == 1 ) {
        show_help();
    }

    if( args.has("-v") || args.has("--version") ) {
        std::cout << args["0"] << " demo v1.0.0. Compiled on " << __DATE__ << std::endl;
    }

    if( args.has("-d") || args.has("--depth") || args.has("--max-depth") ) {
        std::string arg = args["-d"];
        if( arg.empty() ) arg = args["--depth"];
        if( arg.empty() ) arg = args["--max-depth"];
        int depth = atoi( arg.c_str() );
        std::cout << "provided depth: " << depth << std::endl;
    }

    if( args.has("-f") || args.has("--file") ) {
        std::string arg = args["-f"];
        if( arg.empty() ) arg = args["--file"];
        std::string fname = arg;
        std::cout << "provided file: " << fname << std::endl;
    }

    std::cout << "---" << std::endl;
    std::cout << args.cmdline() << std::endl;
    //std::cout << args.size() << " provided args: " << args.str() << std::endl;
}
#endif
