/**
 * @file onboardlogparserulg.cpp
 * @brief Parser for ULOG file format
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 7/20/2017
 * FIXME this file is full of unhandled corner cases

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
#include <cassert>
#include "stringfun.h"
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

#if 0
/**
 * @brief Message type = INFO
 */
struct message_info_s {
    uint8_t key_len;
    char    key[key_len];
    char    value[header.msg_size-hdr_size-1-key_len];
};
#endif

using namespace std;

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


/**
 * @brief consume format message
 * @param siz
 * @return true on success, else false
 */
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

    string typname = str_format.substr(0, pos);
    string fields = str_format.substr(pos + 1);
    _register_format(typname, fields);
    return true;
}


/**
 * @brief consume PARAM message
 * @param siz
 * @return true on success, else false
 */
bool OnboardLogParserULG::_get_defs_param(uint16_t siz) {
    // TODO: not just skip
    //_log(MSG_DBG, stringbuilder() << "PARM ignored");
    _filebuf.pubseekoff(siz, ios_base::cur, ios_base::in);
    return true;
}

/**
 * @brief consume flags message
 * @param siz
 * @return true on success, else false
 */
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

/**
 * @brief consume the data type definitions (composite type defs etc.)
 * @return true on success, else error
 */
bool OnboardLogParserULG::_get_defs(void) {

    const ulog_message_header_t*const head = (ulog_message_header_t*) _buffer;
    for(;;) {
        int n = _filebuf.sgetn(_buffer, ULOG_MSG_HEADER_LEN);
        if (n < ULOG_MSG_HEADER_LEN) return false; // is it?

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

        case (int) ADD_LOGGED_MSG: ///< indicates end of definitions
            _log(MSG_DBG, stringbuilder() << "ADD_LOGGED_MSG");
            // seek back by header length
            _filebuf.pubseekoff(-ULOG_MSG_HEADER_LEN, ios_base::cur, ios_base::in);
            return true;
            break;

        case (int) INFO: // fallthrough
        case (int) INFO_MULTIPLE: // fallthrough
            //_log(MSG_DBG, stringbuilder() << "INFO");
            // SKIP THESE. FIXME: don't skip.
            _filebuf.pubseekoff(head->msg_size, ios_base::cur, ios_base::in);
            break;

        default:
            _log(MSG_ERR, stringbuilder() << "Unknown message in def section: " << head->msg_type);
            return false;
            break;
        }
    }

    return true;
}

/**
 * @brief find and consume the ULOG header
 * @return true if all ok, else false
 */
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
    _buflen = 0;
}

/**
 * @brief helper function for logging to GUI
 */
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

    ulog_message_header_t*head = (ulog_message_header_t*) _buffer;
    _buflen = head->msg_size; // FIXME: endianness?
    typ = head->msg_type;
    if (_buflen == 0) return false;

    n = _filebuf.sgetn(_buffer, _buflen);
    if (n < (int)_buflen) return false;
    _buffer[_buflen] = 0;

    //std::cout << "Reading log message " << typ << ": len=" << _buflen << std::endl;

    return true;
}

/**
 * @brief read next data message into _buffer.
 * Silently consumes header & definitions.
 * @return true on success, false on error (e.g., eof)
 */
bool OnboardLogParserULG::_get_next_message(int&typ) {
    if (!valid) return false;

    bool ret;
    /* We comsume complete header and defs first... */
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

    /* ... now it is time to consume the data itself and return one msg at a time */
    return _get_log_message(typ);
}

bool is_padding (const string  & str) {
    return str.substr(0, strlen("_padding")) == "_padding";
}

#define READ_FUNCTION(funcname, type, target) \
    static int funcname (const char*const buf, const std::string &nam, OnboardData &data) { \
        if (!is_padding(nam)) { \
            data.target [nam] = *((type*)buf); /*FIXME: endianness fails if host=big*/ \
        } \
        return sizeof(type); \
    }

READ_FUNCTION(read_int8,   int8_t,   _int_data)
READ_FUNCTION(read_uint8,  uint8_t,  _uint_data)
READ_FUNCTION(read_int16,  int16_t,  _int_data)
READ_FUNCTION(read_uint16, uint16_t, _uint_data)
READ_FUNCTION(read_int32,  int32_t,  _int_data)
READ_FUNCTION(read_uint32, uint32_t, _uint_data)
READ_FUNCTION(read_int64,  int64_t,  _int_data)
READ_FUNCTION(read_uint64, uint64_t, _uint_data)
READ_FUNCTION(read_float,  float,    _float_data)
READ_FUNCTION(read_double, double,   _float_data)

/**
 * @brief find read function corresponding to data type
 * @return ptr to function, or NULL if unknown
 */
Read_Field_Function OnboardLogParserULG::_get_field_reader (const std::string & fieldtype) {
    if (fieldtype == "uint8_t" ||
        fieldtype == "bool" ||
        fieldtype == "char")
        return &read_uint8;
    if (fieldtype == "int8_t") return &read_int8;
    if (fieldtype == "uint16_t") return &read_uint16;
    if (fieldtype == "int16_t") return &read_int16;

    if (fieldtype == "uint32_t") return &read_uint32;
    if (fieldtype == "int32_t") return &read_int32;

    if (fieldtype == "uint64_t") return &read_uint64;
    if (fieldtype == "int64_t") return &read_int64;

    if (fieldtype == "float") return &read_float;
    if (fieldtype == "double") return &read_double;
    return NULL;
}

template <typename Iter, typename Cont>
bool is_last(Iter iter, const Cont& cont)
{
    return (iter != cont.end()) && (++iter == cont.end());
}

/**
 * @brief see whether given type spec is an array
 * @param fieldtype
 * @return number of elements. 1=no array, n=array with n elements. In the
 * latter case, the fieldtype is changed to element type (chopped []).
 * If numbner of elements is zero, then the fieldtype is malformed.
 */
unsigned int OnboardLogParserULG::_find_array_spec (std::string & fieldtype) {
    unsigned int elems = 1;
    {
        size_t posarr_a = fieldtype.find('[');
        if (posarr_a != string::npos) {
            // array
            size_t posarr_b = fieldtype.find(']');
            if (posarr_b == string::npos || posarr_a >= posarr_b) {
                return 0;
            }
            const std::string strelems = fieldtype.substr(posarr_a+1, posarr_b - posarr_a - 1);
            elems = atoi (strelems.c_str());
            if (elems < 1) {
                return 0;
            }

            // chop [] from fieldtype
            fieldtype = fieldtype.substr(0, posarr_a);
        }
    }
    return elems;
}

void OnboardLogParserULG::_register_format
(const std::string & name, const std::string & strfields)
{
    format_t fmt;
    fmt.datalen = 0;

    /* fields should be "<field>(;<field>)*;" */
    vector<string> fields;
    string_split(strfields, ';', fields);
    if (fields.empty()) {
        _log (MSG_ERR, stringbuilder() << "Ignoring empty FMT for " << name);
        return;
    }

    OnboardData trashdata;
    char        trashbuf [8];

    for (vector<string>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
        const string & fieldspec = *it;        
        /* each <field> is "<fieldtype>(\[array length\])? <fieldname>" */
        {
            size_t pos = fieldspec.find(' ');
            if (pos == string::npos) {
                _log (MSG_ERR, stringbuilder() << "Malformed field in FMT for " << name);
                return;
            }

            const string fieldname = fieldspec.substr(pos + 1);
            string fieldtype = fieldspec.substr(0, pos);

            // trailing padding is not being logged. TODO: for nested defs, we cannot just skip it
            if (is_padding(fieldname) && is_last(it, fields)) {
                continue;
            }

            /* see if we got an array */
            unsigned int elems = _find_array_spec (fieldtype);
            if (elems == 0) {
                _log (MSG_ERR, stringbuilder() <<
                      "Malformed array field '" << fieldname << "' in FMT for " << name);
                return;
            }

            /* TODO: consolidate char[] to string */

            /* register field while unrolling arrays, if any */
            for (unsigned int e=0; e<elems;++e) {
                field_t      field;
                unsigned int flen = 0;

                if (elems > 1) {
                    stringstream ss;
                    ss << fieldname;
                    ss << "_" << e;
                    field.name = ss.str();
                } else {
                    field.name = fieldname;
                }

                // TODO: They also use types that they defined in defs, and furthermore types can be used before they are defined.

                // set up callback and accumulate msg length
                field.decode = _get_field_reader (fieldtype);
                if (NULL == field.decode) {
                    _log (MSG_ERR, stringbuilder() <<
                          "Unsupported field type '" << fieldtype <<
                          "' in field '" << fieldname << "' in FMT for " << name);
                    return;
                }

                // dry run to find field length
                flen = field.decode (trashbuf, field.name, trashdata);
                fmt.fields.push_back(field);

                fmt.datalen += flen;
            }
        }
    }
    _formats [name] = fmt;
    _log (MSG_DBG, stringbuilder() << "FMT: " << name <<", len=" << fmt.datalen << ", fmt=" << strfields);    
}

void OnboardLogParserULG::_register_message_id
(const std::string & name, uint8_t /*multi_id*/, uint16_t msg_id)
{
    _log(MSG_DBG, stringbuilder() << "Msg type: " << name << ", id=" << msg_id);

    name_map_t::const_iterator it = _message_name.find (msg_id);
    if (it != _message_name.end()) {
        // check if same
        const std::string & name_existing = it->second;
        if (name_existing != name) {
            _log (MSG_WARN, stringbuilder() << "Conflicting message for id " <<
                  msg_id << ": " << name_existing << " vs. " << name);
        } else {
            // duplicate, ignore
        }
    } else {
        _message_name [msg_id] = name; // ignore multi_id
    }
}

bool OnboardLogParserULG::_decode_str_msg (uint16_t msglen, OnboardData & ret) {
    uint8_t lvl = (uint8_t)_buffer[0];
    int n = read_uint64(_buffer + 1, "timestamp", ret);
    assert (n==8);
    const unsigned int STRING_OFF = 9;
    const unsigned int elems = msglen - STRING_OFF;

    string strlvl = "UNKNOWN";
    switch (lvl) {
        case '0': strlvl = "EMERG"; break;
        case '1': strlvl = "ALERT"; break;
        case '2': strlvl = "CRIT"; break;
        case '3': strlvl = "ERR"; break;
        case '4': strlvl = "WARN"; break;
        case '5': strlvl = "NOTICE"; break;
        case '6': strlvl = "INFO"; break;
        case '7': strlvl = "DEBUG"; break;
        default: break;
    }

    ret._msgname_orig = "messages";
    ret._msgname_readable = _make_readable_name (ret._msgname_orig);
    ret._string_data [strlvl] = string(_buffer + STRING_OFF, elems);
    ret._valid = true;
    return true;
}

/**
 * @brief UNTESTED
 * @return true if successfully parsed, else false
 */
bool OnboardLogParserULG::_decode_info_msg (uint16_t msglen, OnboardData & ret) {
    const uint8_t keylen = _buffer[0];
    string strkey (_buffer+1, keylen);

    size_t pos = strkey.find(' ');
    if (pos == string::npos) {
        return false;
    }

    string typname = strkey.substr(0, pos);
    const string desc = strkey.substr(pos + 1);

    unsigned int elems = _find_array_spec(typname);
    if (elems == 0) {
        _log (MSG_ERR, stringbuilder() <<
              "Malformed array field in INFO '" << desc << "' message");
        return false;
    }

    // good to parse now
    char*read = _buffer + 1 + keylen;
    if (typname == "char") {
        assert (elems == msglen - keylen - 1); // not sure about this one. doc does not say what value elems would be carrying
        ret._string_data [desc] = string(read, elems);
    } else if (typname == "uint32_t") {
        for (unsigned int e=0; e<elems; ++e) {
            read += read_uint32 (read, desc, ret);
        }
    } else if (typname == "int32_t") {
        for (unsigned int e=0; e<elems; ++e) {
            read += read_uint32 (read, desc, ret);
        }
    } else {
        _log (MSG_ERR, stringbuilder() <<
              "Unsupported type of info message '" << desc << "'");
        return false;
    }
    ret._msgname_orig = "info";
    ret._msgname_readable = _make_readable_name (ret._msgname_orig);
    ret._valid = true;
    return true;
}

/**
 * @brief read _buffer and decode data message
 * @return true on success, else false
 */
bool OnboardLogParserULG::_decode_data_msg(uint16_t msg_id, OnboardData & ret) {
    name_map_t::const_iterator it_name = _message_name.find (msg_id);
    if (it_name == _message_name.end()) return false;

    // find format spec
    const std::string & message_name = it_name->second;
    format_map_t::const_iterator it_fmt = _formats.find (message_name);
    if (it_fmt == _formats.end()) return false;
    const format_t & fmt = it_fmt->second;

    // message is already in _buffer
    // first two bytes are the msg_id
    const unsigned int DATA_OFF = 2;
    assert (_buflen == fmt.datalen + DATA_OFF);

    // decode fields one by one
    char*readptr = _buffer + DATA_OFF; // data starts after msg_id
    for (std::vector<field_t>::const_iterator it = fmt.fields.begin(); it != fmt.fields.end(); ++it) {
        const field_t & f = *it;
        assert (NULL != f.decode);
        readptr += f.decode (readptr, f.name, ret);
        assert (readptr <= _buffer + fmt.datalen + DATA_OFF);
    }
    assert (readptr == _buffer + fmt.datalen + DATA_OFF);

    // set message name et. al
    ret._msgname_orig = message_name;
    ret._msgname_readable = _make_readable_name (ret._msgname_orig);
    ret._valid = true;

    return true;
}

OnboardData OnboardLogParserULG::get_data(void) {
    OnboardData ret;
    if (!valid) return ret;

    int typ;
    if (_get_next_message(typ)) {

        // _buffer now contains the message only

        switch (typ) {
        case (int) ADD_LOGGED_MSG: // 65
            {
                string topic_name(_buffer + 3);
                uint16_t msg_id = ((uint16_t) _buffer[1]) | (((uint16_t) _buffer[2]) << 8);
                uint8_t  multi_id = (uint8_t) _buffer[0];
                _register_message_id (topic_name, multi_id, msg_id);
            }
            break;

        case (int)DATA: // 68
            {
                uint16_t msg_id = ((uint16_t) _buffer[0]) | (((uint16_t) _buffer[1]) << 8);
                if (!_decode_data_msg (msg_id, ret)) {
                    _log (MSG_ERR, stringbuilder() << "Cannot decode message with id " << msg_id);                    
                    return OnboardData();
                }
            }
            break;

        case (int)INFO:
            {
                if (!_decode_info_msg (_buflen, ret)) {
                    _log (MSG_ERR, stringbuilder() << "Cannot decode info message");
                    return OnboardData();
                }
            }
            break;

        case (int)LOGGING:
            {
                if (!_decode_str_msg (_buflen, ret)) {
                    _log (MSG_ERR, stringbuilder() << "Cannot decode string (logging) message");
                    return OnboardData();
                }
            }
            break;

        case (int)SYNC://fallthrough
        case (int)REMOVE_LOGGED_MSG://fallthrough
        case (int)PARAMETER://fallthrough
        case (int)INFO_MULTIPLE://fallthrough       
        case (int)DROPOUT://fallthrough
            // skip;
            break;

        default:
            _log (MSG_ERR, stringbuilder() << "Unknown message type " << typ);
            return OnboardData();
            assert (false); // must not happen
            break;
        }
    }

    return ret;
}

bool OnboardLogParserULG::has_more_data(void) {
    if (!valid) return false;

    return (_filebuf.sgetc() != std::char_traits<char>::eof());
}
