/**
 * @file SystemTableviewModel.h
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

#ifndef SYSTEMTABLEVIEWMODEL_H
#define SYSTEMTABLEVIEWMODEL_H

#include <QAbstractTableModel>
#include "mavlinkscenario.h"

class SystemTableViewModel : public QAbstractTableModel
 {
    Q_OBJECT
public:
    SystemTableViewModel(QObject *parent, MavlinkScenario  * analyzer);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void reload() {
        layoutChanged();
    }
    void setScenario(MavlinkScenario  * scen) {
        _analyzer = scen;
        #if QT_VERSION >= 0x050000
            layoutChanged();
        #else
            reset(); // does not exist in Qt5
        #endif
    }

    /*************************************
     *    DATA MEMBERS
     *************************************/
    bool valid;
private:
    MavlinkScenario *_analyzer;
};

#endif // SYSTEMTABLEVIEWMODEL_H
