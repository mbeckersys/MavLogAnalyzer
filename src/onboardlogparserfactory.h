/**
 * @file onboardlogparserfactory.h
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

#ifndef ONBOARDLOGPARSERFACTORY_H
#define ONBOARDLOGPARSERFACTORY_H

#include "onboardlogparser.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

/**
 * @brief registers and produces onboard parsers
 */
class OnboardLogParserFactory {
public:    

    typedef OnboardLogParser* (*Create_Parser_Function)(void);

    /**
     * @brief Meyer singleton getter
     */
    static OnboardLogParserFactory& Instance()
      { static OnboardLogParserFactory F; return F; }

    /// returns true if the registration succeeded, false otherwise
    bool Register (const std::string & ext, Create_Parser_Function func);

    /**
     * @return  suitable parser for extension, or NULL
     */
    OnboardLogParser* Create (const std::string & file_ext) const;

private:
    OnboardLogParserFactory();
    OnboardLogParserFactory (OnboardLogParserFactory&);
    OnboardLogParserFactory& operator= (OnboardLogParserFactory const&);

    typedef std::map<std::string, Create_Parser_Function> Creator_Map;
    Creator_Map _map;
};

#endif // ONBOARDLOGPARSERFACTORY_H
