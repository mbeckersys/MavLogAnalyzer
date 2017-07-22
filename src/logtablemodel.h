/**
 * @file logtablemodel.h
 * @brief A model for table view to how log messages. Basically QStandardItemModel with minor extensions
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

#ifndef LOGTABLEMODEL_H
#define LOGTABLEMODEL_H

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <QStandardItemModel>
#include <QList>
#include "logmsg.h"

class LogTableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    /*********************
     * TYPEDEFS
     *********************/

    /*********************
     * METHODS
     *********************/
    explicit LogTableModel(QObject *parent = 0);
    ~LogTableModel();

    // refers to parent
    //int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    // refers to parent
    //int columnCount(const QModelIndex &parent = QModelIndex()) const;
    // this is extended to display DBG, INFO, ERR and so on nicely
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;


    /**
     * @brief add message to model (convenience)
     * @param e
     * @param typ
     * @param msg
     */
    void add_message(const std::string & emitter, logmsgtype_e typ, const std::string & msg);

    /**
     * @brief remove all messages from one emitter (convenience)
     * @param e
     */
    void remove_emitter(const std::string &);

    static QString type2str(logmsgtype_e t);
signals:
    
public slots:
private:

    QList<QStandardItem *> _prepareRow(const QString &first, const QString &second, const QString &third);    
    //QStandardItemModel _model;

    // data for the model        
};

#endif // LOGTABLEMODEL_H
