/**
 * @file qwt_compat.h
 * @brief makes it possible to use qwt in different versions
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 2014-Aug-23
 */

#ifndef QWT_COMPAT_H
#define QWT_COMPAT_H

#include <qwt_global.h>

/**************************
 *  MACROS
 **************************/
#define QWT_VERSION_CHECK(major, minor, patch) ((major << 16) + (minor << 8) + patch)
// used as: #if QWT_VERSION >= QWT_VERSION_CHECK(6,1,0)...

/**************************
 *  VERSION SPECIFIC HACKS
 **************************/
// >= 6.1.0: class QwtPlotAbstractSeriesItem -> class QwtPlotSeriesItem
#if QWT_VERSION >= QWT_VERSION_CHECK(6,1,0)
    #define QWT_ABSTRACT_SERIESITEM QwtPlotSeriesItem
#else
    #define QWT_ABSTRACT_SERIESITEM QwtPlotAbstractSeriesItem
#endif

// >= 6.1.0 Panner and Zoomer use class QwtPlotCanvas -> QWidget
#if QWT_VERSION >= QWT_VERSION_CHECK(6,1,0)
    #define QWT_PLOT_CANVAS QWidget
#else
    #define QWT_PLOT_CANVAS QwtPlotCanvas
#endif

#endif // QWT_COMPAT_H
