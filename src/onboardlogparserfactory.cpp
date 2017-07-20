/**
 * @file onboardlogparserfactory.cpp
 * @brief Factory for Onboard Log Parsers (returns a suitable one)
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

#include "onboardlogparserfactory.h"
#include "onboardlogparser_apm.h"
#include "onboardlogparser_px4.h"
#include "onboardlogparser_ulg.h"

OnboardLogParserFactory::OnboardLogParserFactory() {
    Register (OnboardLogParserPX4::get_extension(), &OnboardLogParserPX4::make_instance);
    Register (OnboardLogParserAPM::get_extension(), &OnboardLogParserAPM::make_instance);
    Register (OnboardLogParserULG::get_extension(), &OnboardLogParserULG::make_instance);
}

bool OnboardLogParserFactory::Register (const std::string & ext, Create_Parser_Function func) {
    Creator_Map::const_iterator exist = _map.find(ext);
    if (exist != _map.end()) return false;
    _map.insert (std::make_pair(ext, func));
    return true;
}

OnboardLogParser* OnboardLogParserFactory::Create (const std::string & file_ext) const {
    Creator_Map::const_iterator it = _map.find (file_ext);
    if (it != _map.end()) {
        OnboardLogParserFactory::Create_Parser_Function Make_One = it->second;
        return Make_One();
    } else {
        return NULL;
    }
}
