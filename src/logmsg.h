/**
 * @file logmsg.h
 * @brief common types for logging
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 12/31/2015
 
    This file is part of MavLogAnalyzer, Copyright 2015 by Martin Becker.
    
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

#ifndef LOGMSG_H
#define LOGMSG_H

#include <sstream>
#include <string>

/**
 * @brief types of log messages
 */
typedef enum {
    MSG_DBG,
    MSG_INFO,
    MSG_WARN,
    MSG_ERR
} logmsgtype_e;

/**
 * @brief helper to convert stringstream to string on the fly
 */
struct stringbuilder {
   std::stringstream ss;
   template<typename T>
   stringbuilder & operator << (const T &data)    {
        ss << data;
        return *this;
   }
   operator std::string() { return ss.str(); }
};

#endif // LOGMSG_H
