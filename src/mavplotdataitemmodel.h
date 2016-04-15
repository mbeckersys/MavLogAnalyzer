/**
 * @file mavplotdataitemmodel.h
 * @brief The data model for the stuff stored in MavPlot
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 12/19/2015
 
    This file is part of MavLogAnalyzer, Copyright 2015 by Martin Becker.
    
    MavLogAnalyzer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
    
 */

#ifndef MAVPLOTDATAITEMMODEL_H
#define MAVPLOTDATAITEMMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <data.h>
#include <vector>
#include <map>

class MavplotDataItemModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MavplotDataItemModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void model_append(const Data *d);
    void model_setname(const Data *d, const std::string &name);
    void model_setcolor(const Data *d, const QColor &col);
    void model_remove(const Data *d);
    void model_clear(void);

signals:
    
public slots:

private:

    /***********************
     * TYPEDEFS
     ***********************/
    typedef struct data_props_s {
        std::string name;
        QColor color;
    } data_props_t;
    typedef std::map<const Data*, data_props_t> propsmap_t;
    typedef std::vector<const Data*> datalist_t;

    /***********************
     * METHODS
     ***********************/
    void _emitChanged(void);
    data_props_t _get_props(const Data*d) const;

    /***********************
     * ATTRIBUTES
     ***********************/
    datalist_t _data;
    propsmap_t _data_props;
    
};

#endif // MAVPLOTDATAITEMMODEL_H
