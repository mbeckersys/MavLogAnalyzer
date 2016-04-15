/**
 * @file SystemTableViewModel.cpp
 * @brief Defines how the systems are shown in a table view
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 18.04.2014
 
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

#include "SystemTableviewModel.h"
#include <algorithm> // advance

SystemTableViewModel::SystemTableViewModel(QObject * /*parent*/, MavlinkScenario * analyzer) {
    if (!analyzer) {
        valid = false;
        _analyzer = NULL;
        return;
    }
    _analyzer = analyzer;
    valid = true;
}

int SystemTableViewModel::rowCount(const QModelIndex & /*parent*/) const {
    return _analyzer->_seen_systems.size();
}

int SystemTableViewModel::columnCount(const QModelIndex & /*parent*/) const {
    return 2; // id, type
}

QVariant SystemTableViewModel::data(const QModelIndex &index, int role) const {
    // get the "index"th element from analyzer
    MavlinkScenario::systemlist::iterator it = _analyzer->_seen_systems.begin();
    advance(it, index.row());


    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case 0: // id
                return QString("%1").arg(it->second->id);
            case 1: // type
                return QString::fromStdString(it->second->mavtype_str);
            default:
                return QString("");
            }
            break;

        case Qt::UserRole:
            {
                return it->first; // return ID of MavSystem
            }
        break;

        default:
        return QVariant();
    }
    return QVariant();
}

QVariant SystemTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole)     {
        if (orientation == Qt::Horizontal)  {
            switch (section)
            {
            case 0:
                return QString("ID");
            case 1:
                return QString("Type");
            default:
                return QString("");
            }
        }
    }
    return QVariant();
}
