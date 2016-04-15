/**
 * @file dialogdatatable.h
 * @brief Show selected data row as table
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 12/21/2015
 
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

#ifndef DIALOGDATATABLE_H
#define DIALOGDATATABLE_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include "data.h"
#include "mavplot.h"

class DialogDataTable : public QDialog
{
    Q_OBJECT
public:
    explicit DialogDataTable(const Data*d, MavPlot*plot = 0, QWidget *parent = 0);
    void setData(const Data*d);

signals:
    
public slots:
    void on_btnOk_clicked(void);
    void on_btnExport_clicked(void);
    void on_tableSelectionChanged(void);

private:   
    /******************
     * METHODS
     ******************/
    void _buildDialog(void);
    void _buildTable(void);
    template <typename ST>
    void _buildTable_datatimeseries(const DataTimeseries<ST>* d);
    template <typename ST>
    void _buildTable_dataevent(const DataEvent<ST>* d);
    
    /******************
     * ATTRIBUTES
     ******************/
    const Data*_data;
    MavPlot*_plot;
    QTableWidget*_datatable;
    QLabel*_lblsum;


};

#endif // DIALOGDATATABLE_H
