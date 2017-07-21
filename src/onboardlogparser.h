/**
 * @file onboardlogparser.h
 * @brief virtual template class for onboard log parsers
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

#ifndef ONBOARDLOGPARSER_H
#define ONBOARDLOGPARSER_H

#include <string>
#include "onboarddata.h"
#include "logger.h"

#include <vector>
#include <string>

/**
 * @brief The OnboardLogParser class
 */
class OnboardLogParser
{
public:

    /**
     * @brief Load
     * @param filename
     * @param ch
     * @return
     */
    virtual bool Load (std::string filename, Logger::logchannel * ch = NULL) = 0;

    OnboardLogParser() : valid(false) {}
    virtual ~OnboardLogParser() {}

    /**
     * @brief get_data
     * @return class holding one data item. Use has_more_data before,
     * to see whether this returs a valid result
     */
    virtual OnboardData get_data(void) = 0;

    /**
     * @brief user can query whether there is more data to fetch
     * @return true if so
     */
    virtual bool has_more_data(void) = 0;

    /**
     * @brief parser must return list of supported file extensions
     * @return
     */
    //virtual std::string get_extension (void) = 0;

    /**
     * @brief return a new instance of my class
     */
    //virtual OnboardLogParser* make_instance (void) = 0;

    /****************************************
     *     DATA MEMBERS
     ****************************************/
    bool valid; //< indicates error


protected:
    std::string _make_readable_name(std::string msgname);
};

#endif // ONBOARDLOGPARSER_H
