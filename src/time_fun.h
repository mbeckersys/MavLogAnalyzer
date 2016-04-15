/**
 * @file time_fun.h
 * @brief time functionality
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 4/28/2014
 
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

#ifndef TIME_FUN_H
#define TIME_FUN_H

#include <string>
#include <time.h>
#include <inttypes.h>

/**
 * @brief epoch_to_datetime
 * @param epoch_sec unix time (seconds since 00:00:00 UTC 1 January 1970)
 * @param databaseformat if true, then formatted in a way s.t. SQL can understand it
 * @return string with local date and time, up to seconds
 */
std::string epoch_to_datetime (double epoch_sec, bool databaseformat = false);

/**
 * @brief epoch_to_tm
 * @param epoch_sec the seconds passed since 00:00:00 UTC 1 January 1970
 * @return time broken into its components
 */
struct tm epoch_to_tm(double epoch_sec);

/**
 * @brief turns seconds into some readable time DURATION
 * @param sec some time given in seconds
 * @return string equivalend time as string, formatted "dd/hh:mm:ss.zzz",
 * only as long as necessary, i.e., if days==0, then days are not mentioned.
 */
std::string seconds_to_timestr (double sec, bool decimals=true);

/**
 * @brief tries to parse a string to unix time
 * @param strtimedate a string to parse
 * @param result_epoch_usec is written here; microseconds since 01.01.1970 1am (UNIX time)
 * @return true on success, else false (then result_epoch_usec) is invalid.
 */
bool string_to_epoch_usec(const std::string & strtimedate, uint64_t & result_epoch_usec);

/**
 * @brief turn a timestruct to an integer holding microseconds since UNIX time
 * @param tm
 * @return
 */
uint64_t tm_to_epoch_usec(const struct tm * const tm);

/**
 * @brief this function returns time since unix epoch in microseconds,
 * based on GPS epoch
 * @param gps_week_ms GPS time (milliseconds from start of GPS week)
 * @param gps_week GPS week number
 * @return
 *
 * Info:
 * - GPS epoch is time since 06. Jan 1980
 * - Unix epoch is time since 01. Jan 1970
 *
 */
uint64_t gpsepoch2unixepoch_usec(uint16_t gps_week, uint32_t gps_week_ms);

/**
 * @brief get_time_secs
 * @return  time since unix epoch in seconds with decimals
 */
double get_time_secs(void);

#endif // TIME_FUN_H
