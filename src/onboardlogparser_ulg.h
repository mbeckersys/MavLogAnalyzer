/**
 * @file onboardlogparserulg.h
 * @brief TODO: describe this file
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 7/20/2017

    This file is part of MavLogAnalyzer, Copyright 2017 by Martin Becker.

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

#ifndef ONBOARDLOGPARSERULG_H
#define ONBOARDLOGPARSERULG_H

#include <fstream>
#include <inttypes.h>
#include <string>
#include <map>
#include "onboardlogparser.h"
#include "logger.h"

#define ULOG_BUFLEN 2048

/**
 * @brief implements a Ulog parser. See https://dev.px4.io/en/log/ulog_file_format.html
 * Closefly following PX4 firmware / replay (Apache 2.0 license).
 */
class OnboardLogParserULG : public OnboardLogParser
{
public:
    OnboardLogParserULG();
    ~OnboardLogParserULG();

    // implement OnboardLogParser::get_data
    OnboardData get_data(void);

    // implement OnboardLogParser::has_more_data
    bool has_more_data(void);

    static std::string get_extension(void) { return "ulg"; }

    // implement super
    bool Load (std::string filename, Logger::logchannel * ch = NULL);

    static OnboardLogParser* make_instance() { return new OnboardLogParserULG; }

private:

    /*******************
     * TYPES
     *******************/
    typedef enum  {
        FORMAT = 'F',
        DATA = 'D',
        INFO = 'I',
        INFO_MULTIPLE = 'M',
        PARAMETER = 'P',
        ADD_LOGGED_MSG = 'A',
        REMOVE_LOGGED_MSG = 'R',
        SYNC = 'S',
        DROPOUT = 'O',
        LOGGING = 'L',
        FLAG_BITS = 'B',
    } ULogMessageType;

    /*******************
     * METHODS
     *******************/
    bool _get_next_message(int&typ);
    bool _get_header(void);
    bool _get_defs(void);
    bool _get_defs_flagbits(uint16_t siz);
    bool _get_defs_format(uint16_t siz);
    bool _get_defs_addlog(uint16_t siz);
    bool _get_defs_param(uint16_t siz);
    bool _get_log_message(int&typ);
    void _log(logmsgtype_e t, const std::string & str);

    /*******************
     * ATTRIBUTES
     *******************/
    std::string        _filename;
    std::filebuf       _filebuf;
    Logger::logchannel*_logchannel;

    typedef enum state_s {WAIT_HEADER, WAIT_DEFS, WAIT_DATA} state_e;

    char         _buffer[ULOG_BUFLEN];
    unsigned int _buflen;
    state_e      _state;

    uint64_t     _read_until_file_position; ///< read limit if log contains appended data
    std::map<std::string, std::string> _file_formats; ///< all formats we read from the file
};

#endif // ONBOARDLOGPARSERULG_H
