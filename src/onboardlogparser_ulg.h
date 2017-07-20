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

#include "onboardlogparser.h"

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
     * ATTRIBUTES
     *******************/
    std::string _filename;
    Logger::logchannel*_logchannel;
};

#endif // ONBOARDLOGPARSERULG_H
