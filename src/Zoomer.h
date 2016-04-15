/**
 * @file Zoomer.h
 * @brief Zoom the plot
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 20.04.2014
 
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

#ifndef ZOOMER_H
#define ZOOMER_H

#include <QPolygon>
#include <QTransform>
#include <qwt_global.h>
#include <qwt_plot_zoomer.h>
#include "qwt_compat.h"

/**
 * @brief enable zoom for qwt plots
 */
class Zoomer: public QwtPlotZoomer {
Q_OBJECT

signals:
    void plotMoved(float viewmin, float viewmax); ///< indicate that the axis shifted due to pan or zoom

public:
    Zoomer(int xAxis, int yAxis, QWT_PLOT_CANVAS *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas), _xzoom(false), _yzoom(false) {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);

        setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton); // RightButton: zoom out by 1
        setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier); // Ctrl+RightButton: zoom out to full size

    }

    void rescale() {
        QwtPlotZoomer::rescale();
        const QRectF & rect = zoomRect();
        float viewmin = rect.bottomLeft().x();
        float viewmax = rect.bottomRight().x();
        emit plotMoved(viewmin, viewmax);
    }

    void setZoomBase() {
        QwtPlotZoomer::setZoomBase();
    }

    void setXZoom (bool on) {
        _xzoom = on;
    }

    void setYZoom (bool on) {
        _yzoom = on;
    }

protected:
    // implements constrained zooming: separate X and Y zooming
    bool accept( QPolygon & poly) const {
        if (!(_xzoom || _yzoom)) return true;

        // x-y zooming

        // get bounding box, correct corners and return manipulated QPolygon
        QRect bb = poly.boundingRect();
        QRect cur = transform(zoomRect()); // turn current zoom rect into pixels

        int xmin = cur.topLeft().x();
        int xmax = cur.bottomRight().x();
        int ymin = cur.topLeft().y();
        int ymax = cur.bottomRight().y();

        bool needsUpdate = false;
        if (_xzoom) {
            // preserve y bounds
            if (bb.top() != ymin) {
                bb.setTop(ymin);
                needsUpdate = true;
            }
            if (bb.bottom() != ymax) {
                bb.setBottom(ymax);
                needsUpdate = true;
            }
        }
        if (_yzoom) {
            // preserve x bounds
            if (bb.left() != xmin) {
                bb.setLeft(xmin);
                needsUpdate = true;
            }
            if (bb.right() != xmax) {
                bb.setRight(xmax);
                needsUpdate = true;
            }
        }
        if (needsUpdate) {
            // re-do polygon
            poly.setPoint(0, bb.topLeft());
            poly.setPoint(1, bb.bottomRight());
        }

        return true;
    }

private:
    // for locked zooms
    bool _xzoom;
    bool _yzoom;
};

#endif // ZOOMER_H
