/**
 * @file datagroup.h
 * @brief Allows hierarchical grouping of data. Does not own data, just points to it.
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

#ifndef DATAGROUP_H
#define DATAGROUP_H

#include <map>
#include <string>
#include "treeitem.h"

class Data; ///< forward decl
class DataGroup : public TreeItem {
public:
    DataGroup(std::string groupname);

    /* note we do not have a default or copy CTOR here, which means
     * the map cannot be accesses like `DataGroup me = mymap["groupname"];`.
     */


    typedef std::map<std::string,Data*> datamap;               ///< associative array: String => Data*
    typedef std::pair<std::string,Data*> datamap_pair;         ///< needed for inserting
    datamap data;                                              ///< data within this group
    typedef std::map<std::string,DataGroup*> groupmap;         ///< associative array: String => DataGroup*
    typedef std::pair<std::string,DataGroup*> groupmap_pair;   ///< needed for inserting
    groupmap groups;                                           ///< subgroups within this group (may again contain data, of course)    

    std::string groupname; ///< descriptive name of this group (this is what goes into the map of thir group's parents)
    DataGroup*parent;

    void set_parent(DataGroup*p) {
        parent = p;
    }

};

#endif // DATAGROUP_H
