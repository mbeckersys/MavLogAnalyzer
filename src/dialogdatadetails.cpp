/**
 * @file dialogdatadetails.cpp
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

#include "dialogdatadetails.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QColorDialog>
#include <qwt_symbol.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QComboBox>
#include <QFrame>
#include <QLineEdit>
#include "mavplot.h"

void DialogDataDetails::_buildDialog(const Data *const d) {
    // build structure
    QVBoxLayout*v = new QVBoxLayout(this);
    QLabel*lblHeading = new QLabel(this); // by connecting to "this", DialogDataDetails cleans it up in the end
    lblHeading->setText("Selected Data:");

    // text widget
    v->addWidget(lblHeading); // that should re-parent
    QTextBrowser*txt = new QTextBrowser(this);
    v->addWidget(txt);
    txt->setText(QString::fromStdString(d->describe_myself()));

    // rename option
    QHBoxLayout*h0 = new QHBoxLayout(); // parent set in next line
    v->addLayout(h0);
    QLabel*lblName = new QLabel(this);
    lblName->setText("Data Name:");
    h0->addWidget(lblName);
    txtName = new QLineEdit(this);
    h0->addWidget(txtName);

    // color picker
    QHBoxLayout*h = new QHBoxLayout(); // parent set in next line
    v->addLayout(h);
    QLabel*lblColor = new QLabel(this);
    lblColor->setText("Color: ");
    h->addWidget(lblColor);
    btnCol = new QPushButton(this);
    btnCol->setEnabled(false);
    h->addWidget(btnCol);

    // scaling
    QHBoxLayout*h1 = new QHBoxLayout(); // parent set in next line
    v->addLayout(h1);
    QLabel*lblscale = new QLabel(this);
    lblscale->setText("Scaling: ");
    h1->addWidget(lblscale);
    cbScale = new QComboBox(this);
    cbScale->addItem("original", QVariant(1.));
    cbScale->addItem("1/10", QVariant(10.));
    cbScale->addItem("1/100", QVariant(100.));
    cbScale->addItem("1/1000", QVariant(1000.));
    cbScale->addItem("1/10000", QVariant(10000.));
    h1->addWidget(cbScale);

    // show data points
    QHBoxLayout*hMarkers = new QHBoxLayout(); // parent set in next line
    v->addLayout(hMarkers);
    cbMarkers= new QCheckBox(this);
    hMarkers->addWidget(cbMarkers);
    cbMarkers->setText("Show Markers");

    // Export
    QPushButton*btnExport = new QPushButton(this);
    btnExport->setText("Export...");
    v->addWidget(btnExport);

    // -- OK & Co.
    QHBoxLayout*hOk = new QHBoxLayout(); // parent set in next line
    v->addLayout(hOk);
    QPushButton*btnOK = new QPushButton(this);
    QPushButton*btnRm = new QPushButton(this);
    hOk->addWidget(btnRm);
    hOk->addWidget(btnOK);
    btnOK->setText("OK");
    btnRm->setText("Remove from Plot");

    // signals
    connect(btnOK, SIGNAL(clicked()), SLOT(on_buttonOk_clicked()));
    connect(btnRm, SIGNAL(clicked()), SLOT(on_buttonRemoveData_clicked()));
    connect(btnCol, SIGNAL(clicked()), SLOT(on_buttonColor_clicked()));
    connect(cbMarkers, SIGNAL(clicked(bool)), SLOT(on_checkboxDataPoints_clicked(bool)));
    connect(btnExport, SIGNAL(clicked()), SLOT(on_buttonExport_clicked()));
    connect(cbScale, SIGNAL(currentIndexChanged(int)), SLOT(on_cbScale_Changed(int)));    
    connect(txtName, SIGNAL(textChanged(QString)), SLOT(on_txtName_changed(QString)));

    //setLayout(v);
    setWindowTitle(QString::fromStdString(d->get_name()));
}

DialogDataDetails::DialogDataDetails(const Data *const d,  QWT_ABSTRACT_SERIESITEM * const s, MavplotDataItemModel & model, QWidget *parent) : QDialog(parent), _data(d), _model(model) , _series(s), _items(NULL) {
    if (!d || !s ) return;

    _buildDialog(d);

    /***********************
     *  CURVES
     ***********************/
    const QwtPlotCurve*curve = dynamic_cast<const QwtPlotCurve*>(s);

    // try to get color from curve...default to white
    QColor currentcol(Qt::white);
    const QwtSymbol*currentsym=NULL;    
    if (curve) {
        currentcol = curve->pen().color();
        currentsym = curve->symbol();
        btnCol->setEnabled(true);
    }
    static const QString COLOR_STYLE("QPushButton { background-color : %1; }");
    btnCol->setStyleSheet(COLOR_STYLE.arg(currentcol.name()));

    // toggle show markers
    cbMarkers->setChecked(currentsym!=NULL);

    // select scale that currently is active
    if (curve) {
        double divisor = _get_scale_from_title(curve->title().text());
        for (int k = 0; k < cbScale->count(); k++) {
            if (cbScale->itemData(k).toDouble() == divisor) {
                cbScale->blockSignals(true);
                cbScale->setCurrentIndex(k);
                cbScale->blockSignals(false);
                break;
            }
        }
    } else {
        cbScale->setEnabled(false);
    }

    // get data title
    txtName->blockSignals(true);
    if (curve) {
        txtName->setText(_get_title_without_scale(curve->title().text()));
    } else {
        txtName->setText(QString::fromStdString(_data->get_name()));
    }
    txtName->blockSignals(false);
}

DialogDataDetails::DialogDataDetails(const Data *const d, std::vector<QwtPlotItem* > * const s, MavplotDataItemModel& model, QWidget *parent) : QDialog(parent), _data(d), _model(model), _series(NULL), _items(s) {
    if (!d || !s ) return;

    _buildDialog(d);

    /***********************
     *  MARKERS
     ***********************/
    QwtPlotMarker*mark = dynamic_cast<QwtPlotMarker*>(s->front());

    // try to get color from marker...default to white
    if (!s->empty()) {
        QColor currentcol(Qt::white);
        //const QwtSymbol*currentsym=NULL;
        if (mark) {            
            currentcol = mark->linePen().color();
            //currentsym = mark->symbol();
            btnCol->setEnabled(true);
        }
        static const QString COLOR_STYLE("QPushButton { background-color : %1; }");
        btnCol->setStyleSheet(COLOR_STYLE.arg(currentcol.name()));

        // toggle show markers
        //cbMarkers->setChecked(currentsym!=NULL);
    }
    cbScale->setEnabled(false); // markers have no scale
    cbMarkers->setEnabled(false); // markers have no point-markers

    // get data title
    txtName->blockSignals(true);
    if (mark) {
        txtName->setText(_get_title_without_scale(mark->title().text()));
    } else {
        txtName->setText(QString::fromStdString(_data->get_name()));
    }
    txtName->blockSignals(false);
}

void DialogDataDetails::on_buttonColor_clicked() {
    // color picker
    QColorDialog *col = new QColorDialog(this);
    QColor usercolor = col->getColor();
    if (usercolor.isValid()) {
        static const QString COLOR_STYLE("QPushButton { background-color : %1; }");
        btnCol->setStyleSheet(COLOR_STYLE.arg(usercolor.name()));
        if (_series) {
            // try to set color
            QwtPlotCurve*curve = dynamic_cast<QwtPlotCurve*>(_series);
            if (curve) {
                curve->setPen(QPen(usercolor));
            }
        }
        if (_items) {
            for (std::vector<QwtPlotItem*>::iterator it = _items->begin(); it != _items->end(); ++it) {
                QwtPlotMarker*mark = dynamic_cast<QwtPlotMarker*>(*it);
                if (mark) {
                    mark->setLinePen(QPen(usercolor, 0, Qt::DashDotLine));
                }
            }
        }
        // call model and set new name
        _model.model_setcolor(_data, usercolor);
    }
}

void DialogDataDetails::on_checkboxDataPoints_clicked(bool on) {
    // try to set line style
    if (_series) {
        QwtPlotCurve*curve = dynamic_cast<QwtPlotCurve*>(_series);
        if (curve) {
            QColor col = curve->pen().color();
            if (on) {
                curve->setSymbol(new QwtSymbol(QwtSymbol::Cross, Qt::NoBrush, QPen(col), QSize(5, 5) ) );
            } else {
                curve->setSymbol(NULL);
            }
        }
    }
}

void DialogDataDetails::on_buttonExport_clicked() {
    // show save dialog and trigger exporter of data..
    QString proposed_filename = QString::fromStdString(_data->get_name());
    QString filename = QFileDialog::getSaveFileName(NULL, "Export data to file...", proposed_filename, "Comma-Separated Values (*.csv)");
    if (!filename.isEmpty()) {
        filename.append(".csv"); // FIXME: conditional append!
        bool ok = _data->export_csv(filename.toStdString());
        if (ok) {
            QMessageBox msgbox(QMessageBox::Information, QString("Export complete."), QString("Saved data to file %1.").arg(filename));
            msgbox.exec();
        } else {
            QMessageBox msgbox(QMessageBox::Warning, QString("Export failed."), QString("Something went wrong. Cannot save data to file %1.").arg(filename));
            msgbox.exec();
        }
    }
}

QString DialogDataDetails::_get_title_without_scale(QString title) {
    QString titleOnly = title;
    int suffixpos = title.lastIndexOf(" ("); // FIXME: if data row has parentheses in it, this goes wrong
    if (suffixpos > 0) {
        titleOnly = title.left(suffixpos);
    }
    return titleOnly;
}

double DialogDataDetails::_get_scale_from_title(QString title) {
    double scale=1.0;

    int suffixpos_paren = title.lastIndexOf(" ("); // FIXME: if data row has parentheses in it, this goes wrong
    if (suffixpos_paren == 0) return scale;              // we did not find any scaling in the name
    int suffixpos_dash = title.lastIndexOf("/");
    if (suffixpos_dash <= suffixpos_paren) return scale; // we did not find any scaling in the name
    if (suffixpos_dash > 0) {
        // get old scale
        QString divisor = title.mid(suffixpos_dash+1);
        divisor = divisor.left(divisor.length()-1);
        scale = divisor.toDouble();
    }
    return scale;
}

QString DialogDataDetails::_get_suffix_from_scale(double scale) {
    QString suffix;
    if (scale != 1.0) {
        suffix = " (x 1/" + QString::number((int)scale) + ")";
    }
    return suffix;
}

void DialogDataDetails::on_cbScale_Changed(int idx) {
    // get new scaling
    double scale = cbScale->itemData(idx).toDouble();    

    // fetch data and apply scaling
    if (_series) {
        QwtPlotCurve*curve = dynamic_cast<QwtPlotCurve*>(_series);
        if (curve) {
            QString newtxt = curve->title().text();            
            // update title: remove old suffix and glue new
            int suffixpos = newtxt.lastIndexOf(" (");
            if (suffixpos > 0) {
                newtxt = newtxt.left(suffixpos); // remove old
            }
            newtxt += _get_suffix_from_scale(scale);
            curve->setTitle(newtxt);

            // re-read original data and scale it
            QVector<double> xdata, ydata;            
            MavPlot::data2xyvect(_data, xdata, ydata, 1./scale);
            curve->setSamples(xdata, ydata); // makes a deep copy
        }
    }    
}

void DialogDataDetails::on_buttonRemoveData_clicked() {
    emit removeDataFromPlot(_data);
    this->close();
}

void DialogDataDetails::on_buttonOk_clicked() {
    this->close();
}

void DialogDataDetails::on_txtName_changed(QString newname) {
    // for timeseries
    if (_series) {
        QwtPlotCurve*curve = dynamic_cast<QwtPlotCurve*>(_series);
        if (curve) {
            double scale = cbScale->itemData(cbScale->currentIndex()).toDouble();
            newname += _get_suffix_from_scale(scale);
            curve->setTitle(newname);
        }         
    }
    // for other annotations, e.g., markers
    if (_items) {
        for (std::vector<QwtPlotItem*>::iterator it = _items->begin(); it != _items->end(); ++it) {
            QwtPlotMarker*mark = dynamic_cast<QwtPlotMarker*>(*it);
            if (mark) {
                mark->setTitle(newname);
            }
        }
    }
    // call model and set new name
    _model.model_setname(_data, newname.toStdString());
}
