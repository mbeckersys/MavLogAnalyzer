/**
 * @file logtablemodel.cpp
 * @brief A model for table view to how log messages
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 12/30/2015
 
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

#include <QFont>
#include <QBrush>
#include "logtablemodel.h"

#define NCOLS 3

LogTableModel::LogTableModel(QObject *parent) :  QStandardItemModel(parent) {            
    setColumnCount(NCOLS);
    setRowCount(0);
}

LogTableModel::~LogTableModel() {
    //nada
}

QList<QStandardItem *> LogTableModel::_prepareRow(const QString &first, const QString &second, const QString &third) {
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(first);
    rowItems << new QStandardItem(second);
    rowItems << new QStandardItem(third);
    return rowItems;
}

QString LogTableModel::type2str(logmsgtype_e t)  {
    switch (t) {
        case MSG_DBG: return "Debug";
        case MSG_INFO: return "Info";
        case MSG_WARN: return "Warning";
        case MSG_ERR: return "Error";
    default: return "Unknown";
    }
}

void LogTableModel::add_message(const std::string & emitter, logmsgtype_e typ, const std::string & msg) {
    // must find the existing root node for the emitter, or make new one
    QStandardItem*em = NULL;
    QList<QStandardItem*> lst  = findItems(QString::fromStdString(emitter));
    if (lst.empty()) {
        // have to add newly
        QList<QStandardItem *> newnode = _prepareRow(QString::fromStdString(emitter), "", "");
        invisibleRootItem()->appendRow(newnode);
        em = newnode.first();
    } else {
        em = lst.first();
    }

    // and this adds a child item to the found emitter
    em->appendRow(_prepareRow("", type2str(typ), QString::fromStdString(msg)));

    const int rownew = rowCount() - 1;
    QModelIndex topleft = this->createIndex(0, 0);
    QModelIndex botright = this->createIndex(rownew, columnCount()-1);
    emit dataChanged(topleft, botright);
}

/**
 * @brief remove all messages from one emitter
 * @param e
 */
void LogTableModel::remove_emitter(const std::string &emitter) {
    // TODO
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("emitter");
            case 1:
                return QString("category");
            case 2:
                return QString("message");
            }
        }
    }
    return QVariant();
}

QVariant LogTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    //int row = index.row();
    int col = index.column();

    switch(role){
    // text to show
    case Qt::DisplayRole:       
        return QStandardItemModel::data(index, Qt::DisplayRole);

    // font face
    case Qt::FontRole:
        if (!index.parent().isValid()) {
            // root nodes bold, others not
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        break;

    // coloring of background
    case Qt::BackgroundRole:    
        if (index.parent().isValid()) {
            // this is not a root node -> use data to decide type
            if (col == 1) {
                QString typ = QStandardItemModel::data(index, Qt::DisplayRole).toString();
                if (0 == typ.compare("Info")) {
                    QBrush bg(Qt::blue);
                    return bg;
                }
                if (0 == typ.compare("Debug")) {
                    QBrush bg(Qt::gray);
                    return bg;
                }
                if (0 == typ.compare("Warning")) {
                    QBrush bg(Qt::yellow);
                    return bg;
                }
                if (0 == typ.compare("Error")) {
                    QBrush bg(Qt::red);
                    return bg;
                }
            }
        }    
        break;

    // message ID
    case Qt::UserRole:
        return QStandardItemModel::data(index, Qt::UserRole);

    default:
        return QVariant();
    }
    return QVariant();
}
