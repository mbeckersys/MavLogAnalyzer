/**
 * @file OnboardLogParserAPM_apm.h
 * @brief can parse *.log files with plain text logs as produced by MissionPlanner
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

#ifndef OnboardLogParserAPMAPM_H
#define OnboardLogParserAPMAPM_H

#include <vector>
#include <map>
#include <stdio.h>
#include <inttypes.h>
#include <string>
#include <csv_parser/csv_parser.hpp>
#include "onboardlogparser.h"
#include "logger.h"

class OnboardData; ///< forward decl

class OnboardLogParserAPM : public OnboardLogParser
{
public:   
    /****************************************
     *     PUBLIC FUNCTIONS
     ****************************************/
    OnboardLogParserAPM(std::string filename, Logger::logchannel * ch = NULL);
    ~OnboardLogParserAPM();

    const std::string& get_filename(void) const;

    // implements OnboardLogParser::has_more_data
    bool has_more_data(void);

    // implements OnboardLogParser::get_data
    OnboardData get_data(void);

    /****************************************
     *     DATA MEMBERS
     ****************************************/    

    // how fields and stuff are separated in the log file
    static const char enclosure_char   = '"';
    static const char field_terminator = ',';
    static const char line_terminator  = '\n';

private:
    /****************************************
     *     METHODS
     ****************************************/    
    bool _parse_message(const csv_row & row, OnboardData & data);

    /****************************************
     *     DATA MEMBERS
     ****************************************/
    // general
    std::string _filename;
    csv_parser  _parser;
    Logger::logchannel*_logchannel;

    // types
    typedef char datatype;
    typedef std::pair<std::string, datatype> colformat; ///< (column name, data type)
    typedef std::vector<colformat> lineformat; ///< each entry is one column. So this maps index to column format

    // hash table for formats
    std::map<std::string, lineformat> _formats; ///< maps line ID -> lineformat

};

#endif // OnboardLogParserAPMAPM_H
