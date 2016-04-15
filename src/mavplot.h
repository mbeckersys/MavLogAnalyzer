/**
 * @file mavplot.h
 * @brief Custom plot widget based on Qwt.
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

#ifndef MAVPLOT_H
#define MAVPLOT_H

#include <set>
#include <QVector>
#include <QStandardItemModel>
#include <QStatusBar>
#include <qwt_plot.h>
#include <qwt_plot_seriesitem.h>
#include <qwt_plot_marker.h>
#include "qwt_compat.h"
#include "data.h"
#include "data_timeseries.h"
#include "data_event.h"
#include "dialogstats.h"
#include "mavplotdataitemmodel.h"

class MavPlot : public QwtPlot
{
    Q_OBJECT
public:

    /****************************
     * CONSTANTS
     ****************************/

    static const unsigned int PLOT_LINE_WIDTH = 3;

    /****************************
     * METHODS
     ****************************/

    explicit MavPlot(QWidget *parent = 0);

    ~MavPlot();

    /**
     * @brief addData
     * @param data
     * @return true if data recognized and added, else false
     */
    bool addData(const Data*const data);

    /**
     * @brief returns the min/max values over all data in the plot
     * @return a rectangle that is a hull over all data series
     */
    const QRectF & get_data_bounds() const {
        return _databounds;
    }

    /**
     * @brief returns an ItemModel holding list of <Data*> currently in this plot
     * This class maintains this model, so widgets which refer to this model get
     * automatically updated when something changes here
     * @return
     */
    MavplotDataItemModel*get_datamodel() {
        return static_cast<MavplotDataItemModel*>(&_model);
    }

    /**
     * @brief get_num_data
     * @return number of data rows/series in the plot
     */
    unsigned int get_num_data() const {
        return _series.size();
    }

    QString getReadableTime(double timeval) const;

    /**
     * @brief enables the plot to write stuff in the statusbar
     * @param b
     */
    void set_statusbar(QStatusBar*b) { _statusbar = b; }

    /**
     * @brief removes data series from plot, and everything which is connected to it.
     * @param Data
     */
    //void removeDataItem(const Data * const);

    void set_markerA(double x);
    void set_markerB(double x);
    void set_markerData(double x); // jump to time mark, independent of data points
    bool set_markerData(const Data * const, unsigned long idx); // jump to index of given row
    void fwd_markerData(const Data * const); // forward marker to next data point of given row
    void rev_markerData(const Data * const); // reverse
    void setmax_markerData(const Data *const d); // set marker to max of current row
    void setmin_markerData(const Data *const d); // set marker to min of current row
    void unset_markerA();
    void unset_markerB();
    void unset_markerData();

    /**
     * @brief rell me about marker positions
     * @return
     */
    QString verboseMarkers() const ;

    /**
     * @brief write stuff to the statusbar, if any given
     */
    void updateStatusbar(void);

    /**
     * @brief call this to clear the plot
     */
    void removeAllData();

    /**
     * @brief convert Data class to two QVectors, which can be plotted
     */
    static bool data2xyvect(const Data * data, QVector<double> & xdata, QVector<double> & ydata, double scale=1.0);
    template <typename ST>
    static bool data2xyvect(const DataTimeseries<ST> * data, QVector<double> & xdata, QVector<double> & ydata, double scale=1.0);

    /**
     * @brief apply/undo coloring for print and PDF export
     * @param yes
     */
    void apply_print_colors(bool yes);

    /**
     * @brief write a CSV file with those data rows in our window
     * @param filename
     * @param onlyview if true, only the displayed data (not the one that is out of window) is exported
     * @return number of data series written
     */
    unsigned int exportCsv(const std::string& filename, bool onlyview = false);
signals:
            
private slots:
    void legendClicked(QwtPlotItem*item);
    void legendClickedNew(const QVariant&, int);
    void onRemoveData(const Data * const);

private:
    /**********************************************************
     *  FUNCTIONS WITH HANDLE PLOTTING FOR VARIOUS DATA TYPES
     **********************************************************/
    // series
    QWT_ABSTRACT_SERIESITEM * _branch_datatype_series(const Data* data, unsigned int plotnumber);
    template <typename ST>
    QWT_ABSTRACT_SERIESITEM * _add_data_timeseries(const DataTimeseries<ST> * data, unsigned int plotnumber);

    // annotations
    std::vector<QwtPlotItem*>* _branch_datatype_annotation(const Data* data, unsigned int plotnumber);
    template <typename ET>
    std::vector<QwtPlotItem*>* _add_data_event(const DataEvent<ET> * data, unsigned int plotnumber);

    QWT_ABSTRACT_SERIESITEM *_get_series(const Data * const d);
    std::vector<QwtPlotItem*>* _get_annotations(const Data * const d);

    /**
     * @brief MavPlot::_updateDataBounds
     * update internal variable _databounds to indicate overall data range
     */
    void _updateDataBounds ();

    /**
     * @brief converty "any" data type to double
     */
    template <typename CT>
    static bool _convert2double(CT in, double &ret, double scale=1.);

    /**
     * @brief returns a color to be used for each data, based on the sequence number of the plot
     * @param plotnumber
     * @return color
     */
    QColor _suggestColor(unsigned int plotnumber);

    /**
     * @brief remove specific data from plot
     * @param d
     * @return true if something was removed, else false
     */
    bool _removeData(const Data *const d);    

    /**
     * @brief maintain Item Model for the widgets that need my data
     * @param d
     */
    void _model_append(const Data *d);
    void _model_remove(const Data *d);
    void _model_clear(void);

    /*********************************
     *  MEMBER VARIABLES
     *********************************/

    /**
     * @brief for each added data we save the pointer to data and the QwtData
     * We use the abstractSeriesItem to be able to store every kind of plot,
     * e.g, curves as well as histograms.
     */
    typedef std::map<Data const*, QWT_ABSTRACT_SERIESITEM *> dataplotmap;
    typedef std::pair<Data const*, QWT_ABSTRACT_SERIESITEM *> dataplotmap_pair;

    /**
     * @brief for each added data we can moreover have annotations that are
     * connected to it. the annotations would have to be removed, if the data is removed.
     * some data type might also just result in an annotation, and not in a SeriesItem.
     * E.g., DataEvent will just result in markers, not in a data row.
     */
    typedef std::map<Data const*, std::vector<QwtPlotItem*>* > annotationsmap;
    typedef std::pair<Data const*, std::vector<QwtPlotItem*>* > annotationsmap_pair;

    // THE data (TM) (C) (R)
    dataplotmap _series;
    annotationsmap _annotations;

    // for widgets external to this plot that want to access the plotted data
    MavplotDataItemModel _model; // FIXME: derive this class?

    // A-B markers
    QwtPlotMarker _user_markers[2];
    bool _user_markers_visible[2];

    // data marker
    QwtPlotMarker _data_marker;
    bool _data_marker_visible;
    QString _data_marker_label;

    // for optimizing the visual representation
    QRectF _databounds;
    ///< available color

    // for print colors
    bool _havePrintColors;
    QColor _col_legend_screen;
    QColor _col_title_screen;
    QPalette _pal_axis_screen;

    // status bar
    QStatusBar*_statusbar;

    friend class DialogStats;
};

#endif // MAVPLOT_H
