/**
 * @file OnboardLogParserAPM.cpp
 * @brief can parse *.log files with plain text logs as produced by MissionPlanner for APM:Copter
 * or APMPlanner. These logs root from the onboard flash of the autopilot.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 11/12/2014
 
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

#include <iostream>
#include "stringfun.h"
#include "onboardlogparser_apm.h"

const std::string &OnboardLogParserAPM::get_filename(void) const {
    return _filename;
}

OnboardLogParserAPM::~OnboardLogParserAPM() {
}

OnboardLogParserAPM::OnboardLogParserAPM(std::string filename, Logger::logchannel *ch) : _filename(filename), _logchannel(ch) {
    valid = _parser.init(_filename.c_str());

    if (valid) {
        _parser.set_enclosed_char(enclosure_char, ENCLOSURE_NONE);
        _parser.set_field_term_char(field_terminator);        
        _parser.set_line_term_char(line_terminator);
    }

}

/**
 * @brief parse a csv row into a class.
 * @param row data is read from here.
 * @param data class instance. Data is written to here.
 * @return true if parsed, else false
 */
bool OnboardLogParserAPM::_parse_message(const csv_row & row, OnboardData & ret) {

    string msgname = row[0];
    string_trim(msgname);

    ret._msgname_orig = msgname; // original name can be used better to compare the message with the spec
    string_trim(ret._msgname_orig);
    ret._msgname_readable = _make_readable_name(ret._msgname_orig); // .. but to the user we want to show pretty names

    map<string, lineformat>::const_iterator rit = _formats.find(msgname);
    if (rit == _formats.end()) return false;

    /************
     * FOUND IT!
     ************/    

    /*
     * for parameters it is is different: [1]=name, [2]=value. We must not store "name" in
     * strings and "value" in float, but only in float and use [1] as the name
     */
    const bool is_param = (msgname.compare("PARM")==0);
    unsigned int k0 = is_param ? 2 : 1;

    if (row.size() > 1) {
        for (unsigned int k=k0; k<row.size(); k++) {
            const lineformat f = rit->second;
            unsigned int colidx = k-1;

            if (colidx >= f.size()) break;

            const colformat c = f[colidx];
            string colname;
            if (is_param) {
                colname = row[1];
            } else {
                colname = c.first;
            }
            string_trim(colname);

            // demux datatype
            switch (c.second) {
            case 'i': // int32_t
            case 'h': // int16_t
            case 'b': // int8_t
                ret._int_data[colname] = atoi(row[k].c_str());
                break;

            case 'I': // uint32_t
            case 'H': // uint16_t
            case 'B': // uint8_t
                ret._uint_data[colname] = ((unsigned int) atoll(row[k].c_str()));
                break;

            case 'L': // uint32_t -> but is given as float
            case 'E': // uint32_t*100 -> but is given as float
            case 'e': // int32_t*100 -> but is given as float
            case 'C': // uint16*100 -> but is given as float
            case 'c': // int16*100 -> but is given as float
            case 'f': // float
                ret._float_data[colname] = atof(row[k].c_str());
                break;

            case 'M': // mode-string
            case 'Z': // string
            case 'N': // char[16]
            default:
                // unknown datatypes are mapped as strings
                ret._string_data[colname] = row[k];
                break;
            }
        }
    }
    return true;
}

OnboardData OnboardLogParserAPM::get_data(void) {
    OnboardData ret;

    csv_row row = _parser.get_row();
    // now row is a vector<string>
    if (row.empty()) {
        return ret;
    }

    /*
     * first look up the type of data, then use polymorphism
     * to create the right class to return
     */
    if (row[0].compare("FMT")==0) {
        // these rows give column headers and data types. Do not return anything, but store it internally.
        // a line is formatted as follows
        // 0 -> FMT
        // 1 -> message ID
        // 2 -> message/row length
        // 3 -> message/row name
        // 4 -> data types for fields, encoded as format characters:
        /*
            +Format characters in the format string for binary log messages
            + b : int8_t -- OK
            + B : uint8_t -- OK
            + h : int16_t -- OK
            + H : uint16_t -- OK
            + i : int32_t -- OK
            + I : uint32_t -- OK
            + f : float -- OK
            + N : char[16] -- OK
            + c : int16_t * 100 -- OK
            + C : uint16_t * 100 -- OK
            + e : int32_t * 100 -- OK
            + E : uint32_t * 100
            + L : uint32_t latitude/longitude
            + Z : string (null-terminated?)
        */
        // 5+ -> labels for fields

        if (row.size() < 5) return ret;

        string formats = row[4];
        string_trim(formats, true);

        string rowname = row[3];
        string_trim(rowname);
        lineformat lf;
        if (row.size() > 5) { // labels of fields in message
            for (unsigned int k=5; k<row.size(); k++) {
                colformat cf;
                string colname = row[k]; string_trim(colname);
                const string typespec = formats.substr(k-5,1);
                const char typechar = typespec.c_str()[0];
                cf = make_pair(colname, typechar);
                lf.push_back(cf);
            }
        }
        _formats[rowname] = lf;
        return ret;
    } else {
        /*
         * Now all the rest here are rows which are described by FMT.
         * All we do is demux them into vectors of their respective data
         * types and label them according to column names.
         */
        ret._valid = _parse_message(row, ret);
    }
    // -- END
    if (!ret.is_valid()) {
        std::cerr << "OnboardLogParserAPM: unrecognized data row: " << row[0] << std::endl;
    }
    return ret;
}

bool OnboardLogParserAPM::has_more_data(void) {
    if (!valid) return false;
    return _parser.has_more_rows();
}
