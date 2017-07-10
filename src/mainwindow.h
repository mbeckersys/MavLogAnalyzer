/**
 * @file mainwindow.h
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 2014-Apr-17

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <list>
#include <QMainWindow>
#include <QSettings>
#include <QItemSelectionModel>
#include <QPolygon>
#include <QTreeView>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qstringlistmodel.h>
#include <qstandarditemmodel.h>
#include "mavlinkparser.h"
#include "onboardlogparser_apm.h"
#include "mavlinkscenario.h"
#include "SystemTableviewModel.h"
#include "datatreeviewmodel.h"
#include "dialogprogressbar.h"
#include "dialogstats.h"
#include "dialogscenarioprops.h"
#include "dialogdatatable.h"
#include "mavplot.h"
#include "Zoomer.h"
#include "Panner.h"
#include "cmdlineargs.h"
#include "dbconnector.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class FilterWindow;
public:
    explicit MainWindow(std::list<MavlinkParser*> &parsers, CmdlineArgs *const args, QWidget *parent = 0);
    ~MainWindow();

    void showProgressBar();
    void updateProgressBarValue(unsigned int val, unsigned int max);
    void updateProgressBarTitle(QString text);
    void hideProgressBar();

public slots:
    void on_plotZoomed(float viewmin, float viewmax);
    void on_plotPanned(int dx, int dy);

private slots:
    void on_systemSelectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/);
    void on_datarowSelectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/);
    void on_DBSelectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/);
    void moved(const QPoint &);
    void selected(const QPolygon &);
    void print();
    void notimplementedyet();
    void exportDocument();
    void exportCsv();
    void enableLockMode(bool);
    void enableVLockMode(bool on);
    void enableHLockMode(bool on);
    void showInfo(QString text = "");
    void on_scrollHPlot_sliderMoved(int position);
    void on_buttonAddData_clicked();
    void on_buttonAutoFit_clicked();
    void on_buttonAddFile_clicked();
    void on_buttonSetMarkerA(bool on);
    void on_buttonSetMarkerB(bool on);
    void on_buttonSetMarkerData(bool on);
    void on_buttonSetXZoom(bool checked);
    void on_buttonSetYZoom(bool on);
    void on_buttonClear_clicked();
    void on_treeData_doubleClicked(const QModelIndex &index);
    void on_buttonCalcStats_clicked();
	void on_buttonSearchDB_clicked();
    void on_buttonClearScenario_clicked();
    void on_buttonSaveDB_clicked();
    void on_buttonScenarioProps_clicked();
    void on_buttonSetupDB_clicked();     
    void on_buttonAddFileWithDelay_clicked();
    void on_buttonDataPrev_clicked();
    void on_buttonDataNext_clicked();
    void on_cboDataSel_currentIndexChanged(int index);
    void on_buttonDataMin_clicked();
    void on_buttonDataMax_clicked();
    void on_buttonDataTable_clicked();
    void on_buttonLogExpand_clicked();
    void on_buttonLogCollapse_clicked();
    void on_buttonLogRemove_clicked();

signals:
    void systemSelectionChangedSignal(); ///< indicate that someone clicked on another system -> we need to reload TreeView and the info box

private:
    void _addDataToPlot(TreeItem * item);
    void _addFile(double delay = 0.0);
    bool _fileLoaded(const QString &fname) const;
    MavlinkScenario* _forceChooseScenario(const std::vector<MavlinkScenario*>& items) const ;
    void _load_windows_settings(void);
    void _save_windows_settings(void);
    void _setupSignalsAndSlots(void);
    void _setupPlotWidget(void);
    void _setupToolbar(void);
    void _setupFilelist(void);
    void _buildDialog(void);
    void _styling(void);
    void _updateHScroll(void);
    void _updateTreeData(const MavSystem*const sys);
    void _updateTextInfo(const MavSystem*const sys);
    void _clearScenario(void);
    const Data*_get_cboDataSel(void);
    QStringList _getFileNames(void);

    /******************************
     *  DATA MEMBERS
     ******************************/
    Ui::MainWindow *ui;
    CmdlineArgs*_args;
    QSettings _settings;

    Data*_dataSelected; // cannot be const because of lazy load
    DataGroup*_datagroupSelected;

    const MavSystem*_lastsys;
    MavlinkScenario*_analyzer;
    std::list<MavlinkParser*>*_parsers;
    SystemTableViewModel*_stvm;
    DataTreeViewModel *_dtvm;

    // for plot
    MavPlot         *d_plot;
    Zoomer          *d_zoomer;
    QwtPlotPicker   *d_picker;
    Panner          *d_panner;

    // for A-B marker & data cursor
    bool _markerA;
    bool _markerB;
    bool _markerData;

    // for progress bar
    DialogProgressBar*_dlgprogress;
    DialogStats * _dlgstats;

    // for log messages
    QTreeView*_logmsg;

    // for data views
    DialogDataTable*_dlgdatatable;

    // for database
	QStandardItemModel *_DBResultModel;
    DBConnector::db_props_t _dbprops;
};

#endif // MAINWINDOW_H
