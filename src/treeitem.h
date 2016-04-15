/**
 * @file treeitem.h
 * @brief we need this to allow handling DataGroup and Data uniformly, which is required to get them into treeview.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 20.04.2014
 
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

#ifndef TREEITEM_H
#define TREEITEM_H

class TreeItem {
public:        
    virtual ~TreeItem() {} ///< this class is the polymorphic base class, therefore it needs a virtual DTOR

    enum {GROUP, DATA} itemtype;    ///< set in CTOR, which one the implementing class is
};

#endif // TREEITEM_H
