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

#include "onboardlogparser_ulg.h"

OnboardLogParserULG::OnboardLogParserULG() {}

bool OnboardLogParserULG::Load (std::string filename, Logger::logchannel * ch) {
    // initialize buffer
    _filename = filename;
    _logchannel = ch;
    // TODO: open
    return valid;
}

OnboardData OnboardLogParserULG::get_data(void) {
    OnboardData ret;
    if (!valid) return ret;
    // TODO: parse
    return ret;
}

bool OnboardLogParserULG::has_more_data(void) {
    if (!valid) return false;

    return true; // TODO
}
