/**
 * @file time_fun.cpp
 * @brief time functionality
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 4/28/2014
 * FIXME: not thread-safe!
 
    This file is part of MavLogAnalyzer, Copyright 2014 by Martin Becker.
    
    MavLogAnalyzer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.     

 */

#define _XOPEN_SOURCE /// strptime
#include <time.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h> // memset
#include <math.h>
#include "time_fun.h"
#include "stringfun.h"

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
#else
#include <sys/time.h>
#endif

using namespace std;

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
#include <ctype.h>
#include <string.h>
#include <time.h>
 
 
/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E          0x01
#define ALT_O          0x02
//#define LEGAL_ALT(x)       { if (alt_format & ~(x)) return (0); }
#define LEGAL_ALT(x)       { ; }
#define TM_YEAR_BASE   (1970)
 
static   int conv_num(const char **, int *, int, int);
static int strncasecmp(char *s1, char *s2, size_t n);
 
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
/** @unused
static const char *am_pm[2] = {
     "AM", "PM"
};
*/
 
 
char * strptime(const char *buf, const char *fmt, struct tm *tm)
{
    char c;
    const char *bp;
    size_t len = 0;
    int alt_format, i, split_year = 0;
 
    bp = buf;

    while ((c = *fmt) != '\0')
    {
        /* Clear `alternate' modifier prior to new conversion. */
        alt_format = 0;
     
        /* Eat up white-space. */
        if (isspace(c))
        {
            while (isspace(*bp))
                bp++;
 
            fmt++;
            continue;
        }
                 
             if ((c = *fmt++) != '%')
                  goto literal;
 
 
again:        switch (c = *fmt++)
        {
            case '%': /* "%%" is converted to "%". */
                literal:
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
                for (i = 0; i < 7; i++)
                {
                    /* Full name. */
                    len = strlen(day[i]);
                    if (strncasecmp((char *)(day[i]), (char *)bp, len) == 0)
                        break;
         
                    /* Abbreviated name. */
                    len = strlen(abday[i]);
                    if (strncasecmp((char *)(abday[i]), (char *)bp, len) == 0)
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
                for (i = 0; i < 12; i++)
                {
                     /* Full name. */

                    len = strlen(mon[i]);
                    if (strncasecmp((char *)(mon[i]), (char *)bp, len) == 0)
                        break;
 
                    /* Abbreviated name. */
                    len = strlen(abmon[i]);
                    if (strncasecmp((char *)(abmon[i]),(char *) bp, len) == 0)
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
 
                if (split_year)
                {
                    tm->tm_year = (tm->tm_year % 100) + (i * 100);
                } else {
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
 
//            case 'p': /* The locale's equivalent of AM/PM. */
//                LEGAL_ALT(0);
//                /* AM? */
//                if (strcasecmp(am_pm[0], bp) == 0)
//                {
//                    if (tm->tm_hour > 11)
//                        return (0);
//
//                    bp += strlen(am_pm[0]);
//                    break;
//                }
//                /* PM? */
//                else if (strcasecmp(am_pm[1], bp) == 0)
//                {
//                    if (tm->tm_hour > 11)
//                        return (0);
//
//                    tm->tm_hour += 12;
//                    bp += strlen(am_pm[1]);
//                    break;
//                }
//
//                /* Nothing matched. */
//                return (0);
         
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
 
                if (split_year)
                {
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
 
 
            default: /* Unknown/unsupported conversion. */
                return (0);
        }
 
 
    }
 
    /* LINTED functional specification */
    return ((char *)bp);
}
 
 
static int conv_num(const char **buf, int *dest, int llim, int ulim)
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
 
int strncasecmp(char *s1, char *s2, size_t n)
{
    if (n == 0)
        return 0;
 
    while (n-- != 0 && tolower(*s1) == tolower(*s2))
    {
        if (n == 0 || *s1 == '\0' || *s2 == '\0')
            break;
        s1++;
        s2++;
    }
 
    return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}

double get_time_secs(void) {
    return 0.; // TODO
}

#else /* NOT WIN32" */
double get_time_secs(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)(tv.tv_sec + (tv.tv_usec / 1E6)));
}
#endif /* WIN32 */

struct tm epoch_to_tm(double epoch_sec) {
    time_t epoch_sec_t = (time_t) round(epoch_sec);
    struct tm * ltime = localtime(&epoch_sec_t);///< not thread safe
    struct tm ret  = *ltime; // deep copy
    return ret;
}

string epoch_to_datetime (double epoch_sec, bool databaseformat) {
    struct tm ltime = epoch_to_tm(epoch_sec);

    string strtime;
    if (!databaseformat) {
        strtime = asctime(&ltime);
        string_oneline(strtime);    // remove trailing newline
    } else {
        // for SQL "0000-00-00 00:00:00"
        char strbuf[20];
        strftime(strbuf, 20, "%F %T", &ltime);
        strtime = strbuf;
    }
    return strtime;
}

uint64_t tm_to_epoch_usec(const struct tm * const tm) {
    if (!tm) return 0;

    // take a copy, because mktime() modifies the argument ...
    struct tm * mytm =  new struct tm;
    if (!mytm) return 0;
    memcpy(mytm, tm, sizeof(struct tm));

    time_t val_sec = mktime(mytm);
    uint64_t val = 0;
    if (val_sec != -1) {
        val = val_sec * 1E6;
    }

    delete mytm;
    return val;
}

bool string_to_epoch_usec(const string & strtimedate, uint64_t & result_epoch_usec) {
    struct tm tm;

    // try different possibilities
    memset(&tm, 0, sizeof(struct tm));
    const char*const tmp = strtimedate.c_str();
    const char*const str_end = tmp+strtimedate.length();
    char * res = strptime(tmp, "%Y-%m-%d %H-%M-%S", &tm); // w/ space: Mission Planner
    if (str_end == res) {
        result_epoch_usec = tm_to_epoch_usec(&tm);
        return true;
    }
    res = strptime(tmp, "%Y-%m-%d-%H-%M-%S", &tm); // w/o space
    if (str_end == res) {
        result_epoch_usec = tm_to_epoch_usec(&tm);
        return true;
    }
    res = strptime(tmp, "%Y-%m-%d %H:%M:%S", &tm); // more or less common
    if (str_end == res) {
        result_epoch_usec = tm_to_epoch_usec(&tm);
        return true;
    }
    res = strptime(tmp, "%Y-%m-%d %H-%M", &tm); // also possible
    if (str_end == res) {
        result_epoch_usec = tm_to_epoch_usec(&tm);
        return true;
    }
    return false; // no idea
}

string seconds_to_timestr (double sec, bool decimals) {
    double integral;
    double msec = modf(sec,&integral); // split into decimal and integral part
    time_t seconds((time_t) integral);
    tm *p = gmtime(&seconds);

    stringstream ss;
    ss.precision(3);    
    string s;

    // days
    if (p->tm_yday > 0) {
        ss << std::setw(2) << std::setfill('0') << p->tm_yday << "/";
    }
    // hours
    if (p->tm_hour > 0 || p->tm_yday > 0) {
        ss << std::setw(2) << std::setfill('0') << p->tm_hour << ":";
    }
    // minutes
    if (p->tm_min > 0 || p->tm_hour > 0 || p->tm_yday > 0) {
        ss << std::setw(2) << std::setfill('0') << p->tm_min << ":";
    }
    // seconds
    ss << std::setw(2) << std::setfill('0') << p->tm_sec;
    // subseconds
    if (decimals) {
        stringstream ss1;
        ss1 << std::setprecision(3) << std::setw(3) << std::setfill('0') << msec;
        ss << "." << ss1.str().substr(2); // remove leading zero
    }
    return ss.str();
}

uint64_t gpsepoch2unixepoch_usec(uint16_t gps_week, uint32_t gps_week_ms) {
    const uint64_t ms_per_week = 7000ULL*86400ULL;
    const uint64_t unix_offset = 17000ULL*86400ULL // 10 days (Schaltjahr + 5 days difference with epochs)
                                + 52*10*7000ULL*86400ULL // unix epoch is 10 years before GPS epoch
                                - 15000ULL; // difference between GPS time and UTC: actually 19s since 01.07.2012, but !!! APM uses the value from 2009, which is 15s.
    uint64_t fix_time_ms = unix_offset + gps_week*ms_per_week + gps_week_ms;
    return fix_time_ms*1000UL;
}
