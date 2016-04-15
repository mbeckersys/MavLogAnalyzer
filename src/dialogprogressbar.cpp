/**
 * @file dialogprogressbar.cpp
 * @brief Shows a simple progress bar
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

#include <QVBoxLayout>
#include "dialogprogressbar.h"

DialogProgressBar::DialogProgressBar(QWidget *parent) :
    QDialog(parent) {

    QVBoxLayout*v = new QVBoxLayout(this);
    setLayout(v);

    setWindowTitle("Please wait...");

    _lbl = new QLabel(this);
    _lbl->setText("Progress...");
    _progressbar = new QProgressBar(this);
    _progressbar->setValue(50);
    v->addWidget(_lbl);
    v->addWidget(_progressbar);
}

void DialogProgressBar::setLabel(const QString &text) {
    _lbl->setText(text);
}

void DialogProgressBar::setValue(unsigned int value, unsigned int max) {
    _progressbar->setValue(value);
    _progressbar->setMaximum(max);
}
