/**
 * @file dialogdatadetails.h
 * @brief Shows a summary of selected data, triggered from a plot.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 21.04.2014
 
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

#ifndef DIALOGDATADETAILS_H
#define DIALOGDATADETAILS_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <vector>
#include <qwt_plot_seriesitem.h>
#include <qwt_plot_item.h>
#include "qwt_compat.h"
#include "data.h"
#include "mavplotdataitemmodel.h"

class DialogDataDetails : public QDialog
{
    Q_OBJECT
public:
    explicit DialogDataDetails(const Data *const d,  QWT_ABSTRACT_SERIESITEM * const s, MavplotDataItemModel & model, QWidget *parent = 0);  // for curves
    explicit DialogDataDetails(const Data *const d,  std::vector<QwtPlotItem*> * const s, MavplotDataItemModel & model, QWidget *parent = 0); // for markers

signals:
    void removeDataFromPlot(const Data* const d);

private slots:
    void on_buttonRemoveData_clicked();
    void on_buttonOk_clicked();
    void on_buttonColor_clicked();
    void on_checkboxDataPoints_clicked(bool on);
    void on_buttonExport_clicked();
    void on_cbScale_Changed(int);
    void on_txtName_changed(QString);

private:
    void _buildDialog(const Data *const d);
    QString _get_suffix_from_scale(double scale);
    double _get_scale_from_title(QString title);
    QString _get_title_without_scale(QString title);

    /***************************
     *  MEMBER VARIABLES
     ***************************/
    const Data * _data;    
    MavplotDataItemModel &_model;
    QWT_ABSTRACT_SERIESITEM * _series;
    std::vector<QwtPlotItem*> * _items;
    QPushButton*btnCol;
    QCheckBox*cbMarkers;
    QComboBox*cbScale;
    QLineEdit*txtName;

};

#endif // DIALOGDATADETAILS_H
