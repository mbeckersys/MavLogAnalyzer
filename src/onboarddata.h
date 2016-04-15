/**
 * @file onboarddata.h
 * @brief This class represents a data item that is logged to the onboard data flash
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

#ifndef ONBOARDDATA_H
#define ONBOARDDATA_H

#include <inttypes.h>
#include <string>
#include <map>

class OnboardLogParserAPM; ///< forward decl for friendship
class OnboardLogParserPX4;

class OnboardData
{
public:

    /**********************************
     *  TYPES
     **********************************/
    typedef std::map<std::string, float> floatdata;
    typedef std::map<std::string, int64_t> intdata;
    typedef std::map<std::string, uint64_t>  uintdata;
    typedef std::map<std::string, bool> booldata;
    typedef std::map<std::string, std::string> stringdata;

    /**********************************
     *  METHODS
     **********************************/
    OnboardData() : _valid(false) {}

    /**
     * @brief only if this returns true, this class carries actual data
     * @return
     */
    bool is_valid(void) const { return _valid; }

    /**
     * @brief returns a string describing the collection of data this class contains
     * @return string
     */
    std::string get_message_origname(void) const { return _msgname_orig; }
    std::string get_message_name(void) const { return _msgname_readable; }

    /**
     * @brief access the data
     * @return
     */
    const floatdata & get_floatdata(void) const { return _float_data; }
    const intdata & get_intdata(void) const { return _int_data; }
    const uintdata & get_uintdata(void) const { return _uint_data; }
    const booldata & get_booldata(void) const { return _bool_data; }
    const stringdata & get_stringdata(void) const { return _string_data; }

private:
    /**********************************
     *  VARIABLES
     **********************************/    
    bool _valid;

    std::string _msgname_orig;  ///< msg name as in the log, e.g. "CTUN"
    std::string _msgname_readable; ///< a more readable string, e.g. "Controller Tuning"

    // colums by type. first=col name, second=value
    floatdata  _float_data;
    intdata    _int_data;
    uintdata   _uint_data;
    booldata   _bool_data;
    stringdata _string_data;

    // those who can write this class are friends (friendship cannot be inherited...need to name all implementations of OnboardLogParser here)
friend class OnboardLogParserAPM;
friend class OnboardLogParserPX4;
};

#endif // ONBOARDDATA_H
