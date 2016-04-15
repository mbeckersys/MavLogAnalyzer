/**
 * @file datatreeviewmodel.h
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

#ifndef DATATREEVIEWMODEL_H
#define DATATREEVIEWMODEL_H

#include <QAbstractItemModel>
#include "mavlinkscenario.h"

// see http://qt-project.org/doc/qt-4.8/itemviews-simpletreemodel.html

class DataTreeViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    DataTreeViewModel(QObject *parent, MavlinkScenario  * analyzer);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void setScenario(MavlinkScenario  * analyzer) {        
        #if QT_VERSION >= 0x050000
            beginResetModel();
        #endif

        _analyzer = analyzer;
        _sys = NULL; // invalidate

        #if QT_VERSION >= 0x050000
            endResetModel();
        #else
            reset(); // qt4
        #endif
    }

    void reload() {
        layoutChanged();
    }

    /**
     * @brief used to switch the model between different systems
     * @param sys
     */
    void set_mav_sys(const MavSystem*const sys);

    /*************************************
     *    DATA MEMBERS
     *************************************/
    bool valid;

private:
    MavlinkScenario *_analyzer;
    const MavSystem*_sys;

signals:
    
public slots:
    
};

#endif // DATATREEVIEWMODEL_H
