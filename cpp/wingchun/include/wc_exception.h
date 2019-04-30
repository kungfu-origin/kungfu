//
// Created by PolarAir on 2019-02-21.
//

#ifndef KUNGFU_WC_EXCEPTION_H
#define KUNGFU_WC_EXCEPTION_H

#include <stdexcept>
#include <string>
namespace kungfu
{
    class WCException : public std::runtime_error
    {
    public:
        explicit WCException (const std::string& __s) : std::runtime_error(__s) {}
        explicit WCException (const char* __s)   : std::runtime_error(__s) {}

        virtual ~WCException() noexcept {};
    };
}

#endif //KUNGFU_WC_EXCEPTION_H
