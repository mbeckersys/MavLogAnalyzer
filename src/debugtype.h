/**
 * @file debugtype.h
 * @brief Opens and analyzes logfiles with MAVLink data from ardupilot.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 01.05.2015
 
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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
 */

#ifndef DEBUGTYPE_H
#define DEBUGTYPE_H

#include <string>
#include <typeinfo>

template <typename T>
const char*DEBUGTYPE(const T & var) {
#if 0
    // try to make it readable
    std::string tname = typeid(var).name();

    if (tname.compare("f")==0) {

    } else if () {

    }
#endif
    // unknown
    return typeid(var).name();
}

#endif // DEBUGTYPE_H
