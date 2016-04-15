/**
 * @file datatreeviewmodel.cpp
 * @brief Model for viewing data of one mav system.
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

#include <algorithm>
#include "datatreeviewmodel.h"
#include "treeitem.h"

/*
 * Some explanation: Our model here has only one column (=0), and multiple rows.
 * row numbering is alway per level and parent, e.g.;
 *
 *  node A (row 0, col 0)
 *    |--- node A.1 (row 0, col 0)
 *    |--- node A.2 (row 1, col 0)
 *  node B (row 1, col 0)
 *    |--- node B.1 (row 0, col 0)
 *
 *  By the way...this QAbstractItemModel is *HORRIBLE*. We want multi-parent data, without duplicating it.
 *  Why the hell do we need parent()? Furthermore, providing indizes is even more horrible, if siblings
 *  are of different data types....grml!
 */

/**
 * @brief DataTreeViewModel::DataTreeViewModel
 * @param parent
 * @param analyzer
 */
DataTreeViewModel::DataTreeViewModel(QObject *parent, MavlinkScenario*analyzer) : QAbstractItemModel(parent), _sys(NULL) {
    if (!analyzer) {
        valid = false;
        _analyzer = NULL;        
        return;
    }
    _analyzer = analyzer;        
    valid = true;
}

int DataTreeViewModel::rowCount(const QModelIndex &parent) const {
    /* simply returns the number of child items for the TreeItem
     *that corresponds to a given model index, or the number of
     *top-level items if an invalid index is specified
     */
    if (!_sys) return 0;
    if (!parent.isValid()) return _sys->mav_data_groups.size();

    TreeItem*t = static_cast<TreeItem*>(parent.internalPointer());
    if (TreeItem::DATA == t->itemtype) return 0; // DATA has no children

    // we have a group
    DataGroup*gparent = dynamic_cast<DataGroup*>(t);
    unsigned int ngroups = gparent->groups.size();
    unsigned int ndata = gparent->data.size();
    return ndata + ngroups;
}

int DataTreeViewModel::columnCount(const QModelIndex &/*parent*/) const {
    if (!_sys) return 0;
#if 0
    TreeItem * t = static_cast<TreeItem*>(parent.internalPointer());
    if (TreeItem::GROUP == t->itemtype) {
        DataGroup*p = dynamic_cast<DataGroup*>(t);
        return 1;
    } else {
        Data*p = dynamic_cast<Data*>(t);
        return 2;
    }
#else
    return 1;
#endif
}

void DataTreeViewModel::set_mav_sys(const MavSystem *const sys) {
    _sys = sys;
    layoutChanged(); ///< signal redraw
}

QVariant DataTreeViewModel::data(const QModelIndex &index, int role) const {
    if (!_sys) return QVariant();
    if (!index.isValid()) return QVariant();
    if (role != Qt::DisplayRole) return QVariant();

    TreeItem * t = static_cast<TreeItem*>(index.internalPointer());
    if (TreeItem::GROUP == t->itemtype) {
        DataGroup*p = dynamic_cast<DataGroup*>(t);
        return QString().fromStdString(p->groupname);
    } else {
        Data*p = dynamic_cast<Data*>(t);
        return QString().fromStdString(p->get_name());
    }

    // this is a mess...columns need extra indizes.

}

Qt::ItemFlags DataTreeViewModel::flags(const QModelIndex &index) const {    
    if (!index.isValid()) return 0;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant DataTreeViewModel::headerData(int /*section*/, Qt::Orientation /* orientation */, int /*role*/) const {
    if (!_sys) return QVariant();
    return QString("Titel");
}

QModelIndex DataTreeViewModel::index(int row, int column, const QModelIndex &parent) const {
    if (!_sys) return QModelIndex();

    // ignore column. all columns get their information from the same data class...therefore all columns get the same ptr.
    // return groups
    if (parent.isValid()) {
        // i am the child of another DataGroup.
        TreeItem * t = static_cast<TreeItem*>(parent.internalPointer());
        if (!t) return QModelIndex();
        DataGroup*gparent = dynamic_cast<DataGroup*>(t);
        if (!gparent) return QModelIndex();
        if ((unsigned int)row < (gparent->groups.size())) {
            // -- get 'row'th element in group map
            DataGroup::groupmap::const_iterator it = gparent->groups.begin();
            advance(it, row);
            // -- return pointer to it
            DataGroup*tmp = it->second;
            return createIndex(row, column, dynamic_cast<TreeItem*>(tmp)); // internalPointer = DataGroup*
        } else if ((unsigned int)row >= gparent->groups.size() && (unsigned int)row < (gparent->groups.size() + gparent->data.size())) {
            // return 'row'th element in data map
            TreeItem * t = static_cast<TreeItem*>(parent.internalPointer());
            if (!t) return QModelIndex();
            DataGroup*gparent = dynamic_cast<DataGroup*>(t);
            if (!gparent) return QModelIndex();
            unsigned int newrow = row - gparent->groups.size();
            if (newrow >= gparent->data.size()) return QModelIndex(); // invalid...do not have that many.
            DataGroup::datamap::const_iterator it = gparent->data.begin();
            advance(it, newrow);
            // -- return pointer to it
            Data*tmp = it->second;
            return createIndex(row, column, dynamic_cast<TreeItem*>(tmp)); // internalPointer = Data*
        } else {
            return QModelIndex(); // invalid...do not have that many.
        }

    } else {
        // no parent == root node. always a group, data cannot be on that level. find the 'row'th DataGroup in selected MavSystem
        DataGroup::groupmap const *gparent = &_sys->mav_data_groups;
        if (!gparent) return QModelIndex();
        if ((unsigned int)row >= gparent->size()) return QModelIndex(); // invalid...do not have that many.
        DataGroup::groupmap::const_iterator it = gparent->begin();
        advance(it, row);
        // -- return pointer to it
        DataGroup*tmp = it->second;
        return createIndex(row, column, dynamic_cast<TreeItem*>(tmp)); // internalPointer = DataGroup*
    }

    return QModelIndex();
}

QModelIndex DataTreeViewModel::parent(const QModelIndex &index) const {
    if (!_sys) return QModelIndex();
    if (!index.isValid()) return QModelIndex();

    // hm..if index==data, then it has a parent. but if index==data group, not necessarily.
    TreeItem*item = static_cast<TreeItem*>(index.internalPointer());                
    if (!item) return QModelIndex();
    if (TreeItem::GROUP == item->itemtype) {
        DataGroup*tmp = dynamic_cast<DataGroup*>(item);
        if (!tmp) return QModelIndex();
        if (tmp->parent) {
            // again...find the freaking row of parent
            DataGroup*p = tmp->parent;
            if (!p) return QModelIndex();
            unsigned int idx=0;
            for (DataGroup::groupmap::const_iterator it= p->groups.begin(); it != p->groups.end(); ++it) {
                if (it->second == tmp) {                    
                    return createIndex(idx, 0, tmp->parent);
                }
                idx++;
            }
            return QModelIndex();
        } else {
            return QModelIndex(); // root..no parent
        }
    } else {
        Data*tmp = dynamic_cast<Data*>(item);
        if (!tmp) return QModelIndex();
        return createIndex(index.row(), 0, tmp->parent);
    }

    return QModelIndex();
}
