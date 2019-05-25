/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * Timer for kungfu system.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Provide basic nano time and time transformation
 */

#ifndef YIJINJING_TIMER_H
#define YIJINJING_TIMER_H

#ifdef _WINDOWS
#include <ctime>
#include <ctype.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
#define TM_YEAR_BASE 1900

/*
* We do not implement alternate representations. However, we always
* check whether a given modifier is allowed for a certain conversion.
*/
#define ALT_E     0x01
#define ALT_O     0x02
#define LEGAL_ALT(x)    { if (alt_format & ~(x)) return (0); }


static  int conv_num(const char **, int *, int, int);

static const char *day[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday"
};
static const char *abday[7] = {
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static const char *mon[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};
static const char *abmon[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
	"AM", "PM"
};

inline int conv_num(const char **buf, int *dest, int llim, int ulim)
{
	int result = 0;

	/* The limit also determines the number of valid digits. */
	int rulim = ulim;

	if (**buf < '0' || **buf > '9')
		return (0);

	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim)
		return (0);

	*dest = result;
	return (1);
}

inline char * strptime(const char *buf, const char *fmt, struct tm *tm)
{
	char c;
	const char *bp;
	size_t len = 0;
	int alt_format, i, split_year = 0;

	bp = buf;

	while ((c = *fmt) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}

		if ((c = *fmt++) != '%')
			goto literal;


	again:    switch (c = *fmt++) {
	case '%': /* "%%" is converted to "%". */
		literal :
			if (c != *bp++)
				return (0);
		break;

		/*
		* "Alternative" modifiers. Just set the appropriate flag
		* and start over again.
		*/
	case 'E': /* "%E?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_E;
		goto again;

	case 'O': /* "%O?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_O;
		goto again;

		/*
		* "Complex" conversion rules, implemented through recursion.
		*/
	case 'c': /* Date and time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%x %X", tm)))
			return (0);
		break;

	case 'D': /* The date as "%m/%d/%y". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

	case 'R': /* The time as "%H:%M". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M", tm)))
			return (0);
		break;

	case 'r': /* The time in 12-hour clock representation. */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
			return (0);
		break;

	case 'T': /* The time as "%H:%M:%S". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'X': /* The time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'x': /* The date, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

		/*
		* "Elementary" conversion rules.
		*/
	case 'A': /* The day of week, using the locale's form. */
	case 'a':
		LEGAL_ALT(0);
		for (i = 0; i < 7; i++) {
			/* Full name. */
			len = strlen(day[i]);
			if (strncmp(day[i], bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abday[i]);
			if (strncmp(abday[i], bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 7)
			return (0);

		tm->tm_wday = i;
		bp += len;
		break;

	case 'B': /* The month, using the locale's form. */
	case 'b':
	case 'h':
		LEGAL_ALT(0);
		for (i = 0; i < 12; i++) {
			/* Full name. */
			len = strlen(mon[i]);
			if (strncmp(mon[i], bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abmon[i]);
			if (strncmp(abmon[i], bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 12)
			return (0);

		tm->tm_mon = i;
		bp += len;
		break;

	case 'C': /* The century number. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = (tm->tm_year % 100) + (i * 100);
		}
		else {
			tm->tm_year = i * 100;
			split_year = 1;
		}
		break;

	case 'd': /* The day of month. */
	case 'e':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
			return (0);
		break;

	case 'k': /* The hour (24-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'H':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
			return (0);
		break;

	case 'l': /* The hour (12-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'I':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
			return (0);
		if (tm->tm_hour == 12)
			tm->tm_hour = 0;
		break;

	case 'j': /* The day of year. */
		LEGAL_ALT(0);
		if (!(conv_num(&bp, &i, 1, 366)))
			return (0);
		tm->tm_yday = i - 1;
		break;

	case 'M': /* The minute. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
			return (0);
		break;

	case 'm': /* The month. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &i, 1, 12)))
			return (0);
		tm->tm_mon = i - 1;
		break;

	case 'p': /* The locale's equivalent of AM/PM. */
		LEGAL_ALT(0);
		/* AM? */
		if (strcmp(am_pm[0], bp) == 0) {
			if (tm->tm_hour > 11)
				return (0);

			bp += strlen(am_pm[0]);
			break;
		}
		/* PM? */
		else if (strcmp(am_pm[1], bp) == 0) {
			if (tm->tm_hour > 11)
				return (0);

			tm->tm_hour += 12;
			bp += strlen(am_pm[1]);
			break;
		}

		/* Nothing matched. */
		return (0);

	case 'S': /* The seconds. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
			return (0);
		break;

	case 'U': /* The week of year, beginning on sunday. */
	case 'W': /* The week of year, beginning on monday. */
		LEGAL_ALT(ALT_O);
		/*
		* XXX This is bogus, as we can not assume any valid
		* information present in the tm structure at this
		* point to calculate a real value, so just check the
		* range for now.
		*/
		if (!(conv_num(&bp, &i, 0, 53)))
			return (0);
		break;

	case 'w': /* The day of week, beginning on sunday. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
			return (0);
		break;

	case 'Y': /* The year. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 9999)))
			return (0);

		tm->tm_year = i - TM_YEAR_BASE;
		break;

	case 'y': /* The year within 100 years of the epoch. */
		LEGAL_ALT(ALT_E | ALT_O);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = ((tm->tm_year / 100) * 100) + i;
			break;
		}
		split_year = 1;
		if (i <= 68)
			tm->tm_year = i + 2000 - TM_YEAR_BASE;
		else
			tm->tm_year = i + 1900 - TM_YEAR_BASE;
		break;

		/*
		* Miscellaneous conversions.
		*/
	case 'n': /* Any kind of white-space. */
	case 't':
		LEGAL_ALT(0);
		while (isspace(*bp))
			bp++;
		break;


	default:  /* Unknown/unsupported conversion. */
		return (0);
	}


	}

	/* LINTED functional specification */
	return ((char *)bp);
}

//////////////////////////////////////////////////////////////////////////
#endif // _WINDOWS

#include <kungfu/yijinjing/common.h>

YJJ_NAMESPACE_START

const int64_t MILLISECONDS_PER_SECOND = 1000;
const int64_t MICROSECONDS_PER_MILLISECOND = 1000;
const int64_t NANOSECONDS_PER_MICROSECOND = 1000;

const int64_t MICROSECONDS_PER_SECOND = MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;
const int64_t NANOSECONDS_PER_MILLISECOND = NANOSECONDS_PER_MICROSECOND * MICROSECONDS_PER_MILLISECOND;
const int64_t NANOSECONDS_PER_SECOND = NANOSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;

const int  SECONDS_PER_MINUTE = 60;
const int  MINUTES_PER_HOUR = 60;
const int  HOURS_PER_DAY = 24;
const int  SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

const int64_t MILLISECONDS_PER_MINUTE = MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE;
const int64_t NANOSECONDS_PER_MINUTE = NANOSECONDS_PER_SECOND * SECONDS_PER_MINUTE;
const int64_t NANOSECONDS_PER_HOUR = NANOSECONDS_PER_SECOND * SECONDS_PER_HOUR;
const int64_t NANOSECONDS_PER_DAY = NANOSECONDS_PER_HOUR * HOURS_PER_DAY;

/**
 * timer for nanosecond, main class
 */
class NanoTimer
{
public:
    /** return current nano time: unix-timestamp * 1e9 + nano-part */
    int64_t   getNano() const;
    /** return basetime_in_nano */
    inline int64_t getBasetimeInNano() const { return basetime_in_nano; }
    /** singleton */
    static NanoTimer* getInstance();

private:
    NanoTimer();
    /** singleton */
    static std::shared_ptr<NanoTimer> m_ptr;
    int64_t basetime_in_nano;
};

/**
 * util function to utilize NanoTimer
 * @return current nano time in int64_t (unix-timestamp * 1e9 + nano-part)
 */
inline int64_t getNanoTime()
{
    return NanoTimer::getInstance()->getNano();
}

/**
 * util function to utilize NanoTimer
 * @return second diff in int64_t for nano time matching
 */
inline int64_t getBasetimeInNano()
{
    return NanoTimer::getInstance()->getBasetimeInNano();
}

/**
 * parse struct tm to nano time
 * @param _tm ctime struct
 * @return nano time in int64_t
 */
inline int64_t parseTm(struct tm _tm)
{
#ifdef _WINDOWS
    return mktime(&_tm) * NANOSECONDS_PER_SECOND;
#else
    return timelocal(&_tm) * NANOSECONDS_PER_SECOND;
#endif // _WINDOWS
}

/**
 * parse string time to nano time
 * @param timeStr string-formatted time
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return nano time in int64_t
 */
inline int64_t parseTime(const char* timeStr, const char* format)
{
    struct tm _tm;
    strptime(timeStr, format, &_tm);
    return parseTm(_tm);
}

/**
 * dump int64_t time to string with format
 * @param nano nano time in int64_t
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return string-formatted time
 */
inline string parseNano(int64_t nano, const char* format)
{
    if (nano <= 0)
        return string("NULL");
    int64_t seconds = nano / NANOSECONDS_PER_SECOND;
    struct tm * dt;
    char buffer [30];
    dt = localtime((const time_t *)&seconds);
    strftime(buffer, sizeof(buffer), format, dt);
    return string(buffer);
}

/**
 * dump int64_t time to struct tm
 * @param nano nano time in int64_t
 * @return ctime struct
 */
inline struct tm parseNano(int64_t nano)
{
    time_t sec_num = nano / NANOSECONDS_PER_SECOND;
    return *localtime(&sec_num);
}

YJJ_NAMESPACE_END

#endif //YIJINJING_TIMER_H