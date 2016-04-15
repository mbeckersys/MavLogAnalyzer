/**
 * @file DialogDBSettings.cpp
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

#include "dialogdbsettings.h"
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include "dbconnector.h"

void DialogDBSettings::_buildDialog(void) {
    // build structure
    QGridLayout*g = new QGridLayout(this);

    unsigned int row=0;

    // DB host
    QLabel*lbl = new QLabel(this);
    lbl->setText("Database host:");
    g->addWidget(lbl, row, 0);
    txtDbhost = new QLineEdit(this);
    g->addWidget(txtDbhost,row, 1);
    txtDbhost->setText(QString().fromStdString(_dbprops_tmp.dbhost));

    // DB name
    lbl = new QLabel(this);
    lbl->setText("Database name:");
    g->addWidget(lbl, ++row, 0);
    txtDbname = new QLineEdit(this);
    g->addWidget(txtDbname, row, 1);
    txtDbname->setText(QString().fromStdString(_dbprops_tmp.dbname));

    // DB user
    lbl = new QLabel(this);
    lbl->setText("User:");
    g->addWidget(lbl, ++row, 0);
    txtUsername = new QLineEdit(this);
    g->addWidget(txtUsername, row, 1);
    txtUsername->setText(QString().fromStdString(_dbprops_tmp.username));

    // DB passwd
    lbl = new QLabel(this);
    lbl->setText("Password:");
    g->addWidget(lbl, ++row, 0);
    txtPassword = new QLineEdit(this);
    g->addWidget(txtPassword, row, 1);
    txtPassword->setText(QString().fromStdString(_dbprops_tmp.password));

    // test button
    QPushButton*btnTest = new QPushButton(this);
    btnTest->setText("Test settings");
    g->addWidget(btnTest, ++row, 0, 1, 2);

    // -- OK & Co.
    QPushButton*btnOK = new QPushButton(this);
    QPushButton*btnCancel = new QPushButton(this);
    g->addWidget(btnCancel, ++row, 0);
    g->addWidget(btnOK, row, 1);
    btnOK->setText("OK");
    btnCancel->setText("Cancel");

    // signals
    connect(btnOK, SIGNAL(clicked()), SLOT(on_buttonOk_clicked()));
    connect(btnCancel, SIGNAL(clicked()), SLOT(on_buttonCancel_clicked()));
    connect(btnTest, SIGNAL(clicked()), SLOT(on_buttonTest_clicked()));
    connect(txtPassword, SIGNAL(textChanged(QString)), SLOT(on_txtPasswordChanged(QString)));
    connect(txtUsername, SIGNAL(textChanged(QString)), SLOT(on_txtUserChanged(QString)));
    connect(txtDbhost, SIGNAL(textChanged(QString)), SLOT(on_txtHostChanged(QString)));
    connect(txtDbname, SIGNAL(textChanged(QString)), SLOT(on_txtDatabaseChanged(QString)));

    //setLayout(v);
    setWindowTitle(QString::fromStdString("Database Settings"));
}

void DialogDBSettings::_saveProperties(void) {
    if (!_dbprops_main) return;
    *_dbprops_main = _dbprops_tmp;
}

DialogDBSettings::DialogDBSettings(DBConnector::db_props_t * arg, QWidget *parent) : QDialog(parent), _dbprops_main(arg), _dbprops_tmp(*arg) {
    _buildDialog();
}

void DialogDBSettings::on_txtPasswordChanged(const QString & s) {
    _dbprops_tmp.password = s.toStdString();
}
void DialogDBSettings::on_txtHostChanged(const QString & s) {
    _dbprops_tmp.dbhost= s.toStdString();
}
void DialogDBSettings::on_txtUserChanged(const QString & s) {
    _dbprops_tmp.username = s.toStdString();
}
void DialogDBSettings::on_txtDatabaseChanged(const QString & s) {
    _dbprops_tmp.dbname = s.toStdString();
}

void DialogDBSettings::on_buttonTest_clicked() {


    DBConnector* dbCon = new DBConnector(_dbprops_tmp);
    std::string errmsg;
    bool success = dbCon->selfTest(errmsg);
    if (success) {
        QMessageBox msgbox(QMessageBox::Information, QString("All right"), QString("Your settings are working."));
        msgbox.exec();
    } else {
        QMessageBox msgbox(QMessageBox::Critical, QString("Nope"), QString("Something is wrong. Error message:" + QString().fromStdString(errmsg)));
        msgbox.exec();
    }

    delete dbCon;
}

void DialogDBSettings::on_buttonOk_clicked() {
    _saveProperties();
    this->close();
}

void DialogDBSettings::on_buttonCancel_clicked() {
    this->close();
}
