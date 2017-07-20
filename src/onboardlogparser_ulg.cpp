/**
 * @file onboardlogparserulg.cpp
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

#include <iostream>
#include "onboardlogparser_ulg.h"

/*
 * File format:
 * ------------
 * |  Header  |
 * ------------
 * |  Defs    |
 * ------------
 * |  Data    |
 * ------------
 *
 * HEADER:
 * ----------------------------------------------------------------------
 * | 0x55 0x4c 0x6f 0x67 0x01 0x12 0x35 | 0x01         | uint64_t       |
 * | File magic (7B)                    | Version (1B) | Timestamp (8B) |
 * ----------------------------------------------------------------------
 */
#define ULOG_HEADER_SIZE 16
#define ULOG_MAGIC {0x55, 0x4C, 0x6F, 0x67, 0x01, 0x12, 0x35}

// header for defs
#define ULOG_MSG_HEADER_LEN 3
typedef struct ulog_message_header_s {
    uint16_t msg_size;
    uint8_t  msg_type;
} ulog_message_header_t;

#define ULOG_INCOMPAT_FLAG0_DATA_APPENDED_MASK (1<<0)


using namespace std;

OnboardLogParserULG::OnboardLogParserULG() {}

bool OnboardLogParserULG::Load (std::string filename, Logger::logchannel * ch) {
    // initialize buffer
    _filename = filename;
    _logchannel = ch;
    _filebuf.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    valid = _filebuf.is_open();
    _state = WAIT_HEADER;
    _buflen = 0;
    _read_until_file_position = 1ULL << 60; ///< read limit if log contains appended data
    return valid;
}


bool OnboardLogParserULG::_get_defs_format(uint16_t siz) {
    char *format = (char *)_buffer;
    int n = _filebuf.sgetn(_buffer, siz);
    if (n < siz) {
        _log(MSG_ERR, stringbuilder() << "Log file incomplete");
        return false;
    }

    format[siz] = 0; // zero-terminate

    string str_format(format);
    size_t pos = str_format.find(':');

    if (pos == string::npos) {
        return false;
    }

    string name = str_format.substr(0, pos);

    _log(MSG_DBG, stringbuilder() << "FMT: " << name);

    string fields = str_format.substr(pos + 1);
    _file_formats[name] = fields;

    return true;
}


bool OnboardLogParserULG::_get_defs_param(uint16_t siz) {
    // TODO: not just skip
    _log(MSG_DBG, stringbuilder() << "PARM ignored");
    _filebuf.pubseekoff(siz, ios_base::cur, ios_base::in);
    return true;
}

bool OnboardLogParserULG::_get_defs_addlog(uint16_t siz) {
    // TODO: not just skip
    _filebuf.pubseekoff(siz, ios_base::cur, ios_base::in);
    return true;
}

bool OnboardLogParserULG::_get_defs_flagbits(uint16_t siz) {
    const int MSGLEN = 40;

    if (siz < MSGLEN) {
        _log(MSG_ERR, stringbuilder() << "Log contains improper flagbits message");
        return false;
    }

    int n = _filebuf.sgetn(_buffer, MSGLEN);
    if (n < MSGLEN) {
        _log(MSG_ERR, stringbuilder() << "Log file incomplete");
        return false;
    }

    uint8_t *message = (uint8_t *)_buffer;
    uint8_t *incompat_flags = message + 8;

    // handle & validate the flags
    bool contains_appended_data = incompat_flags[0] & ULOG_INCOMPAT_FLAG0_DATA_APPENDED_MASK;
    bool has_unknown_incompat_bits = false;

    if (incompat_flags[0] & ~0x1) {
        has_unknown_incompat_bits = true;
    }

    for (int i = 1; i < 8; ++i) {
        if (incompat_flags[i]) {
            has_unknown_incompat_bits = true;
        }
    }

    if (has_unknown_incompat_bits) {
        _log(MSG_ERR, stringbuilder() << "Log contains unknown incompat bits set. Refusing to parse");
        return false;
    }

    if (contains_appended_data) {
        uint64_t appended_offsets[3];
        memcpy(appended_offsets, message + 16, sizeof(appended_offsets));

        if (appended_offsets[0] > 0) {
            // the appended data is currently only used for hardfault dumps, so it's safe to ignore it.
            _log(MSG_ERR, stringbuilder() << "Log contains appended data. Replay will ignore this data");
            _read_until_file_position = appended_offsets[0];
        }
    }

    return true;
}

bool OnboardLogParserULG::_get_defs(void) {

    for(;;) {
        int n = _filebuf.sgetn(_buffer, ULOG_MSG_HEADER_LEN);
        if (n < ULOG_MSG_HEADER_LEN) return false; // is it?


        // TODO: consume def
        ulog_message_header_t*head = (ulog_message_header_t*) _buffer;
        switch (head->msg_type) {
        case (int) FLAG_BITS:
            if (!_get_defs_flagbits(head->msg_size)) return false;
            break;

        case (int) FORMAT:
            if (!_get_defs_format(head->msg_size)) return false;
            break;

        case (int) PARAMETER:
            if (!_get_defs_param(head->msg_size)) return false;
            break;

        case (int) ADD_LOGGED_MSG:
            _log(MSG_DBG, stringbuilder() << "ADD_LOGGED_MSG");
            // go back by header length
            _filebuf.pubseekoff(-ULOG_MSG_HEADER_LEN, ios_base::cur, ios_base::in);
            return true;
            break;

        case (int) INFO: // fallthrough
        case (int) INFO_MULTIPLE: // fallthrough
            _log(MSG_DBG, stringbuilder() << "INFO"); // FIXME: print txt
            // SKIP THESE
            _filebuf.pubseekoff(head->msg_size, ios_base::cur, ios_base::in);
            break;

        default:
            _log(MSG_ERR, stringbuilder() << "Unknown definition message: " << head->msg_type);
            return false;
            break;
        }
    }

    return true;
}

bool OnboardLogParserULG::_get_header(void) {
    const uint8_t magic[] = ULOG_MAGIC;

    for(;;) {
        if (_buflen >= ULOG_HEADER_SIZE) {
            memccpy(_buffer, _buffer+1, ULOG_HEADER_SIZE - 1, 1); // shift left if necessary
        }

        int c = _filebuf.sbumpc();
        if (char_traits<char>::eof() == c) return false;

        _buffer[_buflen++] = c;

        // complete?
        if (_buflen == ULOG_HEADER_SIZE && memcmp(_buffer, _buffer, sizeof(magic)) == 0) {
            return true; // now points to right after header
        }
    }
}

void OnboardLogParserULG::_log(logmsgtype_e t, const std::string & str) {
    if (_logchannel) {
        Logger::Instance().write(t, "OnboardLogParserULP: "  + str, *_logchannel);
    } else {
        if (t == MSG_ERR || t == MSG_WARN) {
            cerr << "OnboardLogParserPX4: " << str << endl;
        } else {
            cout << "OnboardLogParserPX4: " << str << endl;
        }
    }
}

/**
 * @brief read next msg to buffer, return typ and set _buflen
 * @param typ returns the header type
 * @return
 */
bool OnboardLogParserULG::_get_log_message(int&typ) {
    int n = _filebuf.sgetn(_buffer, ULOG_MSG_HEADER_LEN);
    if (n < ULOG_MSG_HEADER_LEN) return false;

    {
        ulog_message_header_t*head = (ulog_message_header_t*) _buffer;
        _buflen = head->msg_size; // FIXME: endianness?
        typ = head->msg_type;

        n = _filebuf.sgetn(_buffer, _buflen);
        if (n < (int)_buflen) return false;
        _buffer[_buflen] = 0;
    }
    return true;
}

/**
 * @return next data message; skips header and defs
 */
bool OnboardLogParserULG::_get_next_message(int&typ) {
    if (!valid) return false;

    bool ret;
    /*
     * We comsume complete header and defs first...
     */
    if (_state == WAIT_HEADER) {
        ret = _get_header();
        if (!ret) {
            valid = false;
            return false;
            _log(MSG_ERR, stringbuilder() << "Cannot find header");
        }
        _log(MSG_INFO, stringbuilder() << "Consumed header");
        _state = WAIT_DEFS;
    }
    if (_state == WAIT_DEFS) {
        ret = _get_defs();
        if (!ret) {
            valid = false;
            return false;
            _log(MSG_ERR, stringbuilder() << "Cannot read definitions");
        }
        _state = WAIT_DATA;
    }

    /* ... now it is time to consume the data and return one msg at a time */
    // first message is ADD_LOGGED_MSG

    return _get_log_message(typ);
}

OnboardData OnboardLogParserULG::get_data(void) {
    OnboardData ret;
    if (!valid) return ret;

    int typ;
    if (_get_next_message(typ)) {

        switch (typ) {
        case (int) ADD_LOGGED_MSG:
            {
                string topic_name(_buffer + 3);
                uint16_t subtyp = ((uint16_t) _buffer[1]) | (((uint16_t) _buffer[2]) << 8);
                _log(MSG_DBG, stringbuilder() << "ADD_LOGGED_MSG Message: " << topic_name << ", type=" << typ << ", subtyp=" << subtyp);
                // TODO: decode msg and return
            }
            break;
        case (int)DATA:
            {
                uint16_t subtyp = ((uint16_t) _buffer[1]) | (((uint16_t) _buffer[2]) << 8);
                _log(MSG_DBG, stringbuilder() << "DATA Message: type=" << typ << ", subtyp=" << subtyp);
                // TODO: decode msg and return
            }
            break;
        case (int)SYNC://fallthrough
        case (int)REMOVE_LOGGED_MSG://fallthrough
        case (int)PARAMETER://fallthrough
        case (int)INFO://fallthrough
        case (int)INFO_MULTIPLE://fallthrough
        case (int)LOGGING://fallthrough
            // skip;
            break;
        default:
            break;
        }
    }

    return ret;
}

bool OnboardLogParserULG::has_more_data(void) {
    if (!valid) return false;

    return (_filebuf.sgetc() != std::char_traits<char>::eof());
}
