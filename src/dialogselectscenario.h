/**
 * @file dialogselectscenario.h
 * @brief TODO: describe this file
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

#ifndef DIALOGSELECTSCENARIO_H
#define DIALOGSELECTSCENARIO_H

#include <QDialog>
#include <QTableWidget>
#include <QTextBrowser>
#include <vector>
#include "mavlinkscenario.h"

class DialogSelectScenario : public QDialog
{
    Q_OBJECT
public:
    explicit DialogSelectScenario(const std::vector<MavlinkScenario*> & scenes, QWidget *parent = 0);
    
signals:
    
public slots:
    void selectClicked();
    void sceneSelectionChanged();
    void sysSelectionChanged();
    
private:
    QTableWidget*_scentable;
    QTableWidget*_systable;
    QTextBrowser*_stext;
    std::vector<MavlinkScenario*> _scenes;
    MavlinkScenario*_curScene;
};

#endif // DIALOGSELECTSCENARIO_H
