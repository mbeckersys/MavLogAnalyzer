/**
 * @file stringfun.cpp
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

#include <sstream>
#include <algorithm>
#include "stringfun.h"

using namespace std;

static int _iscarriagereturn(int c) {
    return (c == '\r');
}

static int _isnull(int c) {
    return (c == 0);
}

string &string_ltrim(string &s, bool linebreak) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
    if (linebreak) s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(_iscarriagereturn))));
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(_isnull))).base(), s.end());
    return s;
}

// trim from end
string &string_rtrim(string &s, bool linebreak) {
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
    if (linebreak) s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(_iscarriagereturn))).base(), s.end());
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(_isnull))).base(), s.end());
    return s;
}

// trim from both ends
string &string_trim(string &s, bool linebreak) {
    return string_ltrim(string_rtrim(s, linebreak), linebreak);
}

vector<string> & string_split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void string_oneline(std::string &str) {
    // windows
    while ( str.find ("\r\n") != string::npos ) {
        str.erase ( str.find ("\r\n"), 2 );
    }
    // linux
    while ( str.find ("\n") != string::npos ) {
        str.erase ( str.find ("\n"), 2 );
    }
}
