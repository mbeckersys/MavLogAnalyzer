/**
 * @file cmdlineargs.h
 * @brief Parses command line and keeps data.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 2014-Apr-17
 
 *  This file is part of MavLogAnalyzer, Copyright 2014 by Martin Becker.

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

#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <list>
#include <string>

class CmdlineArgs
{
public:
    CmdlineArgs(int argc, char**argv);

    /****************************************
     *   DATA MEMBERS
     ****************************************/
    std::list<std::string> filenames;
    bool valid;    ///< indicate whether parsing went well
    bool headless;  ///< start w/o GUI
    double time_maxjump_sec; ///< how much time is allowed to jump between two successive messages

    bool import;               ///Bernd: anaylize File to test DB-Import
private:
    int _parse(int argc, char**argv);
    void _print_usage(FILE * stream) const;
    static bool _file_exists(std::string filename);

    /****************************************
     *   DATA MEMBERS
     ****************************************/


};

#endif // CMDLINEARGS_H
