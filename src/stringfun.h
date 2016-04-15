/**
 * @file stringfun.h
 * @brief String functions.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 19.04.2014
 
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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
 */

#ifndef STRINGFUN_H
#define STRINGFUN_H

#include <vector>
#include <string>
#include <sstream>
#include <set>

// trim from start
std::string &string_ltrim(std::string &s, bool linebreak=false);

// trim from end
std::string &string_rtrim(std::string &s, bool linebreak=false);

// trim from both ends
std::string &string_trim(std::string &s, bool linebreak=false);

// split/explode
std::vector<std::string> & string_split(const std::string &s, char delim, std::vector<std::string> &elems);

// remove all newlines
void string_oneline(std::string &buf);

// generic ", ".join(std::set< >)
template <typename T>
std::string set2str(std::set<T> s) {
    std::stringstream ss;

    bool first = true;
    for (typename std::set<T>::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (!first) {
            ss << ", ";
        } else {
            first = false;
        }
        ss << *it;
    }
    return ss.str();
}

#endif // STRINGFUN_H
