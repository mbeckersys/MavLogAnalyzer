/**
 * @file dialogdatatable.cpp
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

#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include "dialogdatatable.h"

DialogDataTable::DialogDataTable(const Data *d, MavPlot *plot, QWidget *parent) :
    QDialog(parent) , _data(d), _plot(plot) {
    _buildDialog();
    setData(d);
}

void DialogDataTable::setData(const Data*d) {    
    _data = d;
    _buildTable();
}

template <typename ST>
void DialogDataTable::_buildTable_dataevent(const DataEvent<ST>* d) {
    const std::vector<double> t = d->get_time();
    const std::vector<ST> s = d->get_data();
    unsigned int r = 0;
    for (unsigned int k=0; k<t.size(); k++, r++) {
        unsigned int c = 0;
        // time
        QTableWidgetItem*it = new QTableWidgetItem(QString::number(t[k]));
        _datatable->setItem(r,c++,it);
        // data value
        std::stringstream ss;
        ss << s[k];
        it = new QTableWidgetItem(QString::fromStdString(ss.str()));
        _datatable->setItem(r,c++,it);
    }
}

template <typename ST>
void DialogDataTable::_buildTable_datatimeseries(const DataTimeseries<ST>* d) {
    const std::vector<double> t = d->get_time();
    const std::vector<ST> s = d->get_data();
    unsigned int r = 0;
    for (unsigned int k=0; k<t.size(); k++, r++) {
        unsigned int c = 0;
        // time
        QTableWidgetItem*it = new QTableWidgetItem(QString::number(t[k]));
        _datatable->setItem(r,c++,it);
        // data value
        std::stringstream ss;
        ss << s[k];
        it = new QTableWidgetItem(QString::fromStdString(ss.str()));
        _datatable->setItem(r,c++,it);
    }
}

// shorthand for demuxing polymorphic data
#define TRY_BUILDTABLE_DATATIMESERIES(data, typetest) \
    if (const DataTimeseries<typetest>*tmp = dynamic_cast<const DataTimeseries<typetest> *>(data)) { \
        _buildTable_datatimeseries<typetest>(tmp); \
    }
#define TRY_BUILDTABLE_DATAEVENT(data, typetest) \
    if (const DataEvent<typetest>*tmp = dynamic_cast<const DataEvent<typetest> *>(data)) { \
        _buildTable_dataevent<typetest>(tmp); \
    }

/**
 * @brief DialogDataTable::_buildTable
 * fill the tableview based on _data
 */
void DialogDataTable::_buildTable(void) {
    _datatable->clear(); // remove all items; dims stay same
    _datatable->reset(); // remove internal view state
    if (!_data) {
        _lblsum->setText("no data selected");
        return;
    }

    // header & dim
    QStringList head = QString("Time;Data").split(";");
    const int ncols = head.size();
    _datatable->setColumnCount(ncols);
    _datatable->setHorizontalHeaderLabels(head);
    _datatable->setRowCount(_data->size());
    QString summary = QString::fromStdString(Data::get_fullname(_data)) + " (" + QString::number(_data->size()) + " samples)";

    // type-specific tabulation
    if (QString::fromStdString(_data->get_typename()).startsWith("data_timeseries")) {
        summary += " in units of " + QString::fromStdString(_data->get_units());
        TRY_BUILDTABLE_DATATIMESERIES(_data, int);
        TRY_BUILDTABLE_DATATIMESERIES(_data, long);
        TRY_BUILDTABLE_DATATIMESERIES(_data, float);
        TRY_BUILDTABLE_DATATIMESERIES(_data, double);
        TRY_BUILDTABLE_DATATIMESERIES(_data, unsigned int);
        TRY_BUILDTABLE_DATATIMESERIES(_data, unsigned long);
        // TRY_BUILDTABLE_DATATIMESERIES(_data, std::string); ///< string should not be a timeseries, but an event
    } else {
        TRY_BUILDTABLE_DATAEVENT(_data, bool);
        TRY_BUILDTABLE_DATAEVENT(_data, std::string);
    }
    _lblsum->setText(summary);
}

void DialogDataTable::on_btnOk_clicked(void) {
    this->close();
}

void DialogDataTable::on_tableSelectionChanged(void) {
    // which system within the scenario was selected?
    QItemSelectionModel* sel = _datatable->selectionModel();
    QModelIndexList selli  = sel->selectedRows();
    if (selli.empty()) return;
    int row = selli.first().row();
    if (row < 0) return;

    if (_plot) {
        (void) _plot->set_markerData(_data, (unsigned long) row);
    }
}

void DialogDataTable::on_btnExport_clicked(void) {

    QString defaultfilter = "Comma-Separated Values (*.csv)";
    QStringList filter;
    filter += defaultfilter;

    QString fileName = "data.csv";
    fileName = QFileDialog::getSaveFileName(this, "Export File Name", fileName, filter.join(";;"), &defaultfilter, QFileDialog::DontConfirmOverwrite);
    if ( fileName.isEmpty() ) return;

    const bool ret = _data->export_csv(fileName.toStdString());
    if (!ret) {
        QMessageBox msgbox(QMessageBox::Critical, "Export CSV", QString("Sorry, but the export failed. See console."));
        msgbox.exec();
    } else {
        QMessageBox msgbox(QMessageBox::Information, "Export CSV", QString("Successfully written file \""+ fileName + "\"."));
        msgbox.exec();
    }
}

void DialogDataTable::_buildDialog(void) {   
    // widgets
    QLabel*lbl = new QLabel(this);
    lbl->setText("Data Points:");
    _datatable = new QTableWidget(this);
    _datatable->setSelectionBehavior(QAbstractItemView::SelectRows); // entire row
    _datatable->setSelectionMode(QAbstractItemView::SingleSelection); // exactly one
    _datatable->horizontalHeader()->setStretchLastSection(true);

    QPushButton*btnOk = new QPushButton(this);
    btnOk->setText("Close");
    btnOk->setAutoDefault(true);
    QPushButton*btnExport = new QPushButton(this);
    btnExport->setText("Export...");
    _lblsum = new QLabel(this);

    // layout
    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(lbl);
    l->addWidget(_datatable);
    QHBoxLayout *lh = new QHBoxLayout;
    l->addWidget(_lblsum);
    lh->addWidget(btnOk);
    lh->addWidget(btnExport);
    l->addLayout(lh);
    setLayout(l);

    // window
    setWindowTitle(tr("Data Table"));
    setFixedHeight(sizeHint().height());
    setFixedWidth(sizeHint().width());

    // signals
    connect(btnOk, SIGNAL(clicked()), SLOT(on_btnOk_clicked()));
    connect(btnExport, SIGNAL(clicked()), SLOT(on_btnExport_clicked()));
    connect(_datatable, SIGNAL(itemSelectionChanged()), SLOT(on_tableSelectionChanged()));
}
