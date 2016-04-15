/**
 * @file dialogselectscenario.cpp
 * @brief Out of a list of scenarios, make the user chose exactly one
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 11/17/2015
 
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

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QDebug>
#include "dialogselectscenario.h"

using namespace std;

DialogSelectScenario::DialogSelectScenario(const std::vector<MavlinkScenario*> & scenes, QWidget *parent) :
    QDialog(parent), _curScene(NULL)
{
    _scenes = scenes; ///< deep copy

    QLabel*labelscene = new QLabel(tr("Choose one scenario:"));
    QPushButton*btn = new QPushButton(tr("Select"));
    _scentable = new QTableWidget;
    _scentable->setHorizontalHeaderLabels(QString("Name").split(";"));
    _scentable->setColumnCount(1);
    _scentable->setRowCount(scenes.size());


    unsigned int r=0;
    for (std::vector<MavlinkScenario*>::const_iterator s = scenes.begin(); s != scenes.end(); ++s, ++r) {
        QTableWidgetItem*it = new QTableWidgetItem(QString::fromStdString(scenes.at(r)->getName()));
        _scentable->setItem(r,0,it);
    }
    _scentable->setSelectionBehavior(QAbstractItemView::SelectRows); // entire row
    _scentable->setSelectionMode(QAbstractItemView::SingleSelection); // exactly one
    _scentable->selectRow(0); // by default select first
    _scentable->horizontalHeader()->setStretchLastSection(true);

    QLabel*labelsys = new QLabel(tr("Systems in selected scenario:"));
    _systable = new QTableWidget;
    _systable->setSelectionBehavior(QAbstractItemView::SelectRows); // entire row
    _systable->setSelectionMode(QAbstractItemView::SingleSelection); // exactly one
    _systable->horizontalHeader()->setStretchLastSection(true);

    _stext = new QTextBrowser();
    _stext->setReadOnly(true);

    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(labelscene);
    l->addWidget(_scentable);
    l->addWidget(labelsys);
    l->addWidget(_systable);
    l->addWidget(_stext);
    l->addWidget(btn);

    setLayout(l);
    setWindowTitle(tr("Scenario List"));
    setFixedHeight(sizeHint().height());

    // slots
    connect(_scentable, SIGNAL(itemSelectionChanged()), this, SLOT(sceneSelectionChanged()));
    connect(_systable, SIGNAL(itemSelectionChanged()), this, SLOT(sysSelectionChanged()));
    connect(btn, SIGNAL(clicked()), this, SLOT(selectClicked()));

    // default selection
    _scentable->selectRow(0);
    sceneSelectionChanged();
}

void DialogSelectScenario::sysSelectionChanged() {
    if (!_curScene) return;

    // which system within the scenario was selected?
    QItemSelectionModel* sel = _systable->selectionModel();
    if (!sel) return;
    QModelIndexList selli  = sel->selectedRows();
    if (selli.empty()) return;
    int row =selli.first().row();

    std::stringstream ss;

    // get it from active scenario
    std::vector<const MavSystem*> systems = _curScene->getSystems();
    if (row >= 0 && row < (int)systems.size()) {
        const MavSystem*s = systems[row];
        if (s) {

            std::string summary;
            s->get_summary(summary);
            ss << summary << endl;
        }
    }
    _stext->setText(QString::fromStdString(ss.str()));
}

void DialogSelectScenario::sceneSelectionChanged() {
    QItemSelectionModel* sel = _scentable->selectionModel();
    if (!sel) return;
    QModelIndexList selli  = sel->selectedRows();
    if (selli.empty()) return;
    int row = selli.first().row();

    // load scene
    if (row >= 0 && row < (int)_scenes.size()) {
        MavlinkScenario*scene = _scenes.at(row);
        _curScene = scene;
        std::vector<const MavSystem*> systems = scene->getSystems();

        if (scene) {
            std::stringstream ss;
            ss << "Start @" << scene->get_scenario_starttime_sec() << std::endl;

            ss << scene->getDescription();                
            std::string strdesc = ss.str();
            _stext->setText(QString::fromStdString(strdesc));

            // fill system table
            _systable->reset();
            _systable->clear();
            QStringList head = QString("ID;MAV Type;AP Type").split(";");
            const int ncols = head.size();
            _systable->setHorizontalHeaderLabels(head);
            _systable->setColumnCount(ncols);
            _systable->setRowCount(systems.size());
            unsigned int r = 0;
            for (std::vector<const MavSystem*>::const_iterator sit = systems.begin(); sit != systems.end(); ++sit, ++r) {
                unsigned int c = 0;
                const MavSystem*sys = *sit;
                QTableWidgetItem*it = new QTableWidgetItem(QString::number(sys->get_id()));
                _systable->setItem(r,c++,it);
                it = new QTableWidgetItem(QString::fromStdString(sys->get_mavtype()));
                _systable->setItem(r,c++,it);
                it = new QTableWidgetItem(QString::fromStdString(sys->get_aptype()));
                _systable->setItem(r,c++,it);
            }
            if ( r > 0 ) _systable->selectRow(0);
        }

    }

}

void DialogSelectScenario::selectClicked() {
    QItemSelectionModel* sel = _scentable->selectionModel();
    if (!sel) return;
    QModelIndexList selli  = sel->selectedRows();
    if (selli.empty()) return;
    int row = selli.first().row();

    this->setResult(row); // this seems logical, so it stays ;)
    this->close();
    this->setResult(row); // it seems stupid...but needs to be called after close
}
