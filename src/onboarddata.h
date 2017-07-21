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


class OnboardData
{
public:

    /**********************************
     *  TYPES
     **********************************/
    typedef std::map<std::string, float> floatdata_t;
    typedef std::map<std::string, int64_t> intdata_t;
    typedef std::map<std::string, uint64_t>  uintdata_t;
    typedef std::map<std::string, bool> booldata_t;
    typedef std::map<std::string, std::string> stringdata_t;

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
    const floatdata_t & get_floatdata(void) const { return _float_data; }
    const intdata_t & get_intdata(void) const { return _int_data; }
    const uintdata_t & get_uintdata(void) const { return _uint_data; }
    const booldata_t & get_booldata(void) const { return _bool_data; }
    const stringdata_t & get_stringdata(void) const { return _string_data; }


public:
    /**********************************
     *  VARIABLES (that should be private)
     **********************************/
    bool _valid;

    std::string  _msgname_orig;  ///< msg name as in the log, e.g. "CTUN"
    std::string  _msgname_readable; ///< a more readable string, e.g. "Controller Tuning"

    // colums by type. first=col name, second=value
    floatdata_t  _float_data;
    intdata_t    _int_data;
    uintdata_t   _uint_data;
    booldata_t   _bool_data;
    stringdata_t _string_data;

};

#endif // ONBOARDDATA_H
