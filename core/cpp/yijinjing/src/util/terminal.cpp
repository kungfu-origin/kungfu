//
// Created by Keren Dong on 2019-06-10.
//

#include <spdlog/spdlog.h>
#include <spdlog/details/util.h>
#include <spdlog/details/console_globals.h>

#include <kungfu/yijinjing/util/util.h>

namespace kungfu {

    namespace yijinjing {

        namespace util {

#ifdef _WINDOWS
            #include <string>
#include <unordered_map>
#include <wincon.h>
class WinColor
{
public:
    const WORD BOLD = FOREGROUND_INTENSITY;
    const WORD RED = FOREGROUND_RED;
    const WORD GREEN = FOREGROUND_GREEN;
    const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
    WinColor()
    {
        colors_["trace"] = WHITE;
        colors_["debug"] = CYAN;
        colors_["info"] = GREEN;
        colors_["warn"] = YELLOW | BOLD;
        colors_["warning"] = YELLOW | BOLD;
        colors_["error"] = RED | BOLD;
        colors_["critical"] = BACKGROUND_RED | WHITE | BOLD;
        colors_["off"] = 0;
    }
    void print(std::string level, std::string log)
    {
        HANDLE out_handle_ = spdlog::details::console_stdout::handle();
        WORD color_attribs = colors_[level];
        CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
        ::GetConsoleScreenBufferInfo(out_handle_, &orig_buffer_info);
        WORD back_color = orig_buffer_info.wAttributes;
        // retrieve the current background color
        back_color &= static_cast<WORD>(~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY));
        // keep the background color unchanged
        ::SetConsoleTextAttribute(out_handle_, color_attribs | back_color);
        auto start = log.c_str();
        auto end = log.c_str() + log.length();
        auto size = static_cast<DWORD>(end - start);
        ::WriteFile(out_handle_, start, size, nullptr, nullptr);
        ::SetConsoleTextAttribute(out_handle_, orig_buffer_info.wAttributes); // reset to orig colors
    }
private:
    std::unordered_map<std::string, WORD> colors_;
};
WinColor color_printer;
void color_print(std::string level, std::string log)
{
    color_printer.print(level, log);
}
#else

            void color_print(std::string level, std::string log)
            {}

#endif

            bool in_color_terminal()
            {
                return spdlog::details::os::in_terminal(spdlog::details::console_stdout::stream()) && spdlog::details::os::is_color_terminal();
            }

        }
    }
}
