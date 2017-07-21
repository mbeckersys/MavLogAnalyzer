/**
 * @file dialogstats.cpp
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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <sstream>
#include "dialogstats.h"
#include "data.h"

DialogStats::DialogStats(const MavPlot * const plot, QWidget *parent) : QDialog(parent), _plot(plot) {
    _defineColumns();
    _buildDialog();    
    updateData();
}

void DialogStats::on_buttonOk_clicked() {
    this->close();
}

void DialogStats::on_buttonRefresh_clicked() {
    updateData();
}

void DialogStats::on_chkeval_range_changed(int /*state*/) {
    _evalData();
}

void DialogStats::on_spkeval_range_changed(double /*val*/) {
    _evalData();
}

// slow function
std::string DialogStats::_getColById(unsigned int id) {
    // we can only iterate....
    for (std::map<std::string, unsigned int>::iterator it =_columns.begin(); it != _columns.end(); ++it) {
        if (id == it->second) return it->first;
    }
    return "";
}

// fast function
int DialogStats::_getColByName(const std::string & s) {
    std::map<std::string, unsigned int>::iterator it =_columns.find(s);
    if ( it != _columns.end() ) {
        return (int)it->second;
    }
    return -1; // error
}

void DialogStats::_defineColumns(void) {
    // XXX! must be monotonic, starting at zero and free of gaps. apart from that, do whatever you want.
    _columns.clear();
    _columns["name"] = 0;
    _columns["#samples"] = 1;
    _columns["min"] = 2;
    _columns["avg"] = 3;
    _columns["stddev"] = 4;
    _columns["max"] = 5;
    _columns["range"] = 6;
    _columns["units"] = 7;
    _columns["freq"] = 8;
}

void DialogStats::_showProgressBar() {
    _progress.open();
    _progress.show();
}

void DialogStats::_updateProgressBarValue(unsigned int val, unsigned int max) {
    _progress.setValue(val, max);
    _progress.repaint();
}

void DialogStats::_hideProgressBar() {
    // hide it
    _progress.hide();
}

void DialogStats::_evalData(void) {
    for (int k=0; k<_table->rowCount(); k++) {
        QTableWidgetItem* row = _table->item(k, _getColByName("name"));
        if (!row) continue;
        row->setSelected(false);
        if (_chkevalrange->isChecked()) {
            bool succ;
            QTableWidgetItem* cell = _table->item(k, _getColByName("range"));
            if (!cell) continue;
            const double thisRange = cell->text().toDouble(&succ);
            if (!succ) continue;
            const double thrRange = _spevalrange->value();
            if (thisRange >= thrRange) {
                row->setSelected(true);
            }
        }
    }
}

void DialogStats::updateData(void) {
    _getData();    
    _updateTable();
    _evalData();
}

void DialogStats::_updateTable(void) {    
    _table->setRowCount(0);// will remove rows
    _table->setRowCount(_data.size());    

    QTableWidgetItem *header1 = new QTableWidgetItem();
    header1->setText("data series");   

    // t from plot
    QwtInterval xax = _plot->axisInterval(QwtPlot::xBottom);
    const double tmin = xax.minValue();
    const double tmax = xax.maxValue();

    _showProgressBar();
    unsigned int r=0;
    for (std::vector<Data const*>::const_iterator it = _data.begin(); it != _data.end(); ++it) {
        const Data * const d = *it;
        if (!d) continue;
        _table->setItem(r, _getColByName("name"), new QTableWidgetItem(d->get_fullname(d).c_str()));
        d->get_epoch_datastart();
        Data::data_stats s;
        if (d->get_stats_timewindow(tmin, tmax, s)) {
            _table->setItem(r, _getColByName("#samples"), new QTableWidgetItem(QString::number(s.n_samples)));
            _table->setItem(r, _getColByName("min"), new QTableWidgetItem(QString::number(s.min)));
            _table->setItem(r, _getColByName("avg"), new QTableWidgetItem(QString::number(s.avg)));
            _table->setItem(r, _getColByName("stddev"), new QTableWidgetItem(QString::number(s.stddev)));
            _table->setItem(r, _getColByName("max"), new QTableWidgetItem(QString::number(s.max)));
            _table->setItem(r, _getColByName("range"), new QTableWidgetItem(QString::number(fabs(s.max - s.min))));
            _table->setItem(r, _getColByName("freq"), new QTableWidgetItem(QString::number(fabs(s.freq))));
        } else {
            _table->setItem(r, _getColByName("min"), new QTableWidgetItem("stats failed"));
        }
        _table->setItem(r, _getColByName("units"), new QTableWidgetItem((*it)->get_units().c_str()));        
        r++;
        _updateProgressBarValue(r, _data.size());
    }        
    _table->sortByColumn(0);
    _table->sortItems(0);
    _table->repaint();
    std::stringstream ss;
    ss << "time window from " << _plot->getReadableTime(tmin).toStdString() << " to " << _plot->getReadableTime(tmax).toStdString() << ", including " << r << " data series";
    _lblsummary->setText(ss.str().c_str());
    _hideProgressBar();
}

void DialogStats::_getData(void) {
    // first get all data that is in the plot
    _data.clear();
    for (MavPlot::dataplotmap::const_iterator it = _plot->_series.begin(); it != _plot->_series.end(); ++it) {
        _data.push_back(it->first);
    }
}

void DialogStats::_buildDialog(void) {
    // build structure
    QVBoxLayout*v = new QVBoxLayout(this);
    QLabel*lblHeading = new QLabel(this); // by connecting to "this", DialogDataDetails cleans it up in the end
    lblHeading->setText("<b>Statistics of data in selected time range:</b>");
    v->addWidget(lblHeading); // that should re-parent

    // table
    _table = new QTableWidget(this);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    v->addWidget(_table);
    QStringList labels;
    _table->setColumnCount(_columns.size());
    for (unsigned int k=0; k<_columns.size(); k++) {
        labels << QString::fromStdString(_getColById(k));
    }
    _table->setHorizontalHeaderLabels(labels);

    _lblsummary = new QLabel(this);
    _lblsummary->setText("Computing...");
    v->addWidget(_lblsummary);

    // -- eval section
    QGroupBox*gb = new QGroupBox(this);
    v->addWidget(gb);
    gb->setTitle("Quick Eval");
    QVBoxLayout*gbv = new QVBoxLayout(gb);
    // first: eval range
    QHBoxLayout*gbh1 = new QHBoxLayout();
    gbv->addLayout(gbh1);
    _chkevalrange = new QCheckBox(this);
    gbh1->addWidget(_chkevalrange);
    _chkevalrange->setText("Mark data with range larger than:");
    _spevalrange = new QDoubleSpinBox(this);
    _spevalrange->setMinimum(0.);
    _spevalrange->setSingleStep(0.01);
    gbh1->addWidget(_spevalrange);


    // -- OK & Co.
    QHBoxLayout*hOk = new QHBoxLayout(); // parent set in next line
    v->addLayout(hOk);
    QPushButton*btnRefresh = new QPushButton(this);
    hOk->addWidget(btnRefresh);
    btnRefresh->setText("Refresh");
    QPushButton*btnOK = new QPushButton(this);
    hOk->addWidget(btnOK);
    btnOK->setText("OK");

    // signals
    connect(btnOK, SIGNAL(clicked()), SLOT(on_buttonOk_clicked()));
    connect(btnRefresh, SIGNAL(clicked()), SLOT(on_buttonRefresh_clicked()));
    connect(_chkevalrange, SIGNAL(stateChanged(int)), SLOT(on_chkeval_range_changed(int)));
    connect(_spevalrange, SIGNAL(valueChanged(double)), SLOT(on_spkeval_range_changed(double)));
    setWindowTitle("Statistics of selection");
    resize(550, 400);

    _progress.setLabel("Calculating stats...");
    _progress.setParent(this);
}
