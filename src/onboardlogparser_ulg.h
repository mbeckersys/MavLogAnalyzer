/**
 * @file onboardlogparserulg.h
 * @brief Parser for ULOG file format
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
 * @brief callback function pointer for field/type readers
 */
typedef int (*Read_Field_Function)(const char*const buf, const std::string &name, OnboardData & data);

/**
 * @brief implements a Ulog parser. See https://dev.px4.io/en/log/ulog_file_format.html
 * Closefly following PX4 firmware / replay (Apache 2.0 license).
 */
class OnboardLogParserULG : public OnboardLogParser {
public:    

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
        FLAG_BITS = 'B', ///< Flag bitset message.
        FORMAT = 'F', ///< format definition for a single (composite) type that can be logged
        INFO = 'I', ///< information message
        INFO_MULTIPLE = 'M', ///< information message multi
        PARAMETER = 'P', ///< parameter message
        /* the following messages are those of the data section */
        DATA = 'D', ///< logged data
        ADD_LOGGED_MSG = 'A', ///< subscribe a message by name and give it an id
        REMOVE_LOGGED_MSG = 'R', ///< unsubscribe a message (not logged anymore)
        SYNC = 'S', ///< synchronization message
        DROPOUT = 'O', ///< mark a dropout (lost logging messages)
        LOGGING = 'L' ///< Logged string message
    } ULogMessageType;

    typedef struct field_s {
        std::string          name;
        Read_Field_Function  decode; ///< callback to decode field
    } field_t;

    typedef struct format_s {
        uint16_t             datalen;
        std::vector<field_t> fields;
    } format_t;

    typedef std::map<std::string, format_t> format_map_t;
    typedef std::map<uint8_t, std::string>  name_map_t;

    /*******************
     * METHODS
     *******************/
    bool _get_next_message(int&typ);
    bool _get_header(void);
    bool _get_defs(void);
    bool _get_defs_flagbits(uint16_t siz);
    bool _get_defs_format(uint16_t siz);
    bool _get_defs_param(uint16_t siz);
    bool _get_log_message(int&typ);
    void _register_format(const std::string & name, const std::string & strfields);
    void _register_message_id(const std::string & name, uint8_t, uint16_t msg_id);
    bool _decode_data_msg(uint16_t msg_id, OnboardData & ret);
    bool _decode_info_msg(uint16_t msglen, OnboardData & ret);
    bool _decode_str_msg(uint16_t msglen, OnboardData & ret);
    unsigned int _find_array_spec (std::string & fieldtype);
    Read_Field_Function _get_field_reader (const std::string & fieldtype);
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

    format_map_t _formats;
    name_map_t   _message_name;
};

#endif // ONBOARDLOGPARSERULG_H
