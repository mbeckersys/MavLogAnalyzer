/**
 * @file DialogScenarioProps.h
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

#ifndef DIALOGSCENPROPS_H
#define DIALOGSCENPROPS_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include "mavlinkscenario.h"

class DialogScenarioProps : public QDialog
{
    Q_OBJECT
public:
    explicit DialogScenarioProps(MavlinkScenario*const scen, QWidget*parent = 0);

private slots:
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();

private:
    void _buildDialog(void);
    void _saveProperties(void);

    /***************************
     *  MEMBER VARIABLES
     ***************************/
    QLineEdit*txtName;
    QTextEdit*txtDesc;
    MavlinkScenario*_scene;
};

#endif // DIALOGSCENPROPS_H
