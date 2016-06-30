/**
 * @file mainwindow.cpp
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 2014-Apr-18
 *
 *  This file is part of MavLogAnalyzer, Copyright 2014 by Martin Becker.

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

#include <sstream>
#include <iostream>
#include <QSettings>
#include <QMessageBox>
#include <QSpacerItem>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qimagewriter.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpicture.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwt_counter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include <qwt.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include "qwt_compat.h"
#include "mainwindow.h"
#include "dialogstats.h"
#include "dialogscenarioprops.h"
#include "dialogdbsettings.h"
#include "dialogdatatable.h"
#include "Zoomer.h"
#include "ui_mainwindow.h"
#include "filefun.h"
#include "filterwindow.h"
#include "dbconnector.h"
#include "logger.h"
#include <qstringlistmodel.h>
#include <qstandarditemmodel.h>
#include "dialogselectscenario.h"
#include "onboardlogparser_apm.h"
#include "onboardlogparser_px4.h"
//#include <qtconcurrentrun.h>

using namespace std;


void MainWindow::_updateHScroll (void) {

    // view range
    QwtInterval i =  d_plot->axisInterval(QwtPlot::xBottom);
    float viewmin = i.minValue();
    float viewmax = i.maxValue();

    // data range
    QRectF datarange = d_plot->get_data_bounds();
    float datamin = datarange.bottomLeft().x();
    float datamax = datarange.bottomRight().x();
    //qDebug() << "view x: " << viewmin << " .. " << viewmax;
    //qDebug() << "data x: " << datamin << " .. " << datamax;

    // adjust scrollbar
    float pagestep = viewmax-viewmin;
    ui->scrollHPlot->setMinimum(datamin);
    ui->scrollHPlot->setMaximum(datamax-pagestep);
    ui->scrollHPlot->setPageStep(pagestep);
    ui->scrollHPlot->setValue(viewmin);
}


void MainWindow::on_plotZoomed (float /*viewmin*/, float /*viewmax*/) {
    _updateHScroll();
    if (_dlgstats) {
        if (_dlgstats->isVisible())
        _dlgstats->updateData();
    }
}

void MainWindow::on_plotPanned(int /*dx*/, int /*dy*/) {
    _updateHScroll();
    if (_dlgstats) {
        if (_dlgstats->isVisible())
        _dlgstats->updateData();
    }
}

void MainWindow::_save_windows_settings(void) {

    _settings.beginGroup("mainWindow");
    _settings.setValue("splitter_top", ui->splitter_top->saveState());
    _settings.setValue("splitter_bottom", ui->splitter_bottom->saveState());
    _settings.setValue("splitterTopBottom", ui->splitterTopBottom->saveState());
    _settings.endGroup();

    _settings.beginGroup("database");
    _settings.setValue("host", QVariant(QString::fromStdString(_dbprops.dbhost)));
    _settings.setValue("database", QVariant(QString::fromStdString(_dbprops.dbname)));
    _settings.setValue("user", QVariant(QString::fromStdString(_dbprops.username)));
    _settings.setValue("pass", QVariant(QString::fromStdString(_dbprops.password)));
    _settings.endGroup();
}

void MainWindow::_load_windows_settings(void) {

    qDebug() << "Application settings: " << _settings.fileName();
    _settings.beginGroup("mainWindow");
    if (_settings.contains("splitter_top")) {
        ui->splitter_top->restoreState(_settings.value("splitter_top").toByteArray());
    } else {
        // default stretches for the splitters' contained elements
        QList<int> sizes;
        sizes.push_back(20);
        sizes.push_back(80);
        ui->splitter_top->setSizes(sizes);
    }
    if (_settings.contains("splitter_bottom")) {
        ui->splitter_bottom->restoreState(_settings.value("splitter_bottom").toByteArray());
    } else {
        QList<int> sizes;
        sizes.push_back(20);
        sizes.push_back(80);
        ui->splitter_bottom->setSizes(sizes);
    }
    if (_settings.contains("splitterTopBottom")) {
        ui->splitterTopBottom->restoreState(_settings.value("splitterTopBottom").toByteArray());
    } else {
        QList<int> sizes;
        sizes.push_back(50);
        sizes.push_back(50);
        ui->splitterTopBottom->setSizes(sizes);
    }
    _settings.endGroup();

    _settings.beginGroup("database");
    _dbprops.dbhost = _settings.value("host", QVariant("localhost")).toString().toStdString();
    _dbprops.dbname = _settings.value("database", QVariant("mavlog_database")).toString().toStdString();
    _dbprops.username = _settings.value("user", QVariant("mavlog_user")).toString().toStdString();
    _dbprops.password= _settings.value("pass", QVariant("mavlog_password")).toString().toStdString();
    _settings.endGroup();
}

/**
 * @brief This is called when a system was selected in the system table
 * fill other widgets accordingly.
 */
void MainWindow::on_systemSelectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/)  {
    //get the text of the selected item
    const QModelIndex index = ui->tableSystems->selectionModel()->currentIndex();    
    unsigned int sysid = index.data(Qt::UserRole).toUInt();    
    const MavSystem*const sys =  _analyzer->get_system_byid(sysid);
    if (!sys) return;
    _lastsys=sys;

    // now trigger updates
    _updateTextInfo(sys);
    _updateTreeData(sys);
}

/**
 * @brief This is called when a data row on the left-side treeview was selected
 */
void MainWindow::on_datarowSelectionChangedSlot(const QItemSelection &, const QItemSelection &) {
    // memorize the selected data...not yet do anything
    const QModelIndex index = ui->treeData->selectionModel()->currentIndex();
    TreeItem*t = static_cast<TreeItem*>(index.internalPointer()); // FIXME: mixing treeitem is not good
    if (!t) return;

    if (TreeItem::DATA == t->itemtype) {
        Data*data = dynamic_cast<Data*>(t);
        _dataSelected = data;
        _datagroupSelected = NULL;
    } else if (TreeItem::GROUP == t->itemtype) {
        DataGroup*datagroup = dynamic_cast<DataGroup*>(t);
        _datagroupSelected = datagroup;
        _dataSelected = NULL;
    }
}

void MainWindow::_updateTreeData(const MavSystem*const sys) {
    if (!sys) return;

    ui->treeData->clearSelection();
    _dtvm->set_mav_sys(sys);
}

void MainWindow::_updateTextInfo(const MavSystem*const sys) {
    if (!sys) return;

    // assemble a quick and dirty text
    string summary;
    sys->get_summary(summary);
    QString selectedText = QString::fromStdString(summary);
    QString showString = QString("%1").arg(selectedText);
    ui->txtDetails->setText(showString);
}

void MainWindow::_setupSignalsAndSlots(void) {
    // Signal "selection changed" from System Table
    QItemSelectionModel*selectModelts = ui->tableSystems->selectionModel();
    QItemSelectionModel*selectModeltd = ui->treeData->selectionModel();
    QItemSelectionModel*selectModelDB= ui->tableDB->selectionModel();
    connect(selectModelts, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_systemSelectionChangedSlot(QItemSelection,QItemSelection)));
    connect(selectModeltd, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_datarowSelectionChangedSlot(QItemSelection,QItemSelection)));
    connect(selectModelDB, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_DBSelectionChangedSlot(QItemSelection,QItemSelection)));


    connect(d_picker, SIGNAL(moved(const QPoint &)), SLOT(moved(const QPoint &)));
    connect(d_picker, SIGNAL(selected(const QPolygon &)), SLOT(selected(const QPolygon &)));    
    connect(d_panner, SIGNAL(panned(int,int)), this, SLOT(on_plotPanned(int,int)));
    connect(d_zoomer, SIGNAL(plotMoved(float,float)), this, SLOT(on_plotZoomed(float,float)));

    // commented connections are automatic, because naming convention is followed:
    //connect(ui->buttonAddData, SIGNAL(clicked()), SLOT(on_buttonAddData_clicked()));
    //connect(ui->buttonAddFile, SIGNAL(clicked()), SLOT(on_buttonAddFile_clicked()));
    //connect(ui->buttonAddFileWithDelay, SIGNAL(clicked()), SLOT(on_buttonAddFileWithDelay_clicked()));
    //connect(ui->buttonAutoFit, SIGNAL(clicked()), SLOT(on_buttonAutoFit_clicked()));
    //connect(ui->buttonCalcStats, SIGNAL(clicked()), SLOT(on_buttonCalcStats_clicked()));
    //connect(ui->buttonClear, SIGNAL(clicked()), SLOT(on_buttonClear_clicked()));
    //connect(ui->buttonClearScenario, SIGNAL(clicked()), SLOT(on_buttonClearScenario_clicked()));
    //connect(ui->buttonDataMax, SIGNAL(clicked()), SLOT(on_buttonDataMax_clicked()));
    //connect(ui->buttonDataMin, SIGNAL(clicked()), SLOT(on_buttonDataMin_clicked()));
    //connect(ui->buttonDataNext, SIGNAL(clicked()), SLOT(on_buttonDataNext_clicked()));
    //connect(ui->buttonDataPrev, SIGNAL(clicked()), SLOT(on_buttonDataPrev_clicked()));
    connect(ui->buttonDataPut, SIGNAL(toggled(bool)), SLOT(on_buttonSetMarkerData(bool)));
    //connect(ui->buttonDataTable, SIGNAL(clicked()), SLOT(on_buttonDataTable_clicked()));
    connect(ui->buttonExportCsv, SIGNAL(clicked()), SLOT(exportCsv()));
    connect(ui->buttonExportPdf, SIGNAL(clicked()), SLOT(exportDocument()));
    connect(ui->buttonLock, SIGNAL(toggled(bool)), SLOT(enableLockMode(bool)));
    connect(ui->buttonMarkerA, SIGNAL(toggled(bool)), SLOT(on_buttonSetMarkerA(bool)));
    connect(ui->buttonMarkerB, SIGNAL(toggled(bool)), SLOT(on_buttonSetMarkerB(bool)));
    connect(ui->buttonPrint, SIGNAL(clicked()), SLOT(print()));
    //connect(ui->buttonSaveDB, SIGNAL(clicked()), SLOT(on_buttonSaveDB_clicked()));
    //connect(ui->buttonScenarioProps, SIGNAL(clicked()), SLOT(on_buttonScenarioProps_clicked()));
    //connect(ui->buttonSearchDB, SIGNAL(clicked()), SLOT(on_buttonSearchDB_clicked()));
    //connect(ui->buttonSetupDB, SIGNAL(clicked()), SLOT(on_buttonSetupDB_clicked()));
    connect(ui->buttonXzoom, SIGNAL(toggled(bool)), SLOT(on_buttonSetXZoom(bool)));
    connect(ui->buttonYzoom, SIGNAL(toggled(bool)), SLOT(on_buttonSetYZoom(bool)));

}

void MainWindow::print() {
    QPrinter printer(QPrinter::HighResolution);

    QString docName = d_plot->title().text();
    if ( !docName.isEmpty() ){
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("MavLogAnalyzer (C)2013-2016 Martin Becker");
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog dialog(&printer);
    if ( dialog.exec() )  {
        QwtPlotRenderer renderer;


        if ( printer.colorMode() == QPrinter::GrayScale ) {
            renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
            renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
        }                
        renderer.renderTo(d_plot, printer);
    }
}

void MainWindow::notimplementedyet() {
    QString interjections("Absolutely, Achoo, Ack, Ahh, Aha, Ahem, Ahoy, Agreed, Alas, Alright, Alrighty, Alrighty-roo, Alack, Amen, Anytime, Argh, Anyhoo, Anyhow, As if, Attaboy, Attagirl, Awww, Awful, Bam, Bah humbug, Behold, Bingo, Blah, Bless you, Boo, Bravo, Cheers, Crud, Darn, Dang, Doh, Drat, Duh, Eek, Eh, Gee, Geepers, Gee Whiz, Golly, Goodnes, Goodness Gracious, Gosh, Ha, Hallelujah, Hey, Hi, Hmm, Huh, Indeed, Jeez, My gosh, No, Now, Nah, Oops, Ouch, Phew, Please, Rats, Shoot, Shucks, There, Tut, Uggh, Waa, What, Woah, Woops, Wow, Yay, Yes, Yikes");
    QRegExp rx("\\, ");
    QStringList query = interjections.split(rx);
    int k = qrand() % (query.size()-1);
    QString callout(query.at(k));
    ///< randomly select one that makes the user happy
    QMessageBox msgbox(QMessageBox::Warning, callout, QString("Sorry, but this feature is not implemented yet.\nStay tuned."));
    msgbox.exec();
}

/**
 * @brief all data that is added to the view shall be written to CSV now
 */
void MainWindow::exportCsv() {
    // ask for filename
    if (!d_plot) return;

    QString defaultfilter = "Comma-Separated Values (*.csv)";
    QStringList filter;
    filter += defaultfilter;

    QString fileName = "data.csv";
    fileName = QFileDialog::getSaveFileName(this, "Export File Name", fileName, filter.join(";;"), &defaultfilter, QFileDialog::DontConfirmOverwrite);
    if ( fileName.isEmpty() ) return;

    unsigned int n_written = d_plot->exportCsv(fileName.toStdString());
    if (n_written == 0) {
        QMessageBox msgbox(QMessageBox::Critical, "Export CSV", QString("Sorry, but the export failed. See console."));
        msgbox.exec();
    } else {
        QMessageBox msgbox(QMessageBox::Information, "Export CSV", QString("Successfully written file \""+ fileName + "\" with " + QString::number(n_written) + " data series."));
        msgbox.exec();
    }
}

void MainWindow::exportDocument() {
#ifndef QT_NO_PRINTER
    QString fileName = "plot.pdf";
#else
    QString fileName = "plot.png";
#endif

#ifndef QT_NO_FILEDIALOG
    const QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();

    QStringList filter;
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    if ( imageFormats.size() > 0 ) {
        QString imageFilter("Images (");
        for ( int i = 0; i < imageFormats.size(); i++ ) {
            if ( i > 0 ) imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";
        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(this, "Export File Name", fileName, filter.join(";;"), NULL, QFileDialog::DontConfirmOverwrite);
#endif

    if ( !fileName.isEmpty() ) {
        QwtPlotRenderer renderer;

        // flags to make the document readable when exported (no dark background and so on)
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);
        #if (QWT_VERSION < QWT_VERSION_CHECK(6,1,0))
            renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, false);
        #endif

        // create PDF
        d_plot->apply_print_colors(true);
        renderer.renderDocument(d_plot, fileName, QSizeF(300, 200), 150);
        d_plot->apply_print_colors(false);
    }
}


void MainWindow::enableVLockMode(bool /*on*/) {
    // cannot zoom vertically anymore, only horizontally
    //d_zoomer->setRubberBand(QwtPlotZoomer::VLineRubberBand);
}

void MainWindow::enableHLockMode(bool /*on*/) {
    // cannot zoom horizontally anymore, only vertically
    //d_zoomer->setRubberBand(QwtPlotZoomer::HLineRubberBand);
}

void MainWindow::enableLockMode(bool on) {
    // locked means: no zoom. only man and pick.           
    d_zoomer->setEnabled(!on);
    d_picker->setEnabled(on);
    ui->buttonXzoom->setChecked(false); d_zoomer->setXZoom(false);
    ui->buttonYzoom->setChecked(false); d_zoomer->setYZoom(false);
    // A-B markers only when zoom is locked
    ui->buttonMarkerA->setEnabled(on);
    ui->buttonMarkerB->setEnabled(on);
    //ui->buttonDataPut->setEnabled(on);
    if (on) {
        ui->lblMarkerState->setText("Enabled");
    } else {
        ui->lblMarkerState->setText("Lock zoom!");
    }
    // make sure button is consistent
    if (ui->buttonLock->isChecked() != on) {
        ui->buttonLock->setChecked(on);
    }
    showInfo();
}

void MainWindow::_styling(void) {
    // all bold labels get assigned the class "bold"
    QList<QLabel*> list = this->findChildren<QLabel*>();
    foreach(QLabel *l, list) {
        if (l->fontInfo().bold()) {
            l->setProperty("class", "bold");
            l->style()->unpolish(l);
            l->ensurePolished();
        }
    }
#if 0 /* looks weirdo */
    // TUM logo
    QPixmap pixmap(":/images/logo_tum.png");
    pixmap = pixmap.scaledToHeight(20,Qt::SmoothTransformation);
    ui->lblLOGOTUM->setScaledContents(true);
    ui->lblLOGOTUM->setPixmap(pixmap);
    ui->lblLOGOTUM->setMask(pixmap.mask());
    ui->lblLOGOTUM->show();

    QPixmap pixmapr(":/images/logo_rcs.png");
    pixmapr = pixmapr.scaledToHeight(20,Qt::SmoothTransformation);
    ui->lblLOGORCS->setScaledContents(true);
    ui->lblLOGORCS->setPixmap(pixmapr);
    ui->lblLOGORCS->setMask(pixmapr.mask());
    ui->lblLOGORCS->show();
#endif
}

void MainWindow::showInfo(QString text)
{
    if ( text == QString::null ) {
        if ( d_picker->rubberBand() ) {
            text = "Cursor Pos: Press left mouse button in plot region";
        } else {
            text = "Zoom: Press mouse button and drag";
        }
    }

#ifndef QT_NO_STATUSBAR
    statusBar()->showMessage(text);
#endif
}

void MainWindow::moved(const QPoint &pos)
{
    QString info;
    double time_raw = d_plot->invTransform(QwtPlot::xBottom, pos.x());
    QString time_readable = d_plot->getReadableTime(time_raw);
    info.sprintf("Time=%s, Value=%g",
        time_readable.toUtf8().data(),
        d_plot->invTransform(QwtPlot::yLeft, pos.y())
    );
    showInfo(info);
}

void MainWindow::selected(const QPolygon &pos) {
    double time_raw = d_plot->invTransform(QwtPlot::xBottom, pos.first().x());

    if (_markerData) {
        d_plot->set_markerData(time_raw);
        ui->buttonDataNext->setEnabled(true);
        ui->buttonDataPrev->setEnabled(true);
        ui->buttonDataMax->setEnabled(true);
        ui->buttonDataMin->setEnabled(true);
        ui->cboDataSel->setModel(d_plot->get_datamodel());
        ui->buttonDataPut->setChecked(false);
    }
    else if (_markerA) {
        d_plot->set_markerA(time_raw);       
    }
    else if (_markerB) {
        d_plot->set_markerB(time_raw);        
    }

    showInfo(d_plot->verboseMarkers());
}

void MainWindow::_setupPlotWidget(void) {
    d_plot = new MavPlot(this);
    ui->vlPlot->insertWidget(0, d_plot);

    const int margin = 5;
    d_plot->setContentsMargins( margin, margin, margin, 0 );
    ui->cboDataSel->setModel(d_plot->get_datamodel());

    //setContextMenuPolicy(Qt::NoContextMenu);

    /***************************************************
     *  SET-UP PAN, ZOOM and PICKING
     ***************************************************/
    d_zoomer = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, d_plot->canvas());
    d_zoomer->setRubberBand(QwtPicker::RectRubberBand);
    d_zoomer->setRubberBandPen(QColor(Qt::yellow));
    d_zoomer->setTrackerMode(QwtPicker::ActiveOnly);
    d_zoomer->setTrackerPen(QColor(Qt::white));
    d_zoomer->setMaxStackDepth(10);

    d_panner = new Panner(d_plot->canvas());
    d_panner->setMouseButton(Qt::MidButton);

    d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, d_plot->canvas());
    d_picker->setStateMachine(new QwtPickerDragPointMachine());
    d_picker->setRubberBandPen(QColor(Qt::yellow));
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setTrackerPen(QColor(Qt::white));

#ifndef QT_NO_STATUSBAR
    d_plot->set_statusbar(this->statusBar());
#endif
    enableLockMode(false);
    showInfo();
}

void MainWindow::_setupToolbar(void) {
    /***************************************************
     *  TOOLBAR
     ***************************************************/
#ifndef QT_NO_STATUSBAR
    (void)statusBar();
#endif
}

void MainWindow::_buildDialog(void) {
    // start models for table view and tree view
    _stvm = new SystemTableViewModel(NULL,_analyzer);
    _dtvm = new DataTreeViewModel(NULL, _analyzer);
    _DBResultModel = new QStandardItemModel;
    QStringList headerNamesResults;
    headerNamesResults<<"ID"  << "Date" << "Name" << "Description";
    _DBResultModel->setHorizontalHeaderLabels(headerNamesResults);
    ui->tableDB->horizontalHeader()->setStretchLastSection(true);
    ui->tableDB->setSortingEnabled(true);


    // set-up System table and data tree
    ui->tableSystems->setModel(_stvm); // model which holds data for this view
    ui->treeData->setModel(_dtvm);
    ui->tableDB->setModel(_DBResultModel);
    ui->tableDB->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableDB->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableSystems->show();
    ui->tableSystems->horizontalHeader()->setStretchLastSection(true);
    ui->treeData->show();
    _lastsys = NULL;

    // add logview at the bottom
    QVBoxLayout*vbox = new QVBoxLayout; // we want to put buttons on top to hide it
    QWidget*w = new QWidget; // ... but we need a widget to put into splitter...
    w->setLayout(vbox); // ... which carries the layout
    w->setContentsMargins(0,0,0,0);
    vbox->setContentsMargins(0,0,0,0);
    ui->splitterTopBottom->addWidget(w);
    QHBoxLayout*hbox = new QHBoxLayout;

    QLabel*l = new QLabel;
    l->setText("Messages:");
    QSpacerItem*sp1 = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QSpacerItem*sp2 = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPushButton*btnExpandAll = new QPushButton;
    btnExpandAll->setText("Expand All");
    QPushButton*btnCollapseAll = new QPushButton;
    btnCollapseAll->setText("Collapse All");
    QPushButton*btnRemoveLog = new QPushButton;
    btnRemoveLog->setText("Remove Selected");
    hbox->addWidget(l);
    hbox->addSpacerItem(sp1);
    hbox->addWidget(btnExpandAll);
    hbox->addWidget(btnCollapseAll);
    hbox->addWidget(btnRemoveLog);
    hbox->addSpacerItem(sp2);

    connect(btnExpandAll, SIGNAL(clicked()), SLOT(on_buttonLogExpand_clicked()));
    connect(btnCollapseAll, SIGNAL(clicked()), SLOT(on_buttonLogCollapse_clicked()));
    connect(btnRemoveLog, SIGNAL(clicked()), SLOT(on_buttonLogRemove_clicked()));

    vbox->addLayout(hbox);
    _logmsg = new QTreeView;
    vbox->addWidget(_logmsg); // ... which carries the treeview
    _logmsg->header()->setStretchLastSection(true);
    #if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
        _logmsg->header()->setMovable(false);
    #endif
    _logmsg->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _logmsg->setSortingEnabled(true);
    _logmsg->setSelectionMode(QAbstractItemView::ContiguousSelection); // MultiSelection is not possible because removing rows from QAbstractItemModel cannot do that
    _logmsg->setSelectionBehavior(QAbstractItemView::SelectRows);
    LogTableModel*ltm = Logger::Instance().getModel();
    if (ltm) _logmsg->setModel(ltm); // ... which shows the logmodel
    _logmsg->expandAll();


    // splitter & size hints
    /* Layout:

      central widget
        - splitterTopBottom
          - splitter_top
            - scenario
            - database/file
          - hBottom
            - Actions
            - splitter_bottom
              - tree
              - plot

      +-----------------------------------------
      |  scenario   |    database/file        | 2=stretch
      |           <-|->        ^              | expanding size policy
      +-----------------------------------------
      |          |             v       | tool | 3=stretch
      |  tree  <-|->       plot        |      | expanding size policy
      |          |             ^       |      |
      +-----------------------------------------
      |  log messages          v              | 1=stretch, expanding size policy
      +----------------------------------------

     */
    // stretch behavior
    ui->splitterTopBottom->setStretchFactor(0, 2); // 1/3 for child "scenario"
    ui->splitterTopBottom->setStretchFactor(1, 3); // 1/2 for child "plot"
    ui->splitterTopBottom->setStretchFactor(2, 1); // 1/6 for child "log messages"
    ui->splitterTopBottom->setCollapsible(0, true); // scenario can be collapsed
    ui->splitterTopBottom->setCollapsible(1, false); // plot cannot be collapsed
    ui->splitterTopBottom->setCollapsible(2, true); // log messagfes can be collapsed
    ui->hBottom->setStretch(0, 1); // tree+plot get all the space
    ui->hBottom->setStretch(1, 0); // vertical sep
    ui->hBottom->setStretch(2, 0); // actions
    ui->treeData->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding); // allow splitter to resize horiz, but vertically take up all space

    // tighter margins
    ui->splitterTopBottom->setContentsMargins(0,0,0,0);
    ui->splitterTopBottom->setHandleWidth(5);
    ui->splitter_bottom->setHandleWidth(5);
    ui->splitterTopBottom->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->splitter_top->setHandleWidth(5);
    ui->centralWidget->setContentsMargins(0,0,0,0);
    ui->centralWidget->layout()->setContentsMargins(5,5,5,5);
    ui->tab->layout()->setContentsMargins(5,5,5,5);
    ui->tab_2->layout()->setContentsMargins(5,5,5,5);
    ui->grScenario->layout()->setContentsMargins(5,8,5,5);

    //ui->centralWidget->setStyleSheet("background-color: red");
}

void MainWindow::_setupFilelist(void) {
    for (list<MavlinkParser*>::const_iterator it = _parsers->begin(); it != _parsers->end(); ++it) {
        QString fname = QString().fromStdString((*it)->get_filename());
        ui->listFiles->addItem(fname);
    }
}

MainWindow::MainWindow(list<MavlinkParser*> &parsers, CmdlineArgs *const args, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _settings("DE.TUM.EI.RCS", "MavLogAnalyzer"), _dataSelected(NULL), _datagroupSelected(NULL),
    _markerA(false), _markerB(false), _markerData(false),
    _dlgprogress(NULL), _dlgstats(NULL), _dlgdatatable(NULL) {

    ui->setupUi(this);    
    _args = args;

    cout << "GUI got " << parsers.size() << " parsers from cmdline" << endl;
    _analyzer = new MavlinkScenario(args);
    _parsers = &parsers;

    // parse all in the one analyzer for the time being
    mavlink_message_t msg;
    for (list<MavlinkParser*>::iterator it = parsers.begin(); it!= parsers.end(); ++it) {
        while ((*it)->get_next_msg(msg)) {
            _analyzer->add_mavlink_message(msg);
        }
    }
    _analyzer->process();
    _analyzer->dump_overview();

    _buildDialog();

    _setupFilelist();

    _setupToolbar();

    _setupPlotWidget();

    _setupSignalsAndSlots();    

    _styling();

    _load_windows_settings();
}

MainWindow::~MainWindow() {
    _save_windows_settings();
    delete ui;
    delete _analyzer;
}

/**
 * @brief adds data or datagroup (recursively) to plot
 */
void MainWindow::_addDataToPlot(TreeItem * const item) {
    if (!item) return;


    // figure out whether it is a group or data
    const DataGroup * const g = dynamic_cast<const DataGroup * const>(item);
    if (g) {
        // try to add all data and all groups in here
        for (DataGroup::groupmap::const_iterator itg = g->groups.begin(); itg != g->groups.end(); ++itg) {
            _addDataToPlot(dynamic_cast<const TreeItem*const>(itg->second)); // FIXME: conversion from const TreeItem* to TreeItem* (fpermissive)
        }
        for (DataGroup::datamap::const_iterator itd = g->data.begin(); itd != g->data.end(); ++itd) {
            _addDataToPlot(dynamic_cast<const TreeItem*const>(itd->second)); // FIXME: conversion from const TreeItem* to TreeItem* (fpermissive)
        }
    }
    Data * const d = dynamic_cast<Data * const>(item);
    if (d) {
        // no group -> single item. just add it.

        if (d->is_deferred()) {
            DBConnector* dbCon = new DBConnector(_dbprops);
            showProgressBar();
            if (!dbCon->loadDataGroup(d, _dlgprogress)) {
                cerr << "ERROR loading data group from database.";
            }
            hideProgressBar();
            delete dbCon;
            //d->unset_deferred(); // cuz now its loaded // TODO
        }

        const Data * const d = dynamic_cast<const Data * const>(item);
        if (!d_plot->addData(d)) {
            QMessageBox msgbox(QMessageBox::Warning, QString("Error"), QString("Sorry, but data type of data %1 is not recognized.\nExtend mavplot.cpp to handle this type.").arg(d->get_name().c_str()));
            msgbox.exec();
            return;
        }
    }
    // update stats window
    if (_dlgstats) {
        if (_dlgstats->isVisible())
        _dlgstats->updateData();
    }
}

void MainWindow::on_buttonAddData_clicked() {
    bool set_zoom_base = (d_plot->get_num_data() == 0);

    if (_dataSelected) {
        _addDataToPlot(_dataSelected);
    } else if (_datagroupSelected) {
        _addDataToPlot(dynamic_cast<TreeItem*const>(_datagroupSelected));
    } else {
        return; // nothing selected
    }

    _updateHScroll();
    if (set_zoom_base) {
        d_zoomer->setZoomBase();
    }
}

void MainWindow::on_scrollHPlot_sliderMoved(int position) {
    // view range
    QwtInterval i =  d_plot->axisInterval(QwtPlot::xBottom);
    double viewmin = i.minValue();
    double viewmax = i.maxValue();
    double range = viewmax-viewmin;
    d_plot->setAxisScale(QwtPlot::xBottom, position, position+range);
}

void MainWindow::on_buttonAutoFit_clicked() {
    // let the plot automatically zoom as appropriate
    d_plot->setAxisAutoScale(QwtPlot::xBottom, true);
    d_plot->setAxisAutoScale(QwtPlot::yLeft, true);
    _updateHScroll();
    d_zoomer->setZoomBase();
}

/**
 * @brief check if a specific file is already loaded
 * @param fname
 * @return true if loaded
 */
bool MainWindow::_fileLoaded(const QString& f_fullpath) const {
    bool found=false;
    for(int i = 0; i < ui->listFiles->count(); ++i) {
        QListWidgetItem* item = ui->listFiles->item(i);
        if (item->text() == f_fullpath) {
            found = true;
            QMessageBox msgbox(QMessageBox::Warning, QString("Nope"), QString("File %1 already in list.").arg(f_fullpath));
            msgbox.exec();
            return true;
        }
    }
    return found;
}

/**
 * @brief asks the user to select exactly one scenario out of a list.
 * @param vector with the choices
 * @return ptr to one list item
 */
MavlinkScenario* MainWindow::_forceChooseScenario(const std::vector<MavlinkScenario*>& items) const {
    MavlinkScenario*ret = NULL;

    DialogSelectScenario dlg(items);
    dlg.exec();
    int chosen = dlg.result();
    assert(chosen >=0 && chosen < ((int)items.size()));

    qDebug() << "Chosen scenario " << chosen << " out of " << items.size();
    ret = items[chosen];
    return ret;
}

void MainWindow::_addFile(double delay) {
    // TODO: check whether currently a DB scenario is loaded. Ask user whether to clear or merge in.

    // start in last path
    _settings.beginGroup("fileDialog");
    QString startDir = _settings.value("last_directory","").toString();
    _settings.endGroup();

    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Add Files to scenario", startDir, "MavLink or onboard log (*.tlog *.log *.mavlink *.px4log)");
    if (fileNames.empty()) return;

    // save last path
    QString dirname = QString::fromStdString(getDirname(fileNames.first().toStdString()));
    _settings.beginGroup("fileDialog");
    _settings.setValue("last_directory",dirname);
    _settings.endGroup();

    // do it
    showProgressBar();
    updateProgressBarTitle("Merging in data...");
    updateProgressBarValue(0, fileNames.size());

    bool timeJumpsBack_yesToAll = false;
    bool timeJumpsBack_noToAll = false;    
    bool timeJumpsFwd_yesToAll = false;
    bool timeJumpsFwd_noToAll = false;    

    unsigned int progress=0;
    for (QStringList::Iterator itf = fileNames.begin(); itf != fileNames.end(); ++itf) {
        QString f = *itf;
        QString f_fullpath = QString::fromStdString(getFullPath(f.toStdString()));
        string f_basename = getBasename(f_fullpath.toStdString());
        if(f_fullpath.compare("")==0) continue;       
        if (!_fileLoaded(f_fullpath)) {

            // use a new temporary scene for this file
            std::vector<MavlinkScenario*> fileScenarios;
            MavlinkScenario*tmp_scene = new MavlinkScenario(_args); fileScenarios.push_back(tmp_scene);
            tmp_scene->setName(getBasename(f_fullpath.toStdString()));

            // decide which parser to take and do it
            string ext = getExtension(f_fullpath.toStdString());
            ext = lcase(ext);            
            bool parsed = false;
            if (ext.compare("tlog") == 0 || ext.compare("mavlink") == 0) {
                /*****************
                 * MavLink Log
                 *****************/
                MavlinkParser*mlp = new MavlinkParser(f_fullpath.toStdString());
                if (!mlp) {
                    qDebug() << "Could not allocate memory for new MavlinkParser";
                    continue; // skip file
                }
                if (!mlp->valid) {continue;}
                _parsers->push_back(mlp); ///< FIXME: why do we keep that thing?

                // run it, feed it into analyzer
                mavlink_message_t msg;
                while (mlp->get_next_msg(msg)) {
                    int upd = tmp_scene->add_mavlink_message(msg);

                    // ERROR HANDLING:
                    if (1 == upd) {
                        /************************
                         * fwd jump in time base
                         ************************/
                        bool tolerate = false;
                        if (!timeJumpsFwd_noToAll) {
                            tolerate = timeJumpsFwd_yesToAll;
                            if (!tolerate) {
                                // ask whether to do this. FIXME: give more context to the user
                                QMessageBox msg (QMessageBox::Question, "Time jump detected", "There is a rapid forward time jump in the data. Allow jump and make one scenario or demultiplex into separate scenarios?", QMessageBox::Yes|QMessageBox::No|QMessageBox::YesToAll|QMessageBox::NoToAll);
                                msg.setButtonText(QMessageBox::Yes, "Allow");
                                msg.setButtonText(QMessageBox::No, "Demux");
                                msg.setButtonText(QMessageBox::YesToAll, "Allow all");
                                msg.setButtonText(QMessageBox::NoToAll, "Demux all");
                                switch (msg.exec()) {
                                    case QMessageBox::YesToAll:
                                        timeJumpsFwd_yesToAll = true;
                                        // fallthrough;
                                    case QMessageBox::Yes:
                                        tolerate = true;
                                        break;
                                    case QMessageBox::NoToAll:
                                        timeJumpsFwd_noToAll = true;
                                        // fallthrough
                                    case QMessageBox::No:
                                        // nothing to do
                                        break;
                                    default:
                                        break;
                                }
                            }                        
                        }
                        if (!tolerate) {
                            // demux into new scenario
                            tmp_scene = new MavlinkScenario(_args); fileScenarios.push_back(tmp_scene);
                            tmp_scene->setName(getBasename(f_fullpath.toStdString()) + "_" + QString::number(fileScenarios.size()).toStdString());
                        }
                        tmp_scene->add_mavlink_message(msg, true);

                    } else if (-1 == upd) {
                        /*************************
                         * back jump in time base
                         *************************/
                        bool tolerate = false;
                        if (!timeJumpsBack_noToAll) {
                            tolerate = timeJumpsBack_yesToAll;
                            if (!tolerate) {
                                // ask whether to do this. FIXME: give more context to the user
                                QMessageBox msg(QMessageBox::Question, "Time jump detected", "There is a rapid backward time jump in the data. Allow jump and make one scenario (yes) or demultiplex into separate scenarios (no)?",  QMessageBox::Yes|QMessageBox::No|QMessageBox::YesToAll|QMessageBox::NoToAll);
                                msg.setButtonText(QMessageBox::Yes, "Allow");
                                msg.setButtonText(QMessageBox::No, "Demux");
                                msg.setButtonText(QMessageBox::YesToAll, "Allow all");
                                msg.setButtonText(QMessageBox::NoToAll, "Demux all");
                                switch (msg.exec()) {
                                    case QMessageBox::YesToAll:
                                        timeJumpsBack_yesToAll = true;
                                        // fallthrough;
                                    case QMessageBox::Yes:
                                        tolerate = true;
                                        break;
                                    case QMessageBox::NoToAll:
                                        timeJumpsBack_noToAll = true;
                                        // fallthrough
                                    case QMessageBox::No:
                                        // nothing to do
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                        if (!tolerate) {
                            // demux into new scenario
                            tmp_scene = new MavlinkScenario(_args); fileScenarios.push_back(tmp_scene);
                            tmp_scene->setName(getBasename(f_fullpath.toStdString()) + "_" + QString::number(fileScenarios.size()).toStdString());
                        }
                        tmp_scene->add_mavlink_message(msg, true);
                    }

                }
                parsed = true;
            } else if (ext.compare("log") == 0 || ext.compare("px4log") == 0) {
                /*****************
                 * Onboard Log
                 *****************/
                OnboardLogParser *olp = NULL;
                if (ext.compare("log") == 0) {
                    olp = new OnboardLogParserAPM(f_fullpath.toStdString(), tmp_scene->getLogChannel());
                } else if (ext.compare("px4log") == 0) {
                    olp = new OnboardLogParserPX4(f_fullpath.toStdString(), tmp_scene->getLogChannel());
                }
                if (!olp) {
                    qDebug() << "Could not allocate memory for new OnboardLogParser";
                    continue; // skip file
                }                
                if (!olp->valid) {continue;}

                // - do the parsing into the scenario
                while (olp->has_more_data()) {
                    OnboardData d = olp->get_data();
                    if (d.is_valid()) {
                        tmp_scene->add_onboard_message(d);
                    }
                }
                parsed = true;
            }


            // we tried all parsers at this point

            if (parsed) {
                /****************
                 * GOT NEW DATA
                 ****************/

                /* first of all, if we have multiple scenes for this file, force user to
                 * select one (because MLA can only handle one at a time currently...)
                 */

                // at this point, we have only one scene left. work on this.
                if (fileScenarios.size() > 1) {
                    tmp_scene = _forceChooseScenario(fileScenarios);
                }
                // guess the start time of the logfile by looking at the file name                
                QRegExp rex("\\d{4}-\\d{2}-\\d{2}[- ]\\d{2}-\\d{2}(-\\d{2})?");
                if (rex.indexIn(QString::fromStdString(f_basename)) != -1) {
                    QString str_datetime = rex.cap(0);

                    // remove hyphen between date and time, if there is one
                    if (str_datetime.length()>10) {
                        #if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
                            char pos10 = str_datetime[10].toLatin1();
                        #else
                            char pos10 = str_datetime[10].toAscii();
                        #endif
                        if (pos10 != ' ') {
                            str_datetime[10] = ' ';
                        }
                    }
                    qDebug() << "GUESS DATE:" << str_datetime;
                    uint64_t time_epoch_usec = 0;
                    if (string_to_epoch_usec(str_datetime.toStdString(), time_epoch_usec)) {
                        tmp_scene->set_starttime_guess(time_epoch_usec);
                    }
                }

                ui->listFiles->addItem(f_fullpath);

                // apply delay if any
                if (delay != 0.0) {
                    tmp_scene->shift_time(delay);
                }

                // analyze
                tmp_scene->process();
                tmp_scene->dump_overview();

                // merge in the analyzer
                _analyzer->merge_in(*tmp_scene);

                // set scenario name if empty
                if (_analyzer->getName().empty()) {
                    _analyzer->setName(f_basename);
                }

                // remove all temporary scenes
                for (std::vector<MavlinkScenario*>::iterator fit = fileScenarios.begin(); fit != fileScenarios.end(); ++fit) {
                    delete *fit;
                }
                fileScenarios.clear();
            }
        }
        updateProgressBarValue(++progress, fileNames.size());
    }
    hideProgressBar();
    _stvm->reload(); // update everything;
    _dtvm->reload();

    if (_lastsys) {
        // now trigger updates
        _updateTextInfo(_lastsys); ///< FIXME: if addFile removes a MavSystem, then this ptr could be invalid...
        _updateTreeData(_lastsys);
    }
}

void MainWindow::on_buttonAddFile_clicked() {
    _addFile();
}

void MainWindow::showProgressBar() {
    if (!_dlgprogress) {
        // new dialog
        _dlgprogress = new DialogProgressBar;
        _dlgprogress->open();
    } else {
        _dlgprogress->show();
    }
}

void MainWindow::updateProgressBarTitle(QString text) {
    if (!_dlgprogress) return;
    _dlgprogress->setLabel(text);
    _dlgprogress->repaint();    
}

void MainWindow::updateProgressBarValue(unsigned int val, unsigned int max) {
    if (!_dlgprogress) return;
    _dlgprogress->setValue(val, max);
    _dlgprogress->repaint(); // that costs too much time
}

void MainWindow::hideProgressBar() {
    if (!_dlgprogress) return;
    // hide it
    _dlgprogress->hide();
}

void MainWindow::on_buttonSetMarkerA(bool on) {
    _markerA = on;
    if (on) {
        d_plot->unset_markerA();
        on_buttonSetMarkerB(false);
        ui->buttonMarkerB->setChecked(false);
    }
    // actual setting is done in selected() slot
}

void MainWindow::on_buttonSetMarkerData(bool on) {
    _markerData = on;
    if (on) {        
        d_plot->unset_markerData();        
        ui->buttonDataNext->setEnabled(false);
        ui->buttonDataPrev->setEnabled(false);
        ui->buttonDataMin->setEnabled(false);
        ui->buttonDataMax->setEnabled(false);
        enableLockMode(true); // disable zoom, otherwise I cannot select
    }
    // actual setting is done in selected() slot
}

void MainWindow::on_buttonSetMarkerB(bool on) {
    _markerB = on;
    if (on) {
        d_plot->unset_markerB();
        on_buttonSetMarkerA(false);
        ui->buttonMarkerA->setChecked(false);
    }
}

void MainWindow::on_buttonSetXZoom(bool checked) {
    d_zoomer->setXZoom(checked);
    if (checked) {
        ui->buttonYzoom->setChecked(false);
        d_zoomer->setYZoom(false);
    }
}

void MainWindow::on_buttonSetYZoom(bool on) {
    d_zoomer->setYZoom(on);
    if (on) {
        ui->buttonXzoom->setChecked(false);
        d_zoomer->setXZoom(false);
    }
}

void MainWindow::on_buttonClear_clicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove all Data", "Really remove all data from the plot?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        d_plot->removeAllData();
    }
}

void MainWindow::on_treeData_doubleClicked(const QModelIndex &/*index*/) {
    // ASSUMPTION: index==selection
    on_buttonAddData_clicked(); // delegate
}

void MainWindow::on_buttonSearchDB_clicked() {
    //Open FilterWindow
    FilterWindow filterwindow(_dbprops, this);
    filterwindow.setResultModel(_DBResultModel);
    filterwindow.setUpComboBox();
    filterwindow.setModal(true);
    filterwindow.exec();
}

QStringList MainWindow::_getFileNames(void)
{
    QFileDialog *diag=new QFileDialog;
    QStringList fileNames = diag->getOpenFileNames(NULL, "Add Files to import", NULL, "MavLog files (*.tlog)");
    return fileNames;
}

#if 0
// we do not want direct import, because this only does MavLink and no onboard logs. Also it is more logical, if one loads files into a current scenario, and only that scenario can be pushed to DB
void MainWindow::on_buttonImport_clicked()
{
    //Select files to import
    QStringList fileNames=_getFileNames();
    //QFuture<QStringList> future = QtConcurrent::run(MainWindow::_getFileNames);
    //future.waitForFinished();


    //Import each file to DB
    for (QStringList::Iterator itf = fileNames.begin(); itf != fileNames.end(); ++itf)
    {
        QString f = *itf;
        QString f_fullpath = QString::fromStdString(getFullPath(f.toStdString()));
        cout << "Import File to DB" << endl;
        string filename;
        filename = f_fullpath.toStdString();
        DBConnector* dbCon = new DBConnector(_args);
        dbCon->importFromFile(filename);
        delete (dbCon);
    }
    cout<<"Import done!"<<endl;    
}
#endif

void MainWindow::_clearScenario(void) {    
    // clear GUI
    if (d_plot) d_plot->removeAllData();
    ui->txtDetails->clear();
    ui->listFiles->clear();
    // FIXME: qtableview has a model, and this is invalid now

    // FIXME: would be better to have a clear() instead of making new object
    MavlinkScenario*_killme = _analyzer;
    _analyzer = NULL;
    _lastsys = NULL;
    _analyzer = new MavlinkScenario(_args);
    // before we free memory, remove all refs
    _stvm->setScenario(_analyzer);        
    _dtvm->setScenario(_analyzer);
    ui->tableSystems->reset();
    delete _killme;
}

void MainWindow::on_DBSelectionChangedSlot(const QItemSelection &, const QItemSelection & desel) {

    // if plot is non-empty, reset it
    if (d_plot->get_num_data() > 0) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Remove all Data", "You are about to change the scenario. This will clear the plot. Do you want to continue?", QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            // restore old selection
            //ui->tableDB->selectionModel()->cl
            ui->tableDB->blockSignals(true);
            ui->tableDB->selectionModel()->blockSignals(true);
            ui->tableDB->selectionModel()->select(desel, QItemSelectionModel::ClearAndSelect);
            ui->tableDB->selectionModel()->blockSignals(false);
            ui->tableDB->blockSignals(false);
            return;
        }
        d_plot->removeAllData();
    }

    // FIXME: let user decide whether be merged in or cleared
    _clearScenario();

    showProgressBar();
    updateProgressBarTitle("Loading from DB...");    

    //get ID of selected Data
    int id, row;
    const QModelIndex index = ui->tableDB->selectionModel()->currentIndex();
    row=index.row();
    id = ui->tableDB->model()->data(ui->tableDB->model()->index(row, 0)).toInt();

    //get Scenario from DB    
    DBConnector* dbCon = new DBConnector(_dbprops);
    dbCon->setLazyLoad(ui->chkLazy->isChecked());
    std::cout << dbCon->loadScenarioFromDB(id,*_analyzer, _dlgprogress) <<std::endl;
    std::cout << "FINISHED"<<std::endl;    
    // update everything;
    _stvm->reload();
    _dtvm->reload();
    delete dbCon;

    hideProgressBar();
}

void MainWindow::on_buttonCalcStats_clicked() {
    if (!_dlgstats) {
        _dlgstats = new DialogStats(d_plot);
    }
    _dlgstats->show();
}

void MainWindow::on_buttonClearScenario_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Scenario?", "You are about to clear the scenario. This will clear the plot. Do you want to continue?", QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    _clearScenario();
}

void MainWindow::on_buttonSaveDB_clicked() {
    if (!_analyzer) return;

    DBConnector* dbCon = new DBConnector(_dbprops);
    if (!dbCon) return;

    showProgressBar();
    updateProgressBarTitle("Loading from DB...");

    if (dbCon->saveScenarioToDB(_analyzer, _dlgprogress)) {
        QMessageBox::information(this, "Save to DB", "Successfully saved current scenario to DB.", QMessageBox::Ok);
    } else {
        QMessageBox::warning(this, "Save to DB", "Errors while saving current scenario to DB, see command line.", QMessageBox::Ok);
    }
    delete (dbCon);
    hideProgressBar();
}

void MainWindow::on_buttonScenarioProps_clicked()
{
   DialogScenarioProps dlg(_analyzer);
   dlg.setModal(true);
   dlg.exec(); // blocking until closed
}

void MainWindow::on_buttonSetupDB_clicked() {
    DialogDBSettings dlg(&_dbprops);
    dlg.setModal(true);
    dlg.exec(); // blocking until closed
}

void MainWindow::on_buttonAddFileWithDelay_clicked() {
    // FIXME: this is very user-unfriendly. Make it automatic or something.
    double delay = 0.;

    QInputDialog dlg;
    bool dlgresult;
    QString inp = dlg.getText(this, "Please specify delay", "How many delay shall be applied (in seconds; negative means data is placed earlier than what timestamps indicate):", QLineEdit::Normal, "0.0", &dlgresult);
    delay = inp.toDouble();
    cout << "Loading file with delay=" << delay <<"s"<< endl;
    _addFile(delay);
}

void MainWindow::on_buttonDataPrev_clicked() {
    // get combobox item, then request plot to reverse to last data item
    int idx = ui->cboDataSel->currentIndex();
    if (idx < 0) return;
    const Data*d = _get_cboDataSel();
    if (!d) return;

    //qDebug() << "rev for data " << QString::fromStdString(d->get_name());
    d_plot->rev_markerData(d);    
}

void MainWindow::on_buttonDataNext_clicked() {
    // get combobox item, then request plot to reverse to last data item
    int idx = ui->cboDataSel->currentIndex();
    if (idx < 0) return;
    const Data*d = _get_cboDataSel();
    if (!d) return;

    //qDebug() << "fwd for data " << QString::fromStdString(d->get_name());
    d_plot->fwd_markerData(d);    
}

void MainWindow::on_cboDataSel_currentIndexChanged(int index) {
    if (index >= 0) {
        const Data*d = _get_cboDataSel();
        if (!d) return;
        if (_dlgdatatable) _dlgdatatable->setData(d);
    }
}

const Data*MainWindow::_get_cboDataSel(void) {
    const Data*d = NULL;
    // get combobox item, then request plot to reverse to last data item
    int idx = ui->cboDataSel->currentIndex();
    if (idx < 0) return d;
    QVariant q = ui->cboDataSel->itemData(idx, Qt::UserRole);
    void*vp = q.value<void*>(); // FIXME: ugly!
    d = static_cast<const Data*>(vp);
    return d;
}

void MainWindow::on_buttonDataMin_clicked() {
    const Data*d = _get_cboDataSel();
    if (!d) return;

    //qDebug() << "fwd for data " << QString::fromStdString(d->get_name());
    d_plot->setmin_markerData(d);    
}

void MainWindow::on_buttonDataMax_clicked() {
    const Data*d = _get_cboDataSel();
    if (!d) return;

    //qDebug() << "fwd for data " << QString::fromStdString(d->get_name());
    d_plot->setmax_markerData(d);    
}

void MainWindow::on_buttonLogExpand_clicked() {
    _logmsg->expandAll();
}

void MainWindow::on_buttonLogCollapse_clicked() {
    _logmsg->collapseAll();
}

void MainWindow::on_buttonLogRemove_clicked() {
    // get selection of _logmsg
    QItemSelectionModel * mdl = _logmsg->selectionModel();
    if (!mdl) return;
    QModelIndexList list = mdl->selectedIndexes();    
    Logger::Instance().deleteMessages(list);
}

void MainWindow::on_buttonDataTable_clicked() {
    const Data*d = _get_cboDataSel();
    if (!d) return;
    if (!_dlgdatatable) {
        _dlgdatatable = new DialogDataTable(d, d_plot, this);
        _dlgdatatable->setModal(false);
    } else {
        _dlgdatatable->setData(d);

    }
    _dlgdatatable->show();
}
