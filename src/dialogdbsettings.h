/**
 * @file DialogDBSettings.h
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

#ifndef DIALOGSDBSETTINGS_H
#define DIALOGSDBSETTINGS_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include "dbconnector.h"

class DialogDBSettings : public QDialog
{
    Q_OBJECT
public:
    explicit DialogDBSettings(DBConnector::db_props_t * arg, QWidget*parent = 0);

private slots:
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();
    void on_buttonTest_clicked();
    void on_txtUserChanged(const QString & s);
    void on_txtPasswordChanged(const QString &);
    void on_txtDatabaseChanged(const QString & s);
    void on_txtHostChanged(const QString & s);

private:
    void _buildDialog(void);
    void _saveProperties(void);

    /***************************
     *  MEMBER VARIABLES
     ***************************/
    QLineEdit*txtDbhost;
    QLineEdit*txtDbname;
    QLineEdit*txtUsername;
    QLineEdit*txtPassword;

    DBConnector::db_props_t*_dbprops_main; // ptr to main windows's store
    DBConnector::db_props_t _dbprops_tmp; // temporary in this dialog
};

#endif // DIALOGSDBSETTINGS_H
