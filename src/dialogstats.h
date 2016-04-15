/**
 * @file dialogstats.h
 * @brief Show statistics of data in currently selected time window
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 08.02.2015
 
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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
 */

#ifndef DIALOGSTATS_H
#define DIALOGSTATS_H

#include <QDialog>
#include <QTableWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <vector>
#include "data.h"
#include "mavplot.h"
#include "dialogprogressbar.h"

class MavPlot; ///< forward decl

class DialogStats : public QDialog
{
    Q_OBJECT
public:
    DialogStats(const MavPlot * const plot, QWidget *parent = 0);
    void updateData(void);

private slots:
    void on_buttonOk_clicked();
    void on_buttonRefresh_clicked();
    void on_chkeval_range_changed(int state);
    void on_spkeval_range_changed(double val);

private:
    void _getData(void);
    void _evalData(void);
    void _defineColumns(void);
    void _buildDialog(void);
    void _updateTable(void);
    std::string _getColById(unsigned int id);
    int _getColByName(const std::string & s);
    void _showProgressBar();
    void _updateProgressBarValue(unsigned int val, unsigned int max);
    void _hideProgressBar();

    // -- Column defs
    std::map<std::string, unsigned int> _columns; // map heading to column index

    // -- DATA

    const MavPlot*_plot;
    std::vector<Data const*> _data;
    QTableWidget* _table;
    DialogProgressBar _progress;
    QLabel*_lblsummary;

    // quick eval
    QCheckBox*_chkevalrange;
    QDoubleSpinBox*_spevalrange;
};

#endif // DIALOGSTATS_H
