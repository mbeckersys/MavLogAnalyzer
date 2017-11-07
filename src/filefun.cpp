/**
 * @file filefun.cpp
 * @brief Generic File functions
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 21.04.2014
 
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

#include <algorithm>
#include <limits.h>
#include <stdlib.h>
#ifdef _MSC_VER
    #include <io.h> // _access
    #define F_OK 0
    #ifndef access
        #define access _access
    #endif // !access
#else
    #include <unistd.h> // access, F_OK
#endif // !_MSC_VER

#include "filefun.h"

using namespace std;

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__) || _MSC_VER
    #define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#endif

string getFullPath(const string & filename) {
	char*c_fullpath = realpath(filename.c_str(), NULL);
	if (!c_fullpath) {
		return string("");
	}
	string fullpath (c_fullpath);
	free(c_fullpath);
	return fullpath;
}

string getDirname(const string & fullpath) {
    string dir;
    size_t posslash = fullpath.rfind('/');
    size_t posbslash = fullpath.rfind('\\');
    if (posslash != std::string::npos) {
        dir = fullpath.substr(0, posslash);
    } else if (posbslash != std::string::npos) {
        dir = fullpath.substr(0, posbslash);
    } else {
        dir = fullpath;
    }
    return dir;
}

string getExtension(const string & filename) {
    string ret;
    size_t pos = filename.find_last_of(".");
    if (pos != filename.npos) {
        ret = filename.substr(pos + 1);
    }
    return ret;
}

std::string &lcase(std::string &s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string getBasename(const string & fullpath) {
    string base;
    size_t posslash = fullpath.rfind('/');
    size_t posbslash = fullpath.rfind('\\');
    if (posslash != std::string::npos) {
        base = fullpath.substr(posslash + 1);
    } else if (posbslash != std::string::npos) {
        base = fullpath.substr(posbslash + 1);
    } else {
        base = fullpath;
    }
    return base;
}

bool fileExists(std::string &path) {
    return (access(path.c_str(), F_OK ) != -1);
}
