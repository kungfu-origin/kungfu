/*******************************************************************************************
Copyright 2011, T3 IP, LLC. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY T3 IP, LLC ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL T3 IP, LLC OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of T3 IP, LLC.
*******************************************************************************************/

/*!
 * \file
 * \brief The High Frequency FIX Parser Library.
 * Repository at http://github.com/jamesdbrock/hffix
 */

#ifndef HFFIX_HPP
#define HFFIX_HPP

#include <hffix_fields.hpp> // for field and message tag names and properties. Needed for length_fields[].
#include <cstring>          // for memcpy
#include <string>           //
#include <algorithm>        // for is_tag_a_data_length
#include <numeric>          // for accumulate
#include <iostream>         // for operator<<()
#include <limits>           // for numeric_limits<>::is_signed
#include <stdexcept>        // for exceptions
#if __cplusplus >= 201703L
#include <string_view>      // for push_back_string()
#endif
#if __cplusplus >= 201103L
#include <cstdint>          // for std::uint8_t
#include <chrono>
#endif

#ifndef HFFIX_NO_BOOST_DATETIME
#ifdef DATE_TIME_TIME_HPP___ // The header include guard from boost/date_time/time.hpp
#ifdef DATE_TIME_DATE_HPP___ // The header include guard from boost/date_time/date.hpp
#define HFFIX_BOOST_DATETIME
#endif
#endif
#endif

/*!
\brief Namespace for all types and functions of High Frequency FIX Parser.
*/
namespace hffix {

/* @cond EXCLUDE */

namespace details {

inline void throw_range_error() {
    throw std::out_of_range("hffix message_writer buffer full");
}

template <std::size_t N>
std::ptrdiff_t len(char const (&)[N]) { return std::ptrdiff_t(N - 1); }

/*
\brief Internal ascii-to-integer conversion.

Parses ascii and returns a (possibly negative) integer.

\tparam Int_type The type of integer to be returned.
\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\return The ascii string represented as an integer of type Int_type.
*/
template<typename Int_type> Int_type atoi(char const* begin, char const* end)
{
    Int_type val(0);
    bool isnegative(false);

    if (begin < end && *begin == '-') {
        isnegative = true;
        ++begin;
    }

    for(; begin<end; ++begin) {
        val *= 10;
        val += (Int_type)(*begin - '0');
    }

    return isnegative ? -val : val;
}


/*
\brief Internal ascii-to-unsigned-integer conversion.

Parses ascii and returns an unsigned integer.

\tparam Uint_type The type of unsigned integer to be returned.
\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\return The ascii string represented as an unsigned integer of type Uint_type.
*/
template<typename Uint_type> Uint_type atou(char const* begin, char const* end)
{
    Uint_type val(0);

    for(; begin<end; ++begin) {
        val *= 10u;
        val += (Uint_type)(*begin - '0');
    }

    return val;
}


/*
\brief Internal integer-to-ascii conversion.

Writes an integer out as ascii.

\tparam Int_type Type of integer to be converted.
\param number Value of the integer to be converted.
\param buffer Pointer to location for the ascii to be written.
\param end Past-the-end of the buffer, to check for overflow.
\return Pointer to past-the-end of the ascii that was written.
*/
template<typename Int_type> char* itoa(Int_type number, char* buffer, char* end)
{
    // Write out the digits in reverse order.
    bool isnegative(false);
    if (number < 0) {
        isnegative = true;
        number = -number;
    }

    char*b = buffer;
    do {
        if (b >= end) details::throw_range_error();
        *b++ = '0' + (number % 10);
        number /= 10;
    } while(number);

    if (isnegative) {
        if (b >= end) details::throw_range_error();
        *b++ = '-';
    }

    // Reverse the digits in-place.
    std::reverse(buffer, b);

    return b;
}


/*
\brief Internal unsigned-integer-to-ascii conversion.

Writes an unsigned integer out as ascii.

\tparam Int_type Type of integer to be converted.
\param number Value of the integer to be converted.
\param buffer Pointer to location for the ascii to be written.
\param end Past-the-end of the buffer, to check for overflow.
\return Pointer to past-the-end of the ascii that was written.
*/
template<typename Uint_type> char* utoa(Uint_type number, char* buffer, char* end)
{
    // Write out the digits in reverse order.
    char*b = buffer;
    do {
        if (b >= end) details::throw_range_error();
        *b++ = '0' + (number % 10);
        number /= 10;
    } while(number);

    // Reverse the digits in-place.
    std::reverse(buffer, b);

    return b;
}

/*
\brief Internal ascii-to-decimal conversion.

Converts an ascii string to a decimal float, of the form \htmlonly mantissa&times;10<sup>exponent</sup>\endhtmlonly.

Non-normalized. The exponent will always be less than or equal to zero.
If the decimal float is an integer, the exponent will be zero.

\tparam Int_type Signed integer type for the mantissa and exponent.
\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param mantissa Reference to storage for the mantissa of the decimal float to be returned.
\param exponent Reference to storage for the exponent of the decimal float to be returned.
*/
template<typename Int_type> void atod(char const* begin, char const* end, Int_type& mantissa, Int_type& exponent)
{
    Int_type mantissa_ = 0;
    Int_type exponent_ = 0;
    bool isdecimal(false);
    bool isnegative(false);

    if (begin < end && *begin == '-') {
        isnegative = true;
        ++begin;
    }

    for(; begin < end; ++begin) {
        if (*begin == '.') {
            isdecimal = true;
        } else {
            mantissa_ *= 10;
            mantissa_ += (*begin - '0');
            if (isdecimal) --exponent_;
        }
    }

    if (isnegative) mantissa_ = -mantissa_;
    mantissa = mantissa_;
    exponent = exponent_;
}

/*
\brief Internal decimal-to-ascii conversion.

Converts a decimal float of the form \htmlonly mantissa&times;10<sup>exponent</sup>\endhtmlonly to ascii.

Non-normalized. The exponent parameter must be less than or equal to zero.

\tparam Int_type Integer type for the mantissa and exponent.
\param mantissa The mantissa of the decimal float.
\param exponent The exponent of the decimal float. Must be less than or equal to zero.
\param buffer Pointer to location for the ascii to be written.
\param end Past-the-end of the buffer, to check for overflow.
\return Pointer to past-the-end of the ascii that was written.
*/
template<typename Int_type> char* dtoa(Int_type mantissa, Int_type exponent, char* buffer, char* end)
{
    // Write out the digits in reverse order.
    bool isnegative(false);
    if (mantissa < 0) {
        isnegative = true;
        mantissa = -mantissa;
    }

    char*b = buffer;
    do {
        if (b >= end) details::throw_range_error();
        *b++ = '0' + (mantissa % 10);
        mantissa /= 10;
        if (++exponent == 0) {
            if (b >= end) details::throw_range_error();
            *b++ = '.';
        }
    } while(mantissa > 0 || exponent < 1);

    if (isnegative) {
        if (b >= end) details::throw_range_error();
        *b++ = '-';
    }

    // Reverse the digits in-place.
    std::reverse(buffer, b);

    return b;
}


/*
\brief Internal ascii-to-date conversion.

Parses ascii and returns a LocalMktDate or UTCDate.

\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param[out] year Year.
\param[out] month Month.
\param[out] day Day.
\return True if successful and the out arguments were set.
*/
inline bool atodate(
    char const* begin,
    char const* end,
    int& year,
    int& month,
    int& day
)
{
    if (end - begin != 8) return false;

    year = details::atoi<int>(begin, begin + 4);
    month = details::atoi<int>(begin + 4, begin + 6);
    day = details::atoi<int>(begin + 6, begin + 8);

    return true;
}


/*
\brief Internal ascii-to-time conversion.

Parses ascii and returns a time.

\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param[out] hour Hour.
\param[out] minute Minute.
\param[out] second Second.
\param[out] millisecond Millisecond.
\return True if successful and the out arguments were set.
*/
inline bool atotime(
    char const* begin,
    char const* end,
    int& hour,
    int& minute,
    int& second,
    int& millisecond
)
{
    if (end - begin != 8 && end - begin != 12) return false;

    hour = details::atoi<int>(begin, begin + 2);
    minute = details::atoi<int>(begin + 3, begin + 5);
    second = details::atoi<int>(begin + 6, begin + 8);

    if (end - begin == 12)
        millisecond = details::atoi<int>(begin + 9, begin + 12);
    else
        millisecond = 0;

    return true;
}

#if __cplusplus >= 201103L
/*
\brief SFINAE to check if type is std::chrono::time_point

\tparam Clock the clock on which this time point is measured
\tparam Duration a std::chrono::duration type used to measure the time since epoch
*/
template<typename T>
struct is_time_point : std::false_type {}; 

template<typename Clock, typename Duration>
struct is_time_point<std::chrono::time_point<Clock, Duration>> : std::true_type {}; 

/*
\brief Internal ascii-to-timepoint conversion.

Parses ascii and returns a std::chrono::time_point.

\param begin Pointer to the beginning of the ascii string.
\param end Pointer to past-the-end of the ascii string.
\param[out] tp time_point.
\return True if successful and the out arguments were set.
*/
template <typename TimePoint>
inline typename std::enable_if<details::is_time_point<TimePoint>::value, bool>::type
atotimepoint(
    char const* begin,
    char const* end,
    TimePoint& tp
)
{
    // TODO: after c++20 this simplifies to
    //       std::chrono::parse("%Y%m%d-%T", tp);
    int year, month, day, hour, minute, second, millisecond;
    if (!atotime(begin + 9, end, hour, minute, second, millisecond))
        return false;
    if (!atodate(begin, begin + 8, year, month, day))
        return false;

    // from http://howardhinnant.github.io/date_algorithms.html
    year -= month <= 2;
    const unsigned era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(year - era * 400);
    const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    const unsigned long days_since_epoch = era * 146097 + static_cast<unsigned>(doe) - 719468;

    tp = TimePoint(std::chrono::seconds(days_since_epoch * 24 * 3600) +
                   std::chrono::hours(hour) +
                   std::chrono::minutes(minute) +
                   std::chrono::seconds(second) +
                   std::chrono::milliseconds(millisecond));

    return true;
}

/*
\brief Internal ascii-to-timepoint conversion.

Parses ascii and returns a std::chrono::time_point.

\param tp TimePoint to parse.
\param[out] year Year.
\param[out] month Month.
\param[out] day Day.
\param[out] hour Hour.
\param[out] minute Minute.
\param[out] second Second.
\param[out] millisecond Millisecond.
*/
template <typename TimePoint>
inline typename std::enable_if<details::is_time_point<TimePoint>::value, void>::type
timepointtoparts(TimePoint tp, int& year, int& month, int& day,
                 int& hour, int& minute, int& second, int& millisecond) noexcept
{
    auto epoch_sec = std::chrono::time_point_cast<
        std::chrono::seconds>(tp).time_since_epoch().count();
    auto day_sec = epoch_sec - (epoch_sec % 86400);
    auto days_since_epoch = day_sec / 86400;

    // see http://howardhinnant.github.io/date_algorithms.html
    days_since_epoch += 719468;
    const unsigned era = (days_since_epoch >= 0 ? days_since_epoch : days_since_epoch - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(days_since_epoch - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    year = static_cast<unsigned>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    day = doy - (153 * mp + 2) / 5 + 1;
    month = mp + (mp < 10 ? 3 : -9);
    year += month <= 2;

    auto in_day = tp - std::chrono::seconds(day_sec);
    millisecond = std::chrono::time_point_cast<std::chrono::milliseconds>(
            in_day).time_since_epoch().count();
    hour = millisecond / (60 * 60 * 1000);
    millisecond -= hour * 60 * 60 * 1000;
    minute = millisecond / (60 * 1000);
    millisecond -= minute * 60 * 1000;
    second = millisecond / 1000;
    millisecond -= second * 1000;
}
#endif

} // namespace details

/* @endcond*/

/*!
 * \brief One FIX message for writing.
 *
 * Given a buffer, the message_writer will write a FIX message to the buffer. message_writer does not take ownership of the buffer.
 *
 * <h3>Usage</h3>
 *
 * The message_writer interface is patterned after
 * Back Insertion Sequence Containers, with overloads of `push_back` for different FIX field data types.
 *
 * The `push_back_header()` method will write the _BeginString_ and _BodyLength_ fields to the message,
 * and the FIX Standard Message Header requires also _MsgType_, _SenderCompID_, _TargetCompID_, _MsgSeqNum_ and _SendingTime_.
 * You must write those fields yourself, starting with _MsgType_.
 *
 * After calling all other `push_back` methods and before sending the message, you must call push_back_trailer(),
 * which will write the CheckSum field for you.
 *
 * <h3>Extension</h3>
 *
 * Keep in mind that if you don't like the way any of these `push_back` methods
 * perform serialization, then you can always do your own serialization for any
 * data type, and then append it to the message by `push_back_string()`.
 *
 * For example, these two statements are equivalent for `hffix::message_writer w`.
 *
 * \code
 * w.push_back_int   (hffix::tag::HeartBtInt, 12);
 * w.push_back_string(hffix::tag::HeartBtInt, "12");
 * \endcode
 *
 * For another example, if a trading peer has extended FIX for femtosecond timestamp precision, then a custom timestamp can be serialized to a string and then written to a field.
 *
 * \code
 * w.push_back_string(hffix::tag::SendingTime, "20180309-13:46:01.0123456789123456");
 * \endcode
 */
class message_writer {
public:

    /*!
    \brief Construct by buffer size.
    \param buffer Pointer to the buffer to be written to.
    \param size Size of the buffer in bytes.
    */
    message_writer(char* buffer, size_t size):
        buffer_(buffer),
        buffer_end_(buffer + size),
        next_(buffer),
        body_length_(NULL) {
    }

    /*!
    \brief Construct by buffer begin and end.
    \param begin Pointer to the buffer to be written to.
    \param end Pointer to past-the-end of the buffer to be written to.
    */
    message_writer(char* begin, char* end) :
        buffer_(begin),
        buffer_end_(end),
        next_(begin),
        body_length_(NULL) {
    }

    /*!
    \brief Construct on an array reference to a buffer.
    \tparam N The size of the array.
    \param buffer An array reference. The writer will write into the entire array of length _N_.
    */
    template<size_t N>
    message_writer(char(&buffer)[N]) :
        buffer_(buffer),
        buffer_end_(&(buffer[N])),
        next_(buffer),
        body_length_(NULL) {
    }


    /*!
     * \brief Owns no resources, so destruction is no-op.
     */
    ~message_writer() {}

    /*! \name Buffer Access */
    //@{

    /*!
    \brief Size of the message in bytes.
     *
     * \pre push_back_trailer() has been called.
    */
    size_t message_size() const {
        return next_ - buffer_;
    }

    /*!
     * \brief Pointer to beginning of the message.
     *
     * \pre None.
     */
    char* message_begin() const {
        return buffer_;
    }
    /*!
     * \brief Pointer to past-the-end of the message.
     *
     * \pre push_back_trailer() has been called.
     */
    char* message_end() const {
        return next_;
    }

    /*!
     * \brief Total available buffer size, including buffer already written to by this message.
     */
    size_t buffer_size() const {
        return buffer_end_ - buffer_;
    }

    /*!
     * \brief Remaining available buffer size. Excludes buffer already written to by this message.
     */
    size_t buffer_size_remaining() const {
        return buffer_end_ - next_;
    }

    //@}

    /*! \name Transport Fields */
    //@{

    /*!
     * \brief Write the _BeginString_ and _BodyLength_ fields to the buffer.
     *
     * This method must be called before any other `push_back` method. It may only be called once for each message_writer.
     *
     * \pre No other `push_back` method has yet been called.
     * \param begin_string_version The value for the BeginString FIX field. Should probably be "FIX.4.2" or "FIX.4.3" or "FIX.4.4" or "FIXT.1.1" (for FIX 5.0).
     *
     * \throw std::out_of_range When the remaining buffer size is too small.
     * \throw std::logic_error When called more than once for a single message.
     */
    void push_back_header(char const* begin_string_version) {
        if (body_length_) throw std::logic_error("hffix message_writer.push_back_header called twice");
        if (buffer_end_ - next_ < 2 + std::ptrdiff_t(strlen(begin_string_version)) + 3 + 7) {
            details::throw_range_error();
        }
        memcpy(next_, "8=", 2);
        next_ += 2;
        memcpy(next_, begin_string_version, std::strlen(begin_string_version));
        next_ += std::strlen(begin_string_version);
        *(next_++) = '\x01';
        memcpy(next_, "9=", 2);
        next_ += 2;
        body_length_ = next_;
        next_ += 6; // 6 characters reserved for BodyLength.
        *next_++ = '\x01';
    }


    /*!
     * \brief Write the _CheckSum_ field to the buffer.
     *
     * This function must be called after all other `push_back` functions. It may only be called once for each message_writer.
     *
     * \pre push_back_header() method has been called.
     *
     * \post There is a complete and valid FIX message in the buffer.
     *
     * \param calculate_checksum If this flag is set to false, then instead of iterating over the entire message and
     * calculating the CheckSum, the standard trailer will simply write CheckSum=000. This is fine if you're sending
     * the message to a FIX parser that, like High Frequency FIX Parser, doesn't care about the CheckSum.
     *
     * \throw std::out_of_range When the remaining buffer size is too small.
     * \throw std::logic_error When called before message_writer::push_back_header()
     */
    void push_back_trailer(bool calculate_checksum = true) {
        // Calculate and write out the BodyLength.
        // BodyLength does not include the SOH character after the BodyLength field.
        // BodyLength does not include the SOH character before the CheckSum field.
        if (!body_length_) {
            throw std::logic_error("hffix message_writer.push_back_trailer called before message_writer.push_back_header");
        }

        size_t const len = next_ - (body_length_ + 7);
        body_length_[0] = '0' + (len / 100000) % 10;
        body_length_[1] = '0' + (len / 10000) % 10;
        body_length_[2] = '0' + (len / 1000) % 10;
        body_length_[3] = '0' + (len / 100) % 10;
        body_length_[4] = '0' + (len / 10) % 10;
        body_length_[5] = '0' + len % 10;

        if (buffer_end_ - next_ < 7) {
            details::throw_range_error();
        }

        // write out the CheckSum after optionally calculating it
        if (calculate_checksum) {
#if __cplusplus >= 201103L
            using std::uint8_t;
#else
            typedef unsigned char uint8_t;
#endif
            uint8_t const checksum = std::accumulate(buffer_, next_, uint8_t(0));

            memcpy(next_, "10=", 3);
            next_ += 3;
            next_[0] = '0' + ((checksum / 100) % 10);
            next_[1] = '0' + ((checksum / 10) % 10);
            next_[2] = '0' + (checksum % 10);

            next_ += 3;
            *next_++ = '\x01';
        } else {
            memcpy(next_, "10=000\x01", 7);
            next_ += 7;
        }

    }

    //@}

    /*! \name String Fields */
    //@{

    /*!
    \brief Append a string field to the message.

    \param tag FIX tag.
    \param begin Pointer to the beginning of the string.
    \param end Pointer to past-the-end of the string.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_string(int tag, char const* begin, char const* end) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < (end - begin) + 2) {
            details::throw_range_error();
        }
        *next_++ = '=';
        memcpy(next_, begin, end - begin);
        next_ += (end - begin);
        *next_++ = '\x01';
    }

    /*!
    \brief Append a string field to the message.

    \param tag FIX tag.
    \param cstring Pointer to the beginning of a C-style null-terminated string.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_string(int tag, char const* cstring) {
        // Find the end of the cstring, like strlen, but throw if the cstring
        // is longer than the remaining buffer.
        char const* cstring_end = (char const*)memchr(cstring, 0, buffer_end_ - next_);
        if (cstring_end) push_back_string(tag, cstring, cstring_end);
        else details::throw_range_error();
    }

    /*!
    \brief Append a string field to the message.

    The entire, literal contents of s will be copied to the output buffer, so if you are using std::wstring
    you may need to first convert from UTF-32 to UTF-8, or do some other encoding transformation.

    \param tag FIX tag.
    \param s String.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_string(int tag, std::string const& s) {
        push_back_string(tag, s.data(), s.data() + s.size());
    }


#if __cplusplus >= 201703L
    /*!
    \brief Append a string field to the message.

    The range of s will be copied to the output buffer.

    \param tag FIX tag.
    \param s String.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_string(int tag, std::string_view s) {
        push_back_string(tag, s.begin(), s.end());
    }
#endif

    /*!
    \brief Append a char field to the message.

    \param tag FIX tag.
    \param character An ascii character.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_char(int tag, char character) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < 3) {
            details::throw_range_error();
        }
        *next_++ = '=';
        *next_++ = character;
        *next_++ = '\x01';
    }
//@}


    /*! \name Integer Fields */
//@{
    /*!
    \brief Append an integer field to the message.

    \tparam Int_type Type of integer.
    \param tag FIX tag.
    \param number Integer value.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    template<typename Int_type> void push_back_int(int tag, Int_type number) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (next_ >= buffer_end_) details::throw_range_error();
        *next_++ = '=';
        next_ = details::itoa(number, next_, buffer_end_);
        if (next_ >= buffer_end_) details::throw_range_error();
        *next_++ = '\x01';
    }

//@}

    /*! \name Decimal Float Fields */
//@{

    /*!
    \brief Append a decimal float field to the message.

    The decimal float is of the form \htmlonly mantissa&times;10<sup>exponent</sup>\endhtmlonly.

    Non-normalized. The exponent parameter must be less than or equal to zero. If the exponent
    parameter is zero, no decimal point will be written to the ascii field.

    \tparam Int_type Integer type for the mantissa and exponent.
    \param tag FIX tag.
    \param mantissa The mantissa of the decimal float.
    \param exponent The exponent of the decimal float. Must be less than or equal to zero.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    template<typename Int_type> void push_back_decimal(int tag, Int_type mantissa, Int_type exponent) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (next_ >= buffer_end_) details::throw_range_error();
        *next_++ = '=';
        next_ = details::dtoa(mantissa, exponent, next_, buffer_end_);
        if (next_ >= buffer_end_) details::throw_range_error();
        *next_++ = '\x01';
    }
//@}


    /*! \name Date and Time Fields */
//@{

    /*!
    \brief Append a LocalMktDate or UTCDate field to the message.

    \param tag FIX tag.
    \param year Year.
    \param month Month.
    \param day Day.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_date(int tag, int year, int month, int day) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < details::len("=YYYYMMDD|")) {
            details::throw_range_error();
        }
        *next_++ = '=';
        itoa_padded(year, next_, next_ + 4);
        next_ += 4;
        itoa_padded(month, next_, next_ + 2);
        next_ += 2;
        itoa_padded(day, next_, next_ + 2);
        next_ += 2;
        *next_++ = '\x01';
    }
    /*!
    \brief Append a month-year field to the message.

    \param tag FIX tag.
    \param year Year.
    \param month Month.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_monthyear(int tag, int year, int month) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < details::len("=YYYYMM|")) {
            details::throw_range_error();
        }
        *next_++ = '=';
        itoa_padded(year, next_, next_ + 4);
        next_ += 4;
        itoa_padded(month, next_, next_ + 2);
        next_ += 2;
        *next_++ = '\x01';
    }

    /*!
    \brief Append a UTCTimeOnly field to the message.

    No time zone or daylight savings time transformations are done to the time.

    No fractional seconds are written to the field.

    \param tag FIX tag.
    \param hour Hour.
    \param minute Minute.
    \param second Second.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_timeonly(int tag, int hour, int minute, int second) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < details::len("=HH:MM:SS|")) {
            details::throw_range_error();
        }
        *next_++ = '=';
        itoa_padded(hour, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(minute, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(second, next_, next_ + 2);
        next_ += 2;
        *next_++ = '\x01';
    }

    /*!
    \brief Append a UTCTimeOnly field to the message.

    No time zone or daylight savings time transformations are done to the time.

    \param tag FIX tag.
    \param hour Hour.
    \param minute Minute.
    \param second Second.
    \param millisecond Millisecond.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_timeonly(int tag, int hour, int minute, int second, int millisecond) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < details::len("=HH:MM:SS.sss|")) {
            details::throw_range_error();
        }
        *next_++ = '=';
        itoa_padded(hour, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(minute, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(second, next_, next_ + 2);
        next_ += 2;
        *next_++ = '.';
        itoa_padded(millisecond, next_, next_ + 3);
        next_ += 3;
        *next_++ = '\x01';
    }

    /*!
    \brief Append a UTCTimestamp field to the message.

    No time zone or daylight savings time transformations are done to the timestamp.

    No fractional seconds are written to the field.

    \param tag FIX tag.
    \param year Year.
    \param month Month.
    \param day Day.
    \param hour Hour.
    \param minute Minute.
    \param second Second.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_timestamp(int tag, int year, int month, int day, int hour, int minute, int second) {
        next_ = details::itoa(tag, next_, buffer_end_);

        if (buffer_end_ - next_ < details::len("=YYYYMMDD-HH:MM:SS|")) {
            details::throw_range_error();
        }
        *next_++ = '=';
        itoa_padded(year, next_, next_ + 4);
        next_ += 4;
        itoa_padded(month, next_, next_ + 2);
        next_ += 2;
        itoa_padded(day, next_, next_ + 2);
        next_ += 2;
        *next_++ = '-';
        itoa_padded(hour, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(minute, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(second, next_, next_ + 2);
        next_ += 2;
        *next_++ = '\x01';
    }

    /*!
    \brief Append a UTCTimestamp field to the message.

    No time zone or daylight savings time transformations are done to the timestamp.

    \param tag FIX tag.
    \param year Year.
    \param month Month.
    \param day Day.
    \param hour Hour.
    \param minute Minute.
    \param second Second.
    \param millisecond Millisecond.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_timestamp(int tag, int year, int month, int day, int hour, int minute, int second, int millisecond) {
        next_ = details::itoa(tag, next_, buffer_end_);
        if (buffer_end_ - next_ < details::len("=YYYYMMDD-HH:MM:SS.sss|")) {
            details::throw_range_error();
        }
        *next_++ = '=';
        itoa_padded(year, next_, next_ + 4);
        next_ += 4;
        itoa_padded(month, next_, next_ + 2);
        next_ += 2;
        itoa_padded(day, next_, next_ + 2);
        next_ += 2;
        *next_++ = '-';
        itoa_padded(hour, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(minute, next_, next_ + 2);
        next_ += 2;
        *next_++ = ':';
        itoa_padded(second, next_, next_ + 2);
        next_ += 2;
        *next_++ = '.';
        itoa_padded(millisecond, next_, next_ + 3);
        next_ += 3;
        *next_++ = '\x01';
    }

//@}

#ifdef HFFIX_BOOST_DATETIME

    /*! \name Boost Date and Time Fields */
//@{

    /*!
    \brief Append a LocalMktDate or UTCDate field to the message.

    \param tag FIX tag.
    \param date Date.

    \throw std::out_of_range When the remaining buffer size is too small.

    \see HFFIX_NO_BOOST_DATETIME
    */
    void push_back_date(int tag, boost::gregorian::date date) {
        if (!date.is_not_a_date())
            push_back_date(tag, date.year(), date.month(), date.day());
    }

    /*!
    \brief Append a UTCTimeOnly field to the message.

    No time zone or daylight savings time transformations are done to the time.

    Fractional seconds will be written to the field, rounded to the millisecond.

    \param tag FIX tag.
    \param timeonly Time.

    \throw std::out_of_range When the remaining buffer size is too small.

    \see HFFIX_NO_BOOST_DATETIME
    */
    void push_back_timeonly(int tag, boost::posix_time::time_duration timeonly) {
        if (!timeonly.is_not_a_date_time())
            push_back_timeonly(
                tag,
                timeonly.hours(),
                timeonly.minutes(),
                timeonly.seconds(),
                int(timeonly.fractional_seconds() * 1000 / boost::posix_time::time_duration::ticks_per_second())
            );
    }

    /*!
    \brief Append a UTCTimestamp field to the message.

    No time zone or daylight savings time transformations are done to the timestamp.

    Fractional seconds will be written to the field, rounded to the millisecond.

    \param tag FIX tag.
    \param timestamp Date and time.

    \throw std::out_of_range When the remaining buffer size is too small.

    \see HFFIX_NO_BOOST_DATETIME
    */
    void push_back_timestamp(int tag, boost::posix_time::ptime timestamp) {
        if (!timestamp.is_not_a_date_time())
            push_back_timestamp(
                tag,
                timestamp.date().year(),
                timestamp.date().month(),
                timestamp.date().day(),
                timestamp.time_of_day().hours(),
                timestamp.time_of_day().minutes(),
                timestamp.time_of_day().seconds(),
                int(timestamp.time_of_day().fractional_seconds() * 1000 / boost::posix_time::time_duration::ticks_per_second())
            );
        else
            throw std::logic_error("push_back_timestamp called with not_a_date_time.");
    }
//@}
#endif // HFFIX_BOOST_DATETIME

#if __cplusplus >= 201103L
    /*! \name std::chrono Date and Time Fields */
//@{

    /*!
    \brief Append a `std::chrono::time_point` field to the message.

    Fractional seconds will be written to the field, rounded to the millisecond.

    Uses algorithms from http://howardhinnant.github.io/date_algorithms.html ,
    which implement a proleptic Gregorian calendar. This will probably be
    superseded by C++20.

    \param tag FIX tag.
    \param tp `std::chrono::time_point`.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    template<typename Clock, typename Duration>
    void push_back_timestamp(int tag, std::chrono::time_point<Clock,Duration> tp) {
        // TODO: with c++20, we can use std::chrono::format
        int year, month, day, hour, minute, second, millisecond;
        details::timepointtoparts(tp, year, month, day, hour, minute, second, millisecond);
        push_back_timestamp(tag, year, month, day, hour, minute, second, millisecond);
    }

    /*!
    \brief Append a UTCTimeOnly field to the message.

    No time zone or daylight savings time transformations are done to the time.

    Fractional seconds will be written to the field, rounded to the millisecond.

    \param tag FIX tag.
    \param timeonly Time.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    template<typename Rep, typename Period>
    void push_back_timeonly(int tag, std::chrono::duration<Rep,Period> timeonly) {
        using namespace std::chrono;

        push_back_timeonly(
            tag,
            duration_cast<hours>       (timeonly).count(),
            duration_cast<minutes>     (timeonly %   hours(1)).count(),
            duration_cast<seconds>     (timeonly % minutes(1)).count(),
            duration_cast<milliseconds>(timeonly % seconds(1)).count()
            );
    }

//@}
#endif


    /*! \name Data Fields */
//@{

    /*!
    \brief Append a data length field and a data field to the message.

    Note that this method will append two fields to the message. The first field is an integer equal to
    the content length of the second field. FIX does this so that the content of the second field may contain
    Ascii NULL or SOH or other control characters.

    High Frequency FIX Parser calculates the content length for you
    and writes out both fields, you just have to provide both tags and pointers to the data.
    For most of the data fields in FIX, it is true that tag_data = tag_data_length + 1, but we daren't assume that.

    Example:
    \code
    hffix::message_writer r;
    std::string data("Some data.");
    r.push_back_data(hffix::tag::RawDataLength, hffix::tag::RawData, data.begin(), data.end());
    \endcode

    \param tag_data_length FIX tag for the data length field.
    \param tag_data FIX tag for the data field.
    \param begin Pointer to the beginning of the data.
    \param end Pointer to after-the-end of the data.

    \throw std::out_of_range When the remaining buffer size is too small.
    */
    void push_back_data(int tag_data_length, int tag_data, char const* begin, char const* end) {
        next_ = details::itoa(tag_data_length, next_, buffer_end_);
        if (next_ == buffer_end_) details::throw_range_error();
        *next_++ = '=';
        next_ = details::itoa(end - begin, next_, buffer_end_);
        if (next_ == buffer_end_) details::throw_range_error();
        *next_++ = '\x01';
        next_ = details::itoa(tag_data, next_, buffer_end_);

        if (buffer_end_ - next_ < (end - begin) + 2) {
            details::throw_range_error();
        }
        *next_++ = '=';
        memcpy(next_, begin, end - begin);
        next_ += end - begin;
        *next_++ = '\x01';
    }


//@}
private:
    static void itoa_padded(int x, char* b, char* e) {
        while (e > b) {
            *--e = '0' + (x % 10);
            x /= 10;
        }
    }

    char* buffer_;
    char* buffer_end_;
    char* next_;
    char* body_length_; // Pointer to the location at which the BodyLength should be written, once the length of the message is known. 6 chars, which allows for messagelength up to 999,999.
};

class message_reader;
class message_reader_const_iterator;

/*!
 * \brief FIX field value for hffix::message_reader.
 *
 * <h3>Usage</h3>
 *
 * This class is a range `begin(),end()` of pointers into
 * a `message_reader` buffer which delimit the value for one field.
 *
 * FIX field values are an array of chars, and are usually ASCII.
 * Type conversion deserialization is provided by the `as_` family
 * of methods.
 *
 * <h3>Extension</h3>
 *
 * Keep in mind that if you don't like the way any of the the `as_` methods
 * perform deserialization for a type, then you can deserialize the field value
 * yourself, by reading the string delimited by `begin(),end()`.
 *
 * For example, these two statements should be equivalent for a `field_value v`:
 *
 * \code
 * int i = v.as_int();
 * int i = boost::lexical_cast<int>(v.begin(), v.size());
 * \endcode
*/
class field_value {
public:

    /*! \brief Pointer to the beginning of the field value in the buffer. */
    char const* begin() const {
        return begin_;
    }

    /*! \brief Pointer to past-the-end of the field value in the buffer. */
    char const* end() const {
        return end_;
    }

    /*! \brief Size of the field value, in bytes. */
    size_t size() const {
        return end_ - begin_;
    }


    /*!
    \brief True if the value of the field is equal to the C-string argument.
    */
    inline friend bool operator==(field_value const& that, char const* cstring) {
        return !strncmp(that.begin(), cstring, that.size()) && !cstring[that.size()];
        // TODO Is this correct? Maybe getting too fancy here trying to avoid call to strlen.
    }

    /*!
    \brief True if the value of the field is equal to the C-string argument.
    */
    inline friend bool operator==(char const* cstring, field_value const& that) {
        return that == cstring;
    }

    /*!
    \brief True if the value of the field is not equal to the C-string argument.
    */
    inline friend bool operator!=(field_value const& that, char const* cstring) {
        return !(that == cstring);
    }

    /*!
    \brief True if the value of the field is not equal to the C-string argument.
    */
    inline friend bool operator!=(char const* cstring, field_value const& that) {
        return !(that == cstring);
    }

    /*!
    \brief True if the value of the field is equal to the string argument.
    */
    inline friend bool operator==(field_value const& that, std::string const& s) {
        return that.size() == s.size() && !strncmp(that.begin(), s.data(), that.size());
    }

    /*!
    \brief True if the value of the field is equal to the string argument.
    */
    inline friend bool operator==(std::string const& s, field_value const& that) {
        return that == s;
    }

    /*!
    \brief True if the value of the field is not equal to the string argument.
    */
    inline friend bool operator!=(field_value const& that, std::string const& s) {
        return !(that == s);
    }

    /*!
    \brief True if the value of the field is not equal to the string argument.
    */
    inline friend bool operator!=(std::string const& s, field_value const& that) {
        return !(that == s);
    }

#if __cplusplus >= 201703L
    /*!
    \brief True if the value of the field is equal to the string_view argument.
    */
    inline friend bool operator==(field_value const& that, std::string_view s) {
        return std::equal(that.begin(), that.end(), s.begin(), s.end());
    }

    /*!
    \brief True if the value of the field is equal to the string_view argument.
    */
    inline friend bool operator==(std::string_view s, field_value const& that) {
        return that == s;
    }

    /*!
    \brief True if the value of the field is not equal to the string_view argument.
    */
    inline friend bool operator!=(field_value const& that, std::string_view s) {
        return !(that == s);
    }

    /*!
    \brief True if the value of the field is not equal to the string_view argument.
    */
    inline friend bool operator!=(std::string_view s, field_value const& that) {
        return !(that == s);
    }
#endif

    /*!
     * \brief Stream out the raw text value of the field.
     */
    friend std::ostream& operator<<(std::ostream& os, field_value const& that) {
        return os.write(that.begin(), that.size());
    }

    /*! \name String Conversion Methods */
//@{

    /*!
     * \brief Ascii value as std::string.
     *
     * \warning This function will, of course, allocate memory if the string is larger than the short-string-optimization size. This is the only function in this library which may allocate memory on the free store. Instead of using this function, consider reading the field value with `begin()` and `end()`.
     *
     * \return An std::string that contains a copy of the ascii value of the field.
     * \throw std::bad_alloc
    */
    std::string as_string() const {
        return std::string(begin(), end());
    }

    /*!
     * \brief Ascii value as char.
     *
     * \return The first char of the ascii value of the field.
     */
    char as_char() const {
        return *begin();
    }

//@}


    /*! \name Decimal Float Conversion Methods */
//@{

    /*!
    \brief Ascii-to-decimal conversion.

    Converts an ascii string to a decimal float, of the form \htmlonly mantissa&times;10<sup>exponent</sup>\endhtmlonly.

    Non-normalized. The exponent will always be less than or equal to zero.
    If the decimal float is an integer, the exponent will be zero.

    \tparam Int_type Integer type for the mantissa and exponent.
    \param[out] mantissa Reference to storage for the mantissa of the decimal float to be returned.
    \param[out] exponent Reference to storage for the exponent of the decimal float to be returned.
    */
    template<typename Int_type> void as_decimal(Int_type& mantissa, Int_type& exponent) const {
        details::atod<Int_type>(begin(), end(), mantissa, exponent);
    }
//@}

private:
    template <typename Int_type, bool Is_signed_integer>
    struct as_int_selector {
    };

    template <typename Int_type>
    struct as_int_selector<Int_type, true> {
        static Int_type call_as_int(char const* begin, char const* end) {
            return details::atoi<Int_type>(begin, end);
        }
    };

    template <typename Int_type>
    struct as_int_selector<Int_type, false> {
        static Int_type call_as_int(char const* begin, char const* end) {
            return details::atou<Int_type>(begin, end);
        }
    };

public:

    /*! \name Integer Conversion Methods */
//@{

    /*!
    \brief Ascii-to-integer conversion.

    Parses ascii and returns an integer.

    \tparam Int_type The type of integer to be returned. May be an unsigned integer.
    \return The ascii field value represented as an integer of type Int_type.
    */
    template<typename Int_type> Int_type as_int() const {
        return as_int_selector<Int_type, std::numeric_limits<Int_type>::is_signed>::call_as_int(begin(), end());
    }
//@}

    /*! \name Date and Time Conversion Methods */
//@{


    /*!
    \brief Ascii-to-date conversion.

    Parses ascii and returns a LocalMktDate or UTCDate.

    \param[out] year Year.
    \param[out] month Month.
    \param[out] day Day.
    \return True if successful and the out arguments were set.
    */
    bool as_date(
        int& year,
        int& month,
        int& day
    ) const {
        return details::atodate(begin(), end(), year, month, day);
    }


    /*!
    \brief Ascii-to-month-year conversion.

    Parses ascii and returns a month-year.

    \param[out] year Year.
    \param[out] month Month.
    \return True if successful and the out arguments were set.
    */
    bool as_monthyear(
        int& year,
        int& month
    ) const {
        if (end() - begin() != 6) return false;

        year = details::atoi<int>(begin(), begin() + 4);
        month = details::atoi<int>(begin() + 4, begin() + 6);

        return true;
    }


    /*!
    \brief Ascii-to-time conversion.

    Parses ascii and returns a time.

    \param[out] hour Hour.
    \param[out] minute Minute.
    \param[out] second Second.
    \param[out] millisecond Millisecond.
    \return True if successful and the out arguments were set.
    */
    bool as_timeonly(
        int& hour,
        int& minute,
        int& second,
        int& millisecond
    ) const {
        return details::atotime(begin(), end(), hour, minute, second, millisecond);
    }

    /*!
    \brief Ascii-to-timestamp conversion.

    Parses ascii and returns a timestamp.

    \param[out] year Year.
    \param[out] month Month.
    \param[out] day Day.
    \param[out] hour Hour.
    \param[out] minute Minute.
    \param[out] second Second.
    \param[out] millisecond Millisecond.
    \return True if successful and the out arguments were set.
    */
    bool as_timestamp(
        int& year,
        int& month,
        int& day,
        int& hour,
        int& minute,
        int& second,
        int& millisecond
    ) const {
        return
            details::atotime(begin() + 9, end(), hour, minute, second, millisecond)
            && details::atodate(begin(), begin() + 8, year, month, day); // take advantage of short-circuit && to check field length
    }

//@}

#ifdef HFFIX_BOOST_DATETIME

    /*! \name Boost Date and Time Conversion Methods */
//@{

    /*!
     * \brief Ascii-to-date conversion.
     *
     * Parses ascii and returns a LocalMktDate or UTCDate.
     *
     * \return Date if parsing was successful, else `boost::posix_time::not_a_date_time`.
     */
    boost::gregorian::date as_date() const {
        int year, month, day;
        if (as_date(year, month, day)) {
            try {
                return boost::gregorian::date(year, month, day);
            } catch(std::exception& ex) {
                return boost::gregorian::date(boost::posix_time::not_a_date_time);
            }
        } else
            return boost::gregorian::date(boost::posix_time::not_a_date_time);
    }

    /*!
     * \brief Ascii-to-time conversion.
     *
     * Parses ascii and returns a time.
     *
     * \return Time if parsing was successful, else boost::posix_time::not_a_date_time.
     */
    boost::posix_time::time_duration as_timeonly() const {
        int hour, minute, second, millisecond;
        if (as_timeonly(hour, minute, second, millisecond)) {
            try {
                return boost::posix_time::time_duration(hour, minute, second, boost::posix_time::time_duration::ticks_per_second() * millisecond / 1000);
            } catch(std::exception& ex) {
                return boost::posix_time::time_duration(boost::posix_time::not_a_date_time);
            }
        } else
            return boost::posix_time::time_duration(boost::posix_time::not_a_date_time);
    }

    /*!
     * \brief Ascii-to-timestamp conversion.
     *
     * Parses ascii and returns a timestamp.
     *
     * \return Date and Time if parsing was successful, else boost::posix_time::not_a_date_time.
    */
    boost::posix_time::ptime as_timestamp() const {
        int year, month, day, hour, minute, second, millisecond;

        if (details::atotime(begin() + 9, end(), hour, minute, second, millisecond)
                && details::atodate(begin(), begin() + 8, year, month, day)) {
            try {
                return boost::posix_time::ptime(
                           boost::gregorian::date(year, month, day),
                           boost::posix_time::time_duration(hour, minute, second, boost::posix_time::time_duration::ticks_per_second() * millisecond / 1000)
                       );
            } catch(std::exception& ex) {
                return boost::posix_time::not_a_date_time;
            }
        } else
            return boost::posix_time::not_a_date_time;
    }
//@}


#endif // HFFIX_BOOST_DATETIME

#if __cplusplus >= 201103L
    /*! \name std::chrono Date and Time Conversion Methods */
//@{

   /*!
    * \brief Ascii-to-time-point conversion.
    *
    * Parses ascii and returns a `std::chrono::time_point`.
    *
    * Uses algorithms from http://howardhinnant.github.io/date_algorithms.html ,
    * which implement a proleptic Gregorian calendar. This will probably
    * be superseded by C++20.
    *
    * \param[out] tp The return value `time_point`.
    *
    * \return True if parsing was successful and `tp` was set, else False.
    */
    template<typename Clock, typename Duration>
    bool as_timestamp(std::chrono::time_point<Clock,Duration>& tp) const {
        if (details::atotimepoint(begin(), end(), tp))
            return true;
        else
            return false;
    }

   /*!
    * \brief Ascii-to-time conversion.
    *
    * Parses ascii and returns a time duration or time-of-day.
    *
    * \param[out] dur The return value `duration`.
    *
    * \return True if parsing was successful and the return value was set,
    * else False.
    */
    template<typename Rep, typename Period>
    bool as_timeonly(std::chrono::duration<Rep,Period>& dur) const {
        int hour, minute, second, millisecond;
        if (as_timeonly(hour, minute, second, millisecond)) {
            dur = std::chrono::hours(hour) +
                  std::chrono::minutes(minute) +
                  std::chrono::seconds(second) +
                  std::chrono::milliseconds(millisecond);
            return true;
        }
        else
            return false;
    }

//@}
#endif


private:
    friend class field;
    friend class message_reader_const_iterator;
    friend class message_reader;
    char const* begin_;
    char const* end_;
};

/*!
 * \brief A FIX field for hffix::message_reader, with tag and hffix::field_value.
 *
 * This class is the hffix::message_reader::value_type for the hffix::message_reader Container.
 */
class field {
public:

    /*! \brief Tag of the field. */
    int tag() const {
        return tag_;
    }

    /*! \brief Weakly-typed value of the field. */
    field_value const& value() const {
        return value_;
    }

    /*! \brief Output stream operator. Output format is "[tag number]=[value]". */
    friend std::ostream& operator<<(std::ostream& os, field const& that) {
        os << that.tag_ << "=";
        return os.write(that.value_.begin(), that.value_.size());
    }

private:
    friend class message_reader_const_iterator;
    friend class message_reader;
    int tag_;
    field_value value_;
};

/*!
\brief The iterator type for hffix::message_reader. Typedef'd as `hffix::message_reader::const_iterator`.

Satisfies the const Input Iterator Concept for an immutable hffix::message_reader container of fields.
*/
class message_reader_const_iterator {

public:

    /*! /brief No-op construction of an invalid iterator.
     *
     * Like a null pointer, the invalid iterator may not be dereferenced.
     */
    message_reader_const_iterator() {}
private:

    message_reader_const_iterator(message_reader const& container, char const* buffer) :
        message_reader_(&container),
        buffer_(buffer),
        current_() {
    }

public:

    //! \brief For std::iterator_traits
    typedef ::std::input_iterator_tag iterator_category;
    //! \brief For std::iterator_traits
    typedef field value_type;
    //! \brief For std::iterator_traits
    typedef std::ptrdiff_t difference_type;
    //! \brief For std::iterator_traits
    typedef field* pointer;
    //! \brief For std::iterator_traits
    typedef field& reference;


    /*!
    \brief Returns a hffix::message_reader::const_reference to a field.
    */
    field const& operator*() const {
        return current_;
    }

    /*!
    \brief Returns a hffix::message_reader::const_pointer to a field.
    */
    field const* operator->() const {
        return &current_;
    }


    //! \brief Equal
    friend bool operator==(message_reader_const_iterator const& a, message_reader_const_iterator const& b) {
        return a.buffer_ == b.buffer_;
    }

    //! \brief Not equal
    friend bool operator!=(message_reader_const_iterator const& a, message_reader_const_iterator const& b) {
        return a.buffer_ != b.buffer_;
    }

    //! \brief Less-than
    friend bool operator<(message_reader_const_iterator const& a, message_reader_const_iterator const& b) {
        return a.buffer_ < b.buffer_;
    }

    //! \brief Greater-than
    friend bool operator>(message_reader_const_iterator const& a, message_reader_const_iterator const& b) {
        return a.buffer_ > b.buffer_;
    }

    //! \brief Less-than or equal
    friend bool operator<=(message_reader_const_iterator const& a, message_reader_const_iterator const& b) {
        return a.buffer_ <= b.buffer_;
    }

    //! \brief Greater-than or equal
    friend bool operator>=(message_reader_const_iterator const& a, message_reader_const_iterator const& b) {
        return a.buffer_ >= b.buffer_;
    }

    //! \brief Postfix increment
    message_reader_const_iterator operator++(int) {
        message_reader_const_iterator i(*this);
        ++(*this);
        return i;
    }

    //! \brief Prefix increment
    message_reader_const_iterator& operator++() {
        increment();
        return *this;
    }

    /*!
     * \brief Addition
     *
     * \param a Iterator to add to.
     * \param addend Addend.
     * \throw logic_error Throws if *addend < 0*
     * \pre *addend >= 0*
     */
    friend message_reader_const_iterator operator+(message_reader_const_iterator a, int addend) {
        if (addend < 0) throw std::logic_error("message_reader::const_iterator is a Forward Iterator, so only positive addends are allowed.");
        for (int i = 0; i < addend; ++i)
            ++a;

        return a;
    }


    //! \brief Addition
    friend message_reader_const_iterator operator+(int addend, message_reader_const_iterator a) {
        return a + addend;
    }

private:
    friend class message_reader;
    message_reader const* message_reader_; // pointer to the message_reader for this iterator
    char const* buffer_; // pointer to the first character of the ascii tag number for the current_ field
    field current_;

    void increment();
};


/*!
 * \brief A predicate constructed with a FIX tag which returns true if the tag of the hffix::field passed to the predicate is equal.
 */
struct tag_equal {
    tag_equal(int tag) : tag(tag) {}
    int tag;
    bool operator()(field const& v) const {
        return v.tag() == tag;
    }
};


/*!
 * \brief An algorithm similar to `std::find_if` for forward-searching over a range and finding items which match a predicate.
 *
 * Instead of searching from `begin` to `end`, searches from `i` to `end`, then searches from `begin` to `i`.
 * Efficient for finding multiple items when the expected ordering of the items is known.
 *
 * This expression:
 * \code
 * find_with_hint(begin, end, i, predicate)
 * \endcode
 * will behave exactly the same as this expression:
 * \code
 * end != (i = std::find_if(begin, end, predicate))
 * \endcode
 * except for these two differences:
 * * In the first expression, `i` is not modifed if no item is found.
 * * The first expression is faster if the found item is a near successor of `i`.
 *
 * Example usage:
 * \code
 * hffix::message_reader::const_iterator i = reader.begin();
 *
 * if (hffix::find_with_hint(reader.begin(), reader.end(), hffix::tag_equal(hffix::tag::MsgSeqNum), i)
 *   int seqnum = i++->as_int<int>();
 *
 * if (hffix::find_with_hint(reader.begin(), reader.end(), hffix::tag_equal(hffix::tag::TargetCompID), i)
 *   std::string targetcompid = i++->as_string();
 * \endcode
 *
 * See also the convenience method hffix::message_reader::find_with_hint.
 *
 * \param begin The beginning of the range to search.
 * \param end The end of the range to search.
 * \param predicate A predicate which provides function `bool operator() (ForwardIterator::value_type const &v) const`.
 * \param i If an item is found which satisfies `predicate`, then `i` is modified to point to the found item. Else `i` is unmodified.
 * \return True if an item was found which matched `predicate`, and `i` was modified to point to the found item.
 */
template <typename ForwardIterator, typename UnaryPredicate>
inline bool find_with_hint(ForwardIterator begin, ForwardIterator end, UnaryPredicate predicate, ForwardIterator & i) {
    ForwardIterator j = std::find_if(i, end, predicate);
    if (j != end) {
        i = j;
        return true;
    }
    j = std::find_if(begin, i, predicate);
    if (j != i) {
        i = j;
        return true;
    }
    return false;
}


/*!
 * \brief One FIX message for reading.
 *
 * An immutable Forward Container of FIX fields. Given a buffer containing a FIX message, the hffix::message_reader
 * will provide an Iterator for iterating over the fields in the message without modifying the buffer. The buffer
 * used to construct the hffix::message_reader must outlive the hffix::message_reader.
 *
 * During construction, hffix::message_reader checks to make sure there is a complete,
 * valid FIX message in the buffer. It looks only at the header and trailer transport fields in the message,
 * not at the content fields, so construction is O(1).
 *
 * If hffix::message_reader is complete and valid after construction,
 * hffix::message_reader::begin() returns an iterator that points to the MsgType field
 * in the FIX Standard Message Header, and
 * hffix::message_reader::end() returns an iterator that points to the CheckSum field in the
 * FIX Standard Message Trailer.
 *
 * The hffix::message_reader will only iterate over content fields of the message, and will skip over all of the framing transport fields
 *  that are mixed in with the content fields in FIX. Here is the list of skipped fields which will not appear when iterating over the fields of the message:
 *
 * - BeginString
 * - BodyLength
 * - CheckSum
 * - And all of the binary data length framing fields listed in hffix::anonymous_namespace{hffix_fields.hpp}::length_fields.
 *
 * Fields of binary data type are content fields, and will be iterated over like any other field.
 * The special FIX binary data length framing fields will be skipped, but the length of the binary data
 * is accessible from the hffix::message_reader::value_type::value().size() of the content field.
*/
class message_reader {

public:

    typedef field value_type;
    typedef field const& const_reference;
    typedef message_reader_const_iterator const_iterator;
    typedef field const* const_pointer;
    typedef size_t size_type;

    /*!
    \brief Construct by buffer size.
    \param buffer Pointer to the buffer to be read.
    \param size Number of bytes in the buffer to be read.
    */
    message_reader(char const* buffer, size_t size) :
        buffer_(buffer),
        buffer_end_(buffer + size),
        begin_(*this, 0),
        end_(*this, 0),
        is_complete_(false),
        is_valid_(true) {
        init();
    }

    /*!
    \brief Construct by buffer begin and end.
    \param begin Pointer to the buffer to be read.
    \param end Pointer to past-the-end of the buffer to be read.
    */
    message_reader(char const* begin, char const* end) :
        buffer_(begin),
        buffer_end_(end),
        begin_(*this, 0),
        end_(*this, 0),
        is_complete_(false),
        is_valid_(true) {
        init();
    }
    /*!
     * \brief Copy constructor. The hffix::message_reader is immutable, so copying it is fine.
     */
    message_reader(message_reader const& that) :
        buffer_(that.buffer_),
        buffer_end_(that.buffer_end_),
        begin_(that.begin_),
        end_(that.end_),
        is_complete_(that.is_complete_),
        is_valid_(that.is_valid_) {
    }

    /*!
     * \brief Construct a message_reader from a message_writer. Equivalent to
     * \code
     * hffix::message_writer w;
     * hffix::message_reader r(w.message_begin(), w.message_end());
     * \endcode
     */
    message_reader(message_writer const& that) :
        buffer_(that.message_begin()),
        buffer_end_(that.message_end()),
        begin_(*this, 0),
        end_(*this, 0),
        is_complete_(false),
        is_valid_(true) {
        init();
    }

    /*!
    \brief Construct on an array reference to a buffer.
    \tparam N The size of the array.
    \param buffer An array reference. The reader will read from the entire array of length _N_.
    */
    template<size_t N>
    message_reader(const char(&buffer)[N]) :
        buffer_(buffer),
        buffer_end_(&(buffer[N])),
        begin_(*this, 0),
        end_(*this, 0),
        is_complete_(false),
        is_valid_(true) {
        init();
    }

    /*!
     * \brief Owns no resources, so destruction is no-op.
     */
    ~message_reader() {
    }

    /*!
     * \brief True if the buffer contains a complete FIX message.
     */
    bool is_complete() const {
        return is_complete_;
    }
    /*!
     * \brief True if the message is valid.
     *
     * A valid message must meet these criteria.
     * * The first field is *BeginString*.
     * * The next field is *BodyLength*, and there is a *CheckSum* field at the end of the message at the location dictated by *BodyLength*.
     * * After *BodyLength* there is a *MsgType* field.
     *
     * If false, the message is unintelligable, and the length of the message is unknown.
     *
     * _fix-42_with_errata_20010501.pdf_ p.17:
     * "Valid FIX Message is a message that is properly formed according to this specification and contains a
     * valid body length and checksum field"
     *
    */
    bool is_valid() const {
        return is_valid_;
    }


    /*!
     * \brief Returns a new message_reader for the next FIX message in the buffer.
     *
     * If this message is_valid() and is_complete(), assume that the next message comes immediately
     * after this one and return a new message_reader constructed at this->message_end().
     *
     * If this message `!`is_valid(), will search the remainder of the buffer
     * for the text "8=FIX", to see if there might be a complete or partial valid message
     * anywhere else in the remainder of the buffer, will return a new message_reader constructed at that location.
     *
     * \throw std::logic_error If this message `!`is_complete().
     */
    message_reader next_message_reader() const {
        if (!is_complete_) {
            throw std::logic_error("Can't call next_message_reader on an incomplete message.");
        }

        if (!is_valid_) { // this message isn't valid, so we have to try to search for the beginning of the next message.
            char const* b = buffer_ + 1;
            while(b < buffer_end_ - 10) {
                if (!std::memcmp(b, "8=FIX", 5))
                    break;
                ++b;
            }
            return message_reader(b, buffer_end_);
        }

        return message_reader(end_.current_.value_.end_ + 1, buffer_end_);
    }

   /*!
    \brief Calulate the checksum for this message.

    Note that the *hffix* library never does this calculation implicitly
    for messages read. For checksum calculation this function must be
    explicitly called.

    The only thing to do after calculating the checksum for this message
    is to compare it to the CheckSum field that the message reports for
    itself, like so:

    \code
    hffix::message_reader r;
    if (r.calculate_check_sum() == r.check_sum()->value()as_int<unsigned char>()) {}
    \endcode

    \return The calculated checksum.

    \throw std::logic_error if called on an invalid message. Check for `is_valid()` before calling.
    */
    unsigned char calculate_check_sum() {
        // return iterator for beginning of nonmutable sequence
        if (!is_valid_) throw std::logic_error("hffix Cannot calculate checksum for an invalid message.");
        return std::accumulate(buffer_, end_.buffer_, (unsigned char)(0));
    }

    /*! \name Field Access */
//@{
    /*!
    \brief An iterator to the MsgType field in the FIX message. Same as hffix::message_reader::message_type().
    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    const_iterator begin() const {
        // return iterator for beginning of nonmutable sequence
        if (!is_valid_) throw std::logic_error("hffix Cannot return iterator for an invalid message.");
        return begin_;
    }

    /*!
    \brief An iterator to the CheckSum field in the FIX message. Same as hffix::message_reader::check_sum().
    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    const_iterator end() const {
        // return iterator for end of nonmutable sequence
        if (!is_valid_) throw std::logic_error("hffix Cannot return iterator for an invalid message.");
        return end_;
    }

    /*!
    \brief An iterator to the MsgType field in the FIX message. Same as hffix::message_reader::begin().
    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    const_iterator message_type() const {
        // return iterator for beginning of nonmutable sequence
        if (!is_valid_) throw std::logic_error("hffix Cannot return iterator for an invalid message.");
        return begin_;
    }

    /*!
    \brief An iterator to the CheckSum field in the FIX message. Same as hffix::message_reader::end().
    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    const_iterator check_sum() const {
        // return iterator for end of nonmutable sequence
        if (!is_valid_) throw std::logic_error("hffix Cannot return iterator for an invalid message.");
        return end_;
    }

    /*!
     * \brief Returns the FIX version prefix BeginString field value begin pointer. (Example: "FIX.4.4")
     */
    char const* prefix_begin() const {
        return buffer_ + 2;
    }

    /*!
     * \brief Returns the FIX version prefix BeginString field value end pointer.
     */
    char const* prefix_end() const {
        return prefix_end_;
    }

    /*!
     * \brief Returns the FIX version prefix BeginString field value size. (Example: returns 7 for "FIX.4.4")
     */
    size_t prefix_size() const {
        return prefix_end_ - buffer_ - 2;
    }

    /*!
     * \brief Convenient synonym for `hffix::find_with_hint(reader.begin(), reader.end(), hffix::tag_equal(tag), i)`.
     *
     * Similar to `std::find_if`. See `hffix::find_with_hint` for details.
     *
     * \param tag The field tag number to find.
     * \param i If a field is found which has the tag number `tag`, then `i` is modified to point to the found item. Else `i` is unmodified.
     * \return True if a field was found, and `i` was modified to point to the found field.
     *
     * Example usage:
     * \code
     * hffix::message_reader::const_iterator i = reader.begin();
     *
     * if (reader.find_with_hint(MsgSeqNum, i))
     *   int seqnum = i++->as_int<int>();
     *
     * if (reader.find_with_hint(TargetCompID, i))
     *   std::string targetcompid = i++->as_string();
     * \endcode
     */
    bool find_with_hint(int tag, const_iterator& i) const {
        return hffix::find_with_hint(begin(), end(), tag_equal(tag), i);
    }

//@}


    /*! \name Buffer Access */
//@{
    /*!
    \brief A pointer to the begining of the buffer.

    buffer_begin() == message_begin()
    */
    char const* buffer_begin() const {
        return buffer_;
    }
    /*!
    \brief A pointer to past-the-end of the buffer.
    */
    char const* buffer_end() const {
        return buffer_end_;
    }

    /*!
    \brief The size of the buffer in bytes.
    */
    size_t buffer_size() const {
        return buffer_end_ - buffer_;
    }

    /*!
    \brief A pointer to the beginning of the FIX message in the buffer.

     buffer_begin() == message_begin()

    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    char const* message_begin() const {
        return buffer_;
    }
    /*!
    \brief A pointer to past-the-end of the FIX message in the buffer.
    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    char const* message_end() const {
        if (!is_valid_) throw std::logic_error("hffix Cannot determine size of an invalid message.");
        return end_.current_.value_.end_ + 1;
    }

    /*!
    \brief The entire size of the FIX message in bytes.
    \throw std::logic_error if called on an invalid message. This exception is preventable by program logic. You should always check if a message is_valid() before reading.
    */
    size_t message_size() const {
        if (!is_valid_) throw std::logic_error("hffix Cannot determine size of an invalid message.");
        return end_.current_.value_.end_ - buffer_ + 1;
    }

//@}

private:
    friend class message_reader_const_iterator;

    void init() {

        // Skip the version prefix string "8=FIX.4.2" or "8=FIXT.1.1", et cetera.
        char const* b = buffer_ + 9; // look for the first '\x01'

        while(true) {
            if (b >= buffer_end_) {
                is_complete_ = false;
                return;
            }
            if (*b == '\x01') {
                prefix_end_ = b;
                break;
            }
            if (b - buffer_ > 11) {
                invalid();
                return;
            }
            ++b;
        }

        if (b + 1 >= buffer_end_) {
            is_complete_ = false;
            return;
        }
        if (b[1] != '9') { // next field must be tag 9 BodyLength
            invalid();
            return;
        }
        b += 3; // skip the " 9=" for tag 9 BodyLength

        size_t bodylength(0); // the value of tag 9 BodyLength

        while(true) {
            if (b >= buffer_end_) {
                is_complete_ = false;
                return;
            }
            if (*b == '\x01') break;
            if (*b < '0' || *b > '9') { // this is the only time we need to check for numeric ascii.
                invalid();
                return;
            }
            bodylength *= 10;
            bodylength += *b++ - '0'; // we know that 0 <= (*b - '0') <= 9, so rvalue will be positive.
        }

        ++b;
        if (b + 3 >= buffer_end_) {
            is_complete_ = false;
            return;
        }

        if (*b != '3' || b[1] != '5') { // next field must be tag 35 MsgType
            invalid();
            return;
        }

        char const* checksum = b + bodylength;

        if (checksum + 7 > buffer_end_) {
            is_complete_ = false;
            return;
        }

        if (*(checksum - 1) != '\x01') { // check for SOH before the checksum.
                                         // this guarantees that at least
                                         // there is one SOH in the message
                                         // which will prevent us from
                                         // falling off of the end of
                                         // a malformed message while
                                         // iterating.
            invalid();
            return;
        }

        if (*(checksum + 6) != '\x01') { // check for trailing SOH
            invalid();
            return;
        }

        begin_.buffer_ = b;
        begin_.current_.tag_ = 35; // MsgType
        b += 3;
        begin_.current_.value_.begin_ = b;
        while(*++b != '\x01') {
            if (b >= checksum) {
                invalid();
                return;
            }
        }
        begin_.current_.value_.end_ = b;

        end_.buffer_ = checksum;
        end_.current_.tag_ = 10; //CheckSum
        end_.current_.value_.begin_ = checksum + 3;
        end_.current_.value_.end_ = checksum + 6;

        is_complete_ = true;
    }

    char const* buffer_;
    char const* buffer_end_;
    const_iterator begin_;
    const_iterator end_;
    bool is_complete_;
    bool is_valid_;
    char const* prefix_end_; // Points after the 8=FIX... Prefix field.

    void invalid() {
        is_complete_ = true; // invalid messages are considered complete, for use of the message_reader::operator++()
        is_valid_ = false;
    }
};


/*! @cond EXCLUDE */
namespace details {
bool is_tag_a_data_length(int tag);
}
/*! @endcond */


inline void message_reader_const_iterator::increment()
{
    buffer_ = current_.value_.end_ + 1;
    current_.value_.begin_ = buffer_;
    current_.tag_ = 0;

    while(*current_.value_.begin_ != '=' && *current_.value_.begin_ != '\x01') {
        current_.tag_ *= 10;
        current_.tag_ += (*current_.value_.begin_ - '0');
        ++current_.value_.begin_;
    }

    // we expect to see a '='. if we see a '\x01' at this point then this field
    // has no value and the message is invalid, so we're doomed. it's too
    // late to set is_invalid, though, so let's just say that this field
    // has a null value.
    if (*current_.value_.begin_ == '\x01') {
        current_.value_.end_ = current_.value_.begin_;
        return;
    }

    // move past the '='.
    ++current_.value_.begin_;

    // find the end of the field value
    current_.value_.end_ = std::find(current_.value_.begin_, message_reader_->message_end(), '\x01');
    if (details::is_tag_a_data_length(current_.tag_)) {
        size_t data_len = details::atou<size_t>(current_.value_.begin_, current_.value_.end_);

        buffer_ = current_.value_.end_ + 1;
        current_.value_.begin_ = buffer_;
        current_.tag_ = 0;

        while(*current_.value_.begin_ != '=') {
            current_.tag_ *= 10;
            current_.tag_ += (*current_.value_.begin_ - '0');
            ++current_.value_.begin_;
        }

        current_.value_.end_ = ++current_.value_.begin_ + data_len;
    }
}

/* @cond EXCLUDE */

namespace details {


// A predicate constructed with an int which returns true if the int passed to
// the predicate is greater than or equal to the int passed to the constructor.
struct int_gte {
    int_gte(int tag) : tag(tag) {}
    int tag;
    bool operator()(int that) const {
        return that >= tag;
    }
};

// Returns true if the argument exists in the length_fields array.
//
// We have to call this function every time a message_reader iterator
// is incremented, so we want it to be fast.
//
// Instead of doing std::binary_search on the sorted range of length_fields,
// we'll take advantage of an assumption that most tags are low-numbered
// tags, and search from the beginning of length_fields, hoping that
// our search will usually end quickly.
//
// TODO: This has not yet been benchmarked against any alternatives.
// The benchmark results would depend on the distrubution of tags
// in the FIX data set.
inline bool is_tag_a_data_length(int tag)
{
    int* length_fields_end = length_fields + (sizeof(length_fields)/sizeof(length_fields[0]));
    int* i = std::find_if(length_fields, length_fields_end, int_gte(tag));
    if (i == length_fields_end) return false;
    return (*i == tag);
}

// \brief std::ostream-able type returned by hffix::field_name function.
template <typename AssociativeContainer> struct field_name_streamer {
    int tag;
    AssociativeContainer const& field_dictionary;
    bool number_alternative;

    field_name_streamer(int tag, AssociativeContainer const& field_dictionary, bool number_alternative) : tag(tag), field_dictionary(field_dictionary), number_alternative(number_alternative) {}

    friend std::ostream& operator<<(std::ostream& os, field_name_streamer that) {
        typename AssociativeContainer::const_iterator i = that.field_dictionary.find(that.tag);
        if (i != that.field_dictionary.end())
            os << i->second;
        else if (that.number_alternative)
            os << that.tag;
        return os;
    }
};

}
/* @endcond */

/*!
  * \brief Given a field tag number and a field name dictionary, returns a type which provides `operator<<`  to write the name of the field to an `std::ostream`.
  * \tparam AssociativeContainer The type of the field name dictionary. Must satisfy concept `AssociativeContainer<int, std::string>`, for example `std::map<int, std::string>` or `std::unordered_map<int, std::string>`. See https://en.cppreference.com/w/cpp/named_req/AssociativeContainer
  *
  * \param tag The field number.
  * \param field_dictionary The field dictionary.
  * \param or_number Specifies behavior if the tag is not found in the dictionary. If true, then the string representation of the flag will be written to the std::ostream. If false, then nothing will be written to the std::ostream. Default is false.
  *
  * Example usage:
  * \code
  * std::map<int, std::string> dictionary;
  * hffix::dictionary_init_field(dictionary);
  * std::cout << hffix::field_name(hffix::tag::SenderCompID, dictionary) << '\n'; // Will print "SenderCompID\n".
  * std::cout << hffix::field_name(1000000, dictionary) << '\n';                  // Unknown field tag, will print "1000000\n".
  * std::cout << hffix::field_name(1000000, dictionary, false) << '\n';           // Unknown field tag, will print "\n".
  * \endcode
*/
template <typename AssociativeContainer> details::field_name_streamer<AssociativeContainer> field_name(int tag, AssociativeContainer const& field_dictionary, bool or_number = true)
{
    return details::field_name_streamer<AssociativeContainer>(tag, field_dictionary, or_number);
}

} // namespace hffix

#endif
