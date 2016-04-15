/**
 * @file DialogScenarioProps.cpp
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

#include "dialogscenarioprops.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLineEdit>

void DialogScenarioProps::_buildDialog(void) {
    // build structure
    QVBoxLayout*v = new QVBoxLayout(this);    

    // rename option
    QHBoxLayout*h0 = new QHBoxLayout(); // parent set in next line
    v->addLayout(h0);
    QLabel*lblName = new QLabel(this);
    lblName->setText("Scenario Name:");
    h0->addWidget(lblName);
    txtName = new QLineEdit(this);
    h0->addWidget(txtName);
    txtName->setText(QString().fromStdString(_scene->getName()));

    // information on time and more
    QHBoxLayout*h1 = new QHBoxLayout(); // parent set in next line
    v->addLayout(h1);
    QLabel*lblStart = new QLabel(this);
    lblStart->setText("Start Time:");
    h1->addWidget(lblStart);
    QLabel*lblStartr = new QLabel(this);
    h1->addWidget(lblStartr);
    QString tstart = QString().fromStdString(epoch_to_datetime(_scene->get_scenario_starttime_sec()));
    lblStartr->setText(tstart);

    // description
    QLabel*lblDesc = new QLabel(this);
    lblDesc->setText("Description:");
    v->addWidget(lblDesc); // that should re-parent
    txtDesc = new QTextEdit(this);
    v->addWidget(txtDesc);
    txtDesc->setText(QString::fromStdString(_scene->getDescription()));

    // -- OK & Co.
    QHBoxLayout*hOk = new QHBoxLayout(); // parent set in next line
    v->addLayout(hOk);
    QPushButton*btnOK = new QPushButton(this);
    QPushButton*btnCancel = new QPushButton(this);
    hOk->addWidget(btnCancel);
    hOk->addWidget(btnOK);
    btnOK->setText("OK");
    btnCancel->setText("Cancel");

    // signals
    connect(btnOK, SIGNAL(clicked()), SLOT(on_buttonOk_clicked()));
    connect(btnCancel, SIGNAL(clicked()), SLOT(on_buttonCancel_clicked()));

    //setLayout(v);
    setWindowTitle(QString::fromStdString("Scenario Properties"));
}

void DialogScenarioProps::_saveProperties(void) {
    if (!_scene) return;
    _scene->setName(txtName->text().toStdString());
    _scene->setDescription(txtDesc->toPlainText().toStdString());
}

DialogScenarioProps::DialogScenarioProps(MavlinkScenario*const scene, QWidget *parent) : QDialog(parent), _scene(scene) {
    if (!scene) return;
    _buildDialog();
}

void DialogScenarioProps::on_buttonOk_clicked() {
    _saveProperties();
    this->close();
}

void DialogScenarioProps::on_buttonCancel_clicked() {
    this->close();
}
