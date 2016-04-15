/**
 * @file mavplotdataitemmodel.cpp
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

#include <algorithm>
#include "mavplotdataitemmodel.h"

MavplotDataItemModel::MavplotDataItemModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void MavplotDataItemModel::model_setname(const Data *d, const std::string & name) {
    if (!d) return;

    propsmap_t::iterator it = _data_props.find(d);
    if (it == _data_props.end()) return;
    it->second.name = name;

    _emitChanged();
}

void MavplotDataItemModel::model_setcolor(const Data *d, const QColor & col) {
    if (!d) return;

    propsmap_t::iterator it = _data_props.find(d);
    if (it == _data_props.end()) return;

    // set color
    it->second.color = col;
    _emitChanged();
}

void MavplotDataItemModel::_emitChanged(void) {
    // emit signal -> we don't know where we deleted...refresh all
    QModelIndex topleft = this->createIndex(0, 0);
    QModelIndex botright = this->createIndex(_data.size()-1, 0);
    emit dataChanged(topleft, botright);
}

void MavplotDataItemModel::model_remove(const Data *d) {
    if (!d) return;
    // find and erase
    datalist_t::iterator it = std::find(_data.begin(), _data.end(), d );
    if (it != _data.end()) {
        _data.erase(it);
        //props
        propsmap_t::iterator pit = _data_props.find(d);
        if (pit != _data_props.end()) {
            _data_props.erase(pit);
        }
        _emitChanged();
    } 
}


void MavplotDataItemModel::model_clear(void) {
    _data.clear();
    _data_props.clear();
    _emitChanged();
}

int MavplotDataItemModel::rowCount(const QModelIndex &/*parent*/) const  {
    return _data.size();
}

int MavplotDataItemModel::columnCount(const QModelIndex &/*parent*/) const {
    return 1;
}

MavplotDataItemModel::data_props_t MavplotDataItemModel::_get_props(const Data*d) const {
    data_props_t p;
    p.name = "unknown";
    if (!d) return p;

    propsmap_t::const_iterator pit = _data_props.find(d);
    if (pit != _data_props.end()) {
        p = pit->second; // deep copy
    }

    return p;
}

QVariant MavplotDataItemModel::data(const QModelIndex &index, int role) const {
    if (index.row() < ((int)0) || index.row() >= ((int)_data.size()))  return QVariant();
    const Data*d = _data[index.row()];
    data_props_t props = _get_props(d);

    switch (role) {
    case Qt::DisplayRole:            
        return QString::fromStdString(props.name);
        break;
    case Qt::ForegroundRole:
        return props.color;
        break;
    case Qt::UserRole:
        return qVariantFromValue((void*)d);
    default:
        // nada
        break;
    }
    return QVariant();
}

void MavplotDataItemModel::model_append(const Data *d) {
    if (!d) return;

    // check if in list, otherwise add
    datalist_t::iterator it = std::find(_data.begin(), _data.end(), d );
    if (it == _data.end()) {
        _data.push_back(d);
        // initial props
        data_props_t props;
        props.name = d->get_name();
        // TODO: color
        _data_props.insert(_data_props.end(), std::make_pair(d, props));
    }
    _emitChanged();
}
