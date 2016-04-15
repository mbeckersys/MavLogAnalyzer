/**
 * @file mavlinkparser.cpp
 * @brief Allows to access a single Mavlink logfile
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 18.04.2014
 
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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
 */

#include "mavlinkparser.h"

MavlinkParser::MavlinkParser(std::string filename) : _fp(NULL), _filename(filename), _n_msg(0) {
    valid = _file_open();
}

MavlinkParser::~MavlinkParser() {
    if (_fp) {
        fclose(_fp);
        valid = false;
    }
}

bool MavlinkParser::get_next_msg(mavlink_message_t &buf) {
    if (!valid) return false;    

    // read byte by byte and ask whether it can be parsed...    
    uint8_t bytebuf;
    int chan=0;
    while(fread(&bytebuf, 1, 1, _fp) > 0) { // argh...inefficient
        if (mavlink_parse_char(chan, bytebuf, &buf, &_r_mavlink_status)) {
            _n_msg++;
            return true;
        }
    }
    return false; // nothing found
}

const mavlink_status_t * MavlinkParser::get_linkstats() const {
    return &_r_mavlink_status;
}

const std::string &MavlinkParser::get_filename() const {
    return _filename;
}

bool MavlinkParser::_file_open() {
    _fp = fopen(_filename.c_str(), "rb");
    if (!_fp) return false;
    return true;
}
