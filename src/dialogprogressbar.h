/**
 * @file dialogprogressbar.h
 * @brief Opens and analyzes logfiles with MAVLink data from ardupilot.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 02.05.2014
 
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

#ifndef DIALOGPROGRESSBAR_H
#define DIALOGPROGRESSBAR_H

#include <QString>
#include <QDialog>
#include <QProgressBar>
#include <QLabel>

class DialogProgressBar : public QDialog
{
    Q_OBJECT
public:
    explicit DialogProgressBar(QWidget *parent = 0);
    void setLabel(const QString & text);
    void setValue(unsigned int value, unsigned int max);

signals:
    
public slots:
    
private:
    QProgressBar*_progressbar;
    QLabel*_lbl;
};

#endif // DIALOGPROGRESSBAR_H
