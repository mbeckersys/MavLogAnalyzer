/**
 * @file onboardlogparserpx4.h
 * @brief TODO: describe this file
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

#ifndef ONBOARDLOGPARSERPX4_H
#define ONBOARDLOGPARSERPX4_H

#include <string>
#include <vector>
#include <fstream>
#include "onboardlogparser.h"
#include "logger.h"

class OnboardLogParserPX4 : public OnboardLogParser
{
public:
    OnboardLogParserPX4();
    ~OnboardLogParserPX4();

    // implement OnboardLogParser::get_data
    OnboardData get_data(void);

    // implement OnboardLogParser::has_more_data
    bool has_more_data(void);

    static std::string get_extension(void) { return "px4log"; }


    // implement super
    std::string get_parser_name(void) const { return "px4"; }

    // implement super
    bool Load (std::string filename, Logger::logchannel * ch = NULL);

    static OnboardLogParser* make_instance() { return new OnboardLogParserPX4; }
private:
    // types
    typedef char datatype;
    typedef std::pair<std::string, datatype> fieldformat; ///< (column name, data type)
    typedef struct {
        std::string name;
        int length; // length of this message in bytes, incl. header length
        std::vector<fieldformat> format; // fields in this message
    } msgformat;

    /*******************
     * METHODS
     *******************/
    bool _get_next_message(int &type);
    void _register_fmt(int typ, int len, const std::string & name, const std::string & format, const std::string & fields);    
    void _parse_message(const msgformat & fmt, OnboardData& ret);
    void _log(logmsgtype_e t, const std::string & str);

    // string reader
    std::string _filebuf_get_string(unsigned int len);

    // generic type reader
    template <typename T>
    void _filebuf_get(T &var) {
        // FIXME: endianness. assumes all is little
        const int LEN = sizeof(T);
        if (LEN <= 0) return;

        char*p = (char*)&var;
        for (int l=0; l<LEN; l++) {
            *p++ = (char)_filebuf.sbumpc();
        }

        // that doesn't work, because filebuf::pubseekoff() is not always seeking
        //_filebuf.sgetn((char*)&var, LEN);
        //_filebuf.pubseekoff(LEN, std::ios_base::cur,std::ios_base::in); // consume -> not working. why??!
        _consumed += LEN;
    }
    // count consumption
    unsigned int _filebuf_getcount(void) const {
        return _consumed;
    }
    // reset consumption counter
    void _filebuf_resetcount(void) {
        _consumed = 0;
    }


    /*******************
     * ATTRIBUTES
     *******************/
    std::string _filename;
    std::filebuf _filebuf;
    Logger::logchannel*_logchannel;
    unsigned int _consumed;

    // hash table for formats
    typedef std::map<int, msgformat> formatmap;
    formatmap _formats; ///< maps message ID -> format
};

#endif // ONBOARDLOGPARSERPX4_H
