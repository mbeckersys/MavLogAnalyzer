/**
 * @file onboardlogparserpx4.cpp
 * @brief pase binary logs of PX4 onboard flash
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 1/20/2016
 
    This file is part of MavLogAnalyzer, Copyright 2016 by Martin Becker.
    
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
#include <inttypes.h>
#include "onboardlogparser_px4.h"
#include "stringfun.h"

/*
 * Message format:
 *
 * ---------------------------------
 * | A3 | 95 | TYPE| ...           |
 * ---------------------------------
 * \------v-------/ \-------v-------/
 *      header            payload
 *
 * TYPES:
 */
#define FMT 0x80
#define PX4_HEAD1 0XA3
#define PX4_HEAD2 0X95
#define PX4_HEADERLEN 3

// more types are defined by message FMT itself...

using namespace std;

OnboardLogParserPX4::OnboardLogParserPX4() : _filename(""), _logchannel(NULL), _consumed(0)
{
    // bootstrap
    _register_fmt(0x80, 89, "FMT", "BBnNZ", "Type,Length,Name,Format,Labels");
}

OnboardLogParserPX4::~OnboardLogParserPX4() {
    _filebuf.close();
}

std::string OnboardLogParserPX4::_filebuf_get_string(unsigned int len) {
    std::string ret;
    for (unsigned int k=0; k < len; k++) {
        int c = _filebuf.sbumpc();
        if (char_traits<char>::eof() == c) break;
        ret += (char(c));
        _consumed++;
    }
    return ret;
}

void OnboardLogParserPX4::_log(logmsgtype_e t, const std::string & str) {
    if (_logchannel) {
        Logger::Instance().write(t, "OnboardLogParserPX4: "  + str, *_logchannel);
    } else {
        if (t == MSG_ERR || t == MSG_WARN) {
            cerr << "OnboardLogParserPX4: " << str << endl;
        } else {
            cout << "OnboardLogParserPX4: " << str << endl;
        }
    }
}

void OnboardLogParserPX4::_register_fmt(int typ, int len, const std::string & name, const std::string & informat, const std::string & fields) {
    msgformat fmt;
    fmt.name = name;
    fmt.length = len;

    vector<string> labels;
    string_split(fields, ',', labels);

    std::string format = informat;
    string_trim(format);

    if (labels.size() != format.size()) {
        _log(MSG_WARN, stringbuilder() << "_register_fmt: ignoring message "<< typ << " because format spec is broken (labels=" << labels.size() << ", fmt(" << format.size() << ")=" << format << ")");
        for (unsigned int o=0; o<format.size(); o++) {
            cout << int(format[o]) << " ";
        }
        cout <<  endl;
        return;
    }

    for (unsigned int k=0; k<format.length(); k++) {
        char typechar = format.at(k);
        std::string fname = labels.at(k); string_trim(fname);
        fieldformat ff = make_pair(fname, typechar);
        fmt.format.push_back(ff);
    }

    _log(MSG_DBG, stringbuilder() << "OnboardLogParserPX4::new message type: " << name << ", id=" << typ << ", len=" << len << ", " << format << ", " << fields);
    _formats.insert(std::make_pair(typ, fmt));
}

void OnboardLogParserPX4::_parse_message(const msgformat & fmt, OnboardData& ret) {
    if (fmt.length < PX4_HEADERLEN) {
        ret._valid = false;
        return;
    }
    const unsigned int LEN = fmt.length - PX4_HEADERLEN;

    ret._msgname_orig = fmt.name; // original name can be used better to compare the message with the spec
    string_trim(ret._msgname_orig);
    ret._msgname_readable = _make_readable_name(ret._msgname_orig); // .. but to the user we want to show pretty names

    // we later check that we dont actually not read more than rem
    _filebuf_resetcount();

    // all fields
    for (unsigned f = 0; f < fmt.format.size(); f++) {
        const std::string fieldlabel = fmt.format[f].first;
        const char fieldtype = fmt.format[f].second;

        // demux into OnboardData: FIXME: check types (could differ from APM)
        switch (fieldtype) {
        case 'i': // int32_t - OK
        {
            int32_t v; _filebuf_get(v);
            ret._int_data[fieldlabel] = v;
        }
            break;

        case 'h': // int16_t - OK
        {
            int16_t v; _filebuf_get(v);
            ret._int_data[fieldlabel] = v;
        }
            break;

        case 'Q': // Uint64
        {
            uint64_t v; _filebuf_get(v);
            ret._uint_data[fieldlabel] = v;
        }
            break;

        case 'q': //Int64
        {
            int64_t v; _filebuf_get(v);
            ret._int_data[fieldlabel] = v;
        }
            break;

        case 'b': // int8_t - OK
        {
            int8_t v; _filebuf_get(v);
            ret._int_data[fieldlabel] = v;
        }
            break;

        case 'I': // uint32_t - OK
        {
            uint32_t v; _filebuf_get(v);
            ret._uint_data[fieldlabel] = v;
        }
            break;

        case 'H': // uint16_t - OK
        {
            uint16_t v; _filebuf_get(v);
            ret._uint_data[fieldlabel] = v;
        }
            break;

        case 'M': // mode-string - OK
        case 'B': // uint8_t
        {
            uint8_t v; _filebuf_get(v);
            ret._uint_data[fieldlabel] = v;
        }
            break;

        case 'L': // int32_t -> but encodes a float - OK
        {
            int32_t v; _filebuf_get(v);
            float vf = v*1E-7;
            ret._float_data[fieldlabel] = vf;
        }
            break;

        case 'E': // uint32_t*100 - OK
        {
            uint32_t v; _filebuf_get(v);
            float vf = v*1E-2;
            ret._float_data[fieldlabel] = vf;
        }
            break;

        case 'e': // int32_t*100 - OK
        {
            int32_t v; _filebuf_get(v);
            float vf = v*1E-2;
            ret._float_data[fieldlabel] = vf;
        }
            break;

        case 'C': // uint16*100 - ok
        {
            uint16_t v; _filebuf_get(v);
            float vf = v*1E-2;
            ret._float_data[fieldlabel] = vf;
        }
            break;

        case 'c': // int16*100 -> but is given as float - OK
        {
            int16_t v; _filebuf_get(v);
            float vf = v*1E-2;
            ret._float_data[fieldlabel] = vf;
        }
            break;

        case 'f': // float - OK
        {
            float v; _filebuf_get(v);
            ret._float_data[fieldlabel] = v;
        }
            break;

        case 'Z': // char[64] - OK
        {
            std::string s = _filebuf_get_string(64);
            ret._string_data[fieldlabel] = s;
        }
            break;

        case 'N': // char[16] - OK
        {
            std::string s = _filebuf_get_string(16);
            ret._string_data[fieldlabel] = s;
        }
            break;

        case 'n': // char[4] - OK
        {
            std::string s = _filebuf_get_string(4);
            ret._string_data[fieldlabel] = s;
        }
            break;

        default:
            // unknown datatypes are ignored...
            _log(MSG_ERR, stringbuilder() << "OnboardLogParserPX4::_parse_message: unknown field type: "<< fieldtype <<"; rest of this message (" + fmt.name + ") might by garbage ");
            break;
        }
    }

    if (LEN != _filebuf_getcount()) {
        ret._valid = false;
        _log(MSG_ERR, stringbuilder() << "OnboardLogParserPX4::_parse_message: message \"" + fmt.name + "\" inconsistent. Ignoring it.");
    } else {
        ret._valid = true;
    }

}

// implement OnboardLogParser::get_data
OnboardData OnboardLogParserPX4::get_data(void) {
    OnboardData ret;
    if (!valid) return ret;

    // 1. try to read next message
    int typ;
    if (_get_next_message(typ)) {
        // see if we know the format and handle it

        formatmap::const_iterator it = _formats.find(typ);
        if (it != _formats.end()) {
            if (it->second.name == "FMT") {
                // defines more messages
                // FMT message is build like follows:
                int type = _filebuf.sbumpc();
                //_log(MSG_INFO, stringbuilder() << "OnboardLogParserPX4: got FMT (" << type <<")" );
                int length = _filebuf.sbumpc();
                std::string name = _filebuf_get_string(4);
                std::string fmt = _filebuf_get_string(16);
                std::string labels = _filebuf_get_string(64);
                _register_fmt(type, length, name, fmt, labels);
            } else {
                // is a message with data -> parse it
                //_log(MSG_INFO, stringbuilder() << "OnboardLogParserPX4: got data " << ((int)typ) );
                _parse_message(it->second,ret);
            }
        } else {
            _log(MSG_INFO, stringbuilder() << "OnboardLogParserPX4: unknown message type " << ((int)typ) );
        }
    }


    return ret;
}

/**
 * @brief OnboardLogParserPX4::_get_next_message
 * @param ret (out) message type
 * @return true if there is a message (scans entire file). when returned, then filebuf is positioned at first payload byte
 */
bool OnboardLogParserPX4::_get_next_message(int&ret) {
    if (!valid) return false;

    bool found = false;
    for(;;) {
        ret = _filebuf.sbumpc(); // read current and proceed -> at second header byte now
        if (PX4_HEAD1 == ret && PX4_HEAD2 == _filebuf.sgetc()) {
            found = true;
            break;
        }
        if (char_traits<char>::eof() == ret) break;
    }
    if (!found) return false;

    // return type
    ret = _filebuf.snextc(); // proceed and read -> ptr is now on the type (3rd header field)
    _filebuf.snextc(); // no points to first payload
    return true;
}

// implement OnboardLogParser::has_more_data
bool OnboardLogParserPX4::has_more_data(void) {
    if (!valid) return false;

    return (_filebuf.sgetc() != std::char_traits<char>::eof());
}

bool OnboardLogParserPX4::Load (std::string filename, Logger::logchannel * ch) {
    // initialize buffer
    _filename = filename;
    _logchannel = ch;
    _filebuf.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    valid = _filebuf.is_open();
    return valid;
}
