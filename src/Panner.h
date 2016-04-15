/**
 * @file Panner.h
 * @brief Pans the Plot.
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

#ifndef PANNER_H
#define PANNER_H

#include <qwt_plot_panner.h>
#include "qwt_compat.h"

class Panner: public QwtPlotPanner {
Q_OBJECT

signals:

public:
    Panner(QWT_PLOT_CANVAS * canvas) : QwtPlotPanner(canvas) {}
    void moveCanvas( int dx, int dy ) {
        QwtPlotPanner::moveCanvas(dx, dy);
    }

};

#endif // PANNER_H
