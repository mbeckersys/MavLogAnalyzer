/**
 * @file mavplot.cpp
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

#include <vector>
#include <sstream>
#include <QMessageBox>
#include <qwt_math.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_color_map.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <QTime>
#include <qmath.h>
#include "mavplot.h"
#include "data_timeseries.h"
#include "data_event.h"
#include "dialogdatadetails.h"

using namespace std;

/**
 * @brief labels at x axis as human-readable time
 */
class HumanReadableTime: public QwtScaleDraw {
public:
    HumanReadableTime(): QwtScaleDraw() {  }
    // just override the label formatting
    virtual QwtText label(double v_sec) const {        

        // isolate only hh:mm:ss:zzzz
        unsigned long epoch_msec = round(v_sec*1000);
        QDateTime qdt = QDateTime::fromMSecsSinceEpoch(epoch_msec);
        QTime daytime(qdt.time());
        return daytime.toString("hh:mm:ss.zzz");
    }    
};

QString MavPlot::getReadableTime(double timeval) const {
    QwtText txt = axisScaleDraw(QwtPlot::xBottom)->label(timeval);
    return QString(txt.text());
}

unsigned int MavPlot::exportCsv(const std::string&filename, bool /* onlyview -> ignored for now*/) {
    unsigned int n_series = 0;
    //std::ofstream fout(filename.c_str());

    for (dataplotmap::iterator it_series = _series.begin(); it_series != _series.end(); ++it_series) {
        const Data*const d = it_series->first;
        if (d) {
            d->export_csv(filename + "_" + d->get_name() + ".csv");
            n_series++;
        }
    }
    for (annotationsmap::iterator it_annot = _annotations.begin(); it_annot != _annotations.end(); ++it_annot) {
        const Data*const d = it_annot->first;
        if (d) {
            d->export_csv(filename + "_" + d->get_name() + ".csv");
            n_series++;
        }
    }

    //fout.close();
    return n_series;
}

void MavPlot::apply_print_colors(bool yes) {
    if (yes && !_havePrintColors) {
        const QColor printcol = QColor(0,0,0); // default color: black
        /*******************
         *  ENABLE PRINT COLS
         *******************/
        // title
        QwtText plottitle = title();
        _col_title_screen = plottitle.color();
        plottitle.setColor(printcol);
        setTitle(plottitle);
        // all legend entries ->
        for (dataplotmap::iterator d = _series.begin(); d != _series.end(); ++d) {
            QWT_ABSTRACT_SERIESITEM * s = d->second;
            if (!s) continue;
            if (d == _series.begin()) {
                _col_legend_screen = s->title().color(); // save color of first legend text
            }

            // thicker lines for all
            QwtPlotCurve * const q = dynamic_cast<QwtPlotCurve * const>(s);
            if (q) {
                QPen p = q->pen();
                p.setWidth(PLOT_LINE_WIDTH);
                p.setCosmetic(true);
                q->setPen(p);
            }

            // text of legend -> black
            QwtText title = s->title();
            title.setColor(printcol);
            s->setTitle(title);
        }
        // axes
        _pal_axis_screen = axisWidget(QwtPlot::xBottom)->palette();
        QPalette pal_print;
        pal_print.setColor(QPalette::WindowText, printcol);
        pal_print.setColor(QPalette::Text, printcol);
        axisWidget(QwtPlot::xBottom)->setPalette(pal_print);
        axisWidget(QwtPlot::yLeft)->setPalette(pal_print);
    } else {
        if (_havePrintColors) {
            /*******************
             *  UNDO
             *******************/
            // title
            QwtText plottitle = title();
            plottitle.setColor(_col_title_screen);
            setTitle(plottitle);
            // all legend entries ->
            for (dataplotmap::iterator d = _series.begin(); d != _series.end(); ++d) {
                QWT_ABSTRACT_SERIESITEM * s = d->second;
                if (!s) continue;
                // thinner lines for all
                QwtPlotCurve * const q = dynamic_cast<QwtPlotCurve * const>(s);
                if (q) {
                    QPen p = q->pen();
                    p.setWidth(1);
                    p.setCosmetic(false);
                    q->setPen(p);
                }
                // restore legend text color
                QwtText title = s->title();
                title.setColor(_col_legend_screen);
                s->setTitle(title);
            }            
            // axes
            axisWidget(QwtPlot::xBottom)->setPalette(_pal_axis_screen);
            axisWidget(QwtPlot::yLeft)->setPalette(_pal_axis_screen);
        }
    }
    _havePrintColors = yes;
}

void MavPlot::_updateDataBounds () {
    bool first = true;
    QRectF allrect;
    for (dataplotmap::const_iterator s = _series.begin(); s != _series.end(); ++s) {
        const QWT_ABSTRACT_SERIESITEM * const that = s->second;
        // FIXME: now we can only do curves
        const QwtPlotCurve * const q = dynamic_cast<const QwtPlotCurve * const>(that);
        if (q) {
            QRectF rect = q->boundingRect();
            if (first) {
                first=false;
                allrect = rect;
            } else {
                #if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
                    allrect = allrect.united(rect);
                #else
                    allrect = allrect.unite(rect);
                #endif
            }
        }
    }
    _databounds = allrect;
}

// for Qwt >= 6.1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void MavPlot::legendClickedNew(const QVariant& q, int /*i*/) {

    //obtain QwtPlotItem and then delegate to legendClicked().
    #if (QWT_VERSION < QWT_VERSION_CHECK(6,1,0))
        return;
    #else
        QwtPlotItem*pi = infoToItem(q);
        if (!pi) return;
        legendClicked(pi);
    #endif
}
#pragma GCC diagnostic pop

void MavPlot::legendClicked(QwtPlotItem*item) {
    // see if it is seriesitem...only these react
    QWT_ABSTRACT_SERIESITEM * si = dynamic_cast<QWT_ABSTRACT_SERIESITEM *>(item);
    if (si) {
        // look up the data behind it the series
        /*
         * FIXME: for now we do an exhaustive search for the key (Data), given the value (si)
         * if it gets too slow one day, consider implementing a reverse map
         */
        for (dataplotmap::const_iterator it = _series.begin(); it != _series.end(); ++it) {
            if (it->second == si) {
                const Data*const d = it->first;
                QWT_ABSTRACT_SERIESITEM * const s = it->second;
                // open up a dialog with details. this dialog also allows to delete data from plot.
                DialogDataDetails*dialog = new DialogDataDetails(d, s, _model);
                connect(dialog, SIGNAL(removeDataFromPlot(Data*const)), SLOT(onRemoveData(Data*const)));
                dialog->show();
                disconnect(this, SLOT(onRemoveData(Data*const)));
                return;
            }
        }
    }

    // now it is an annotation
    for (annotationsmap::const_iterator it = _annotations.begin(); it != _annotations.end(); ++it) {
        if (it->second->front() == item) {
            const Data*const d = it->first;
            vector<QwtPlotItem*> *s = it->second;
            // open up a dialog with details. this dialog also allows to delete data from plot.
            DialogDataDetails*dialog = new DialogDataDetails(d, s, _model);
            connect(dialog, SIGNAL(removeDataFromPlot(Data*const)), SLOT(onRemoveData(Data*const)));
            dialog->show();
            disconnect(this, SLOT(onRemoveData(Data*const)));
            return;
        }
    }

    qDebug() << "Weird...cannot find associated data for legend item";
    return;
}

bool MavPlot::_removeData(const Data *const d) {
    _model.model_remove(d);

#if 1
    // look up whether data has series
    dataplotmap::iterator it_series = _series.find(d);
    if (it_series != _series.end()) {
        // yes -> remove
        QWT_ABSTRACT_SERIESITEM *s = dynamic_cast<QWT_ABSTRACT_SERIESITEM *>(it_series->second);
        if (s) {
            delete s; // that should do it
        }
        _series.erase(it_series); // erase Data from annotations list
        return true;
    }
#else
    QWT_ABSTRACT_SERIESITEM*s = _get_series(d);
    if (s) {
        delete s;
        _series.erase(TODO); // missing iterator here
        return true;
    }

#endif

    // look up whether data has annotations
    annotationsmap::iterator it_annot = _annotations.find(d);
    if (it_annot != _annotations.end()) {
        // yes -> remove
        std::vector<QwtPlotItem*>*v = dynamic_cast<std::vector<QwtPlotItem*>* >(it_annot->second);
        if (v) {
            // free all annotations in the vector...
            for (std::vector<QwtPlotItem*>::iterator it_vect = v->begin(); it_vect != v->end(); ++it_vect) {
                delete *it_vect;
            }
            delete v; // free vector itself
        }
        _annotations.erase(it_annot); // erase Data from annotations list
        return true;
    }
    return false;
}

void MavPlot::onRemoveData(const Data *const d) {
    bool removed = _removeData(d);

    // conditionally update the internal data and replot
    if (removed) {
        _updateDataBounds();
        replot();
    }
}

MavPlot::MavPlot(QWidget *parent) : QwtPlot(parent), _havePrintColors(false) {
    setAutoReplot(false);
    setTitle("MAV System Data Plot");
    setCanvasBackground(QColor(Qt::darkGray));

    // legend
    QwtLegend *legend = new QwtLegend;
    insertLegend(legend, QwtPlot::BottomLegend);
    #if (QWT_VERSION < QWT_VERSION_CHECK(6,1,0))
        legend->setItemMode(QwtLegend::ClickableItem);
        connect(this, SIGNAL(legendClicked(QwtPlotItem*)), SLOT(legendClicked(QwtPlotItem*)));
    #else
        legend->setDefaultItemMode(QwtLegendData::Clickable);        
        connect(legend, SIGNAL(clicked(const QVariant&, int)), SLOT(legendClickedNew(const QVariant&, int)));
    #endif    

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    #if (QWT_VERSION < QWT_VERSION_CHECK(6,1,0))
        grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
        grid->setMinPen(QPen(QColor(0x8, 0x8, 0x8), 0 , Qt::DotLine));
    #else
        grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
        grid->setMinorPen(QPen(QColor(0x8, 0x8, 0x8), 0 , Qt::DotLine));
    #endif
    grid->attach(this);

    // axes
    //enableAxis(QwtPlot::yRight);
    setAxisTitle(QwtPlot::xBottom, "time");
    setAxisScaleDraw(QwtPlot::xBottom, new HumanReadableTime()); // no need to de-alloc or store, plot does it

    // A-B markers
    for (int i=0; i<2; i++) {
        _user_markers[i].setLineStyle(QwtPlotMarker::VLine);
        _user_markers[i].setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
        _user_markers[i].setLinePen(QPen(QColor(255, 140, 0), 0, Qt::SolidLine));
        _user_markers_visible[i] = false;
    }
    _user_markers[0].setLabel(QwtText("A"));
    _user_markers[1].setLabel(QwtText("B"));

    // data marker
    _data_marker.setLineStyle(QwtPlotMarker::VLine);
    _data_marker.setLinePen(QPen(QColor(255, 160, 47), 2, Qt::SolidLine));
    _data_marker.setLabel(QwtText("data"));
    _data_marker.setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
    _data_marker_visible = false;
    _data_marker.setZ(9999); // higher value -> paint on top of everything else

    setAutoReplot(true);
}

MavPlot::~MavPlot() {
    // TODO: a lot of cleanup!!!
}

/**
 * @brief function turning any data into double...it may fail.
 */
template <typename CT>
bool MavPlot::_convert2double(CT in, double &ret, double scale) {
    ret = ((double) in)*scale;
    return true;
}

QColor MavPlot::_suggestColor(unsigned int plotnumber) {
    const QList<QColor> cols = QList<QColor>() << QColor(Qt::lightGray) << QColor(Qt::red) << QColor(Qt::green) << QColor(Qt::cyan) << QColor(Qt::yellow) << QColor(Qt::magenta);

    unsigned index = plotnumber % cols.size();
    return cols[index];
}

/**
 * @brief MavPlot::_add_data_event
 * @param generic plotting function for events
 * @return ptr to plotseries if recognized data, else false
 */
template <typename ET>
std::vector<QwtPlotItem*>* MavPlot::_add_data_event(const DataEvent<ET> * data, unsigned int plotnumber /* used for colors etc */) {

    vector <QwtPlotItem*> * markers = new vector <QwtPlotItem*>;
    if (!markers) {
        qDebug() << "Error allocating memory for an event series";
        return NULL;
    }
    const vector<double> vt = data->get_time();
    const vector<ET>     vd = data->get_data();

    // relative time -> absolute time
    double t_datastart = data->get_epoch_datastart()/1E6;

    for (unsigned int k=0; k<data->size(); k++) {
        QwtPlotMarker * d_marker = new QwtPlotMarker();
        if (!d_marker) {
            qDebug() << "Error allocating memory for a marker";
            continue;
        }
        d_marker->setLineStyle(QwtPlotMarker::VLine);
        d_marker->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
        d_marker->setLinePen(QPen(_suggestColor(plotnumber), 0, Qt::DashDotLine));
        //d_marker->setSymbol( new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::yellow), QColor(Qt::green), QSize(7,7)));
        d_marker->setValue(vt[k] + t_datastart, 0.0);
        d_marker->setLabelOrientation(Qt::Vertical);
        // try to set label from data
        stringstream ss;
        ss << vd[k];
        QString label(ss.str().c_str()); // that relies on QString to be cool and do some clever conversion
        d_marker->setLabel(QwtText(label));
        // --
        QwtPlotItem*item = dynamic_cast<QwtPlotItem*>(d_marker);
        if (!item) {
            delete d_marker;
            return NULL;
        }
        markers->push_back(item);
        d_marker->attach(this);
        // add legend entry for first
        if (markers->size()==1) {
            d_marker->setTitle(QString::fromStdString(data->get_name()));
            d_marker->setItemAttribute(QwtPlotItem::Legend);
        }
    }       

    return markers;
}

/**
 * @brief MavPlot::_add_data_timeseries
 * @param generic plotting functin for timeseries
 * @return ptr to plotseries if recognized data, else false
 */
template <typename ST>
QWT_ABSTRACT_SERIESITEM * MavPlot::_add_data_timeseries(const DataTimeseries<ST> * data, unsigned int plotnumber /* used for colors etc */) {

    /*
     * so we have a timeseries of type ST. We need to "translate" it for the plot, and then add it.
     * One problem is, that Qwt cannot plot double against float...so we need to "convert" every-
     * thing to double here.
     */
    QVector<double> xdata, ydata;
    if (!data2xyvect(data, xdata, ydata)) {
        return NULL;
    }

    QwtPlotCurve *curve = new QwtPlotCurve(QString().fromStdString(data->get_name()));
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setPen(QPen(_suggestColor(plotnumber))); // FIXME: offer choices

    curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);    
    curve->setSamples(xdata, ydata); // makes a deep copy
    curve->attach(this);

    return dynamic_cast<QWT_ABSTRACT_SERIESITEM *>(curve);
}

std::vector<QwtPlotItem*>* MavPlot::_branch_datatype_annotation(const Data* data, unsigned int plotnumber /* used for colors etc */) {
    const DataEvent<bool> * tseb = dynamic_cast<DataEvent<bool> const*>(data);
    if (tseb) return _add_data_event<bool>(tseb, plotnumber);

    const DataEvent<string> * tses = dynamic_cast<DataEvent<string> const*>(data);
    if (tses) return _add_data_event<string>(tses, plotnumber);

    // Add more here, if more data types

    return NULL; // unrecognized data type
}

/**
 * @brief MavPlot::_branch_datatype
 * @param data
 * @return ptr to plotseries if recognized data, else false
 */
// TODO: ugly stuff
QWT_ABSTRACT_SERIESITEM * MavPlot::_branch_datatype_series(const Data* data, unsigned int plotnumber /* used for colors etc */) {
    const DataTimeseries<float> *const tsf = dynamic_cast<DataTimeseries<float> const*>(data);
    if (tsf) return _add_data_timeseries<float>(tsf, plotnumber);

    const DataTimeseries<double> * tsd = dynamic_cast<DataTimeseries<double> const*>(data);
    if (tsd) return _add_data_timeseries<double>(tsd, plotnumber);

    const DataTimeseries<unsigned int> * tsu = dynamic_cast<DataTimeseries<unsigned int> const*>(data);
    if (tsu) return _add_data_timeseries<unsigned int>(tsu, plotnumber);

    const DataTimeseries<int> * tsi = dynamic_cast<DataTimeseries<int> const*>(data);
    if (tsi) return _add_data_timeseries<int>(tsi, plotnumber);

    // Add more here, if more data types

    return NULL; // unrecognized data type
}

// TODO: ugly stuff, same as _branch_datatype_series()
bool MavPlot::data2xyvect(const Data * data, QVector<double> & xdata, QVector<double> & ydata, double scale) {
    const DataTimeseries<float> *const tsf = dynamic_cast<DataTimeseries<float> const*>(data);
    if (tsf) return data2xyvect(tsf, xdata, ydata, scale);

    const DataTimeseries<double> * tsd = dynamic_cast<DataTimeseries<double> const*>(data);
    if (tsd) return data2xyvect(tsd, xdata, ydata, scale);

    const DataTimeseries<unsigned int> * tsu = dynamic_cast<DataTimeseries<unsigned int> const*>(data);
    if (tsu) return data2xyvect(tsu, xdata, ydata, scale);

    const DataTimeseries<int> * tsi = dynamic_cast<DataTimeseries<int> const*>(data);
    if (tsi) return data2xyvect(tsi, xdata, ydata, scale);

    return false;
}

template <typename ST>
bool MavPlot::data2xyvect(const DataTimeseries<ST> * data, QVector<double> & xdata, QVector<double> & ydata, double scale) {
    if (!data) return false;

    xdata = QVector<double>::fromStdVector(data->get_time());
    double t_datastart = data->get_epoch_datastart()/1E6;
    for (QVector<double>::iterator it = xdata.begin(); it != xdata.end(); ++it ) {
        // relative time -> absolute time
        *it += t_datastart;
    }

    // data to double
    for (typename vector<ST>::const_iterator it = data->get_data().begin(); it != data->get_data().end(); ++it) {
        double ret;
        if (!_convert2double(*it, ret, scale)) {
            qDebug() << "ERROR: cannot convert given data type to double";
            return false;
        }
        ydata.push_back(ret);
    }
    return true;
}

bool MavPlot::addData(const Data* data) {
    // first check whether we already have it...
    dataplotmap::iterator it = _series.find(data);
    if (it != _series.end()) return true; // have it already

    /* trick comes here. Data can be of different subtypes. First, find the type,
     * then rewrite it for representation
     */
    // try dynamic casts to find type, then branch
    QWT_ABSTRACT_SERIESITEM *retS = _branch_datatype_series(data, _series.size());
    if (retS) {
        // remember the series
        _series.insert(dataplotmap_pair(data, retS));
        _model.model_append(data);
        _updateDataBounds();
        replot();
        return true;
    }

    // was no seriesitem...try if it is an annotation.
    std::vector<QwtPlotItem*>*retA = _branch_datatype_annotation(data, _annotations.size());
    if (retA) {
        // remember the series
        _annotations.insert(annotationsmap_pair(data, retA));
        _model.model_append(data);
        // annotations do not influence data bounds        
        replot();
        return true;
    }

    // now we really failed to recognize the type of the given data
    return false;
}

void MavPlot::unset_markerData() {
    _data_marker.detach();
    _data_marker_visible = false;
    replot();
    updateStatusbar();
}

void MavPlot::rev_markerData(const Data *const d) {
    if (!d || !_data_marker_visible) return;
    const double markerx = _data_marker.xValue();
    double markerx_next = markerx;
#if 0
    /*******************
     * SCAN IN Data
     *******************/
#else
    /*******************
     * SCAN IN Plot
     *******************/
    // FIXME: heavy code clone from fwd_markerData()
    QPointF xy;
    QString value;
    bool found = false;
    // find out what it is, then scan for data
    const QWT_ABSTRACT_SERIESITEM*s = _get_series(d);
    if (s) {
        // it's a series
        const QwtPlotCurve *curve = dynamic_cast<const QwtPlotCurve *>(s);
        if (curve) {
            for (unsigned int k=0; k<curve->dataSize(); k++) {
                xy = curve->sample(k);
                double x = xy.x();
                if (x < markerx) {
                    found = true;
                    markerx_next = x;
                    value = QString::number(xy.y());
                } else if (x > markerx) {
                    break;
                }
            }

        }
    }
    // annotation
    std::vector<QwtPlotItem*>*v = _get_annotations(d);
    if (v && !found) {
        // it's an annotation...we have to go through all of them
        for (std::vector<QwtPlotItem*>::const_iterator it = v->begin(); it != v->end(); ++it) {
            // only support marker annotations
            const QwtPlotMarker * m = dynamic_cast<const QwtPlotMarker*>(*it);
            if (m) {
                // we only handle vertical markers
                if (QwtPlotMarker::VLine == m->lineStyle()) {
                    xy = m->value();
                    double x = xy.x();
                    if (x < markerx) {
                        found = true;
                        markerx_next = x;
                        value = m->label().text();
                    } else if (x > markerx) {
                        break;
                    }
                }
            }
        }
    }
#endif

    // set marker to found location
    if (found) {
        _data_marker_label = value;
        _data_marker.setValue(markerx_next, 0);
        replot();
    }
    updateStatusbar();
}

QWT_ABSTRACT_SERIESITEM *MavPlot::_get_series(const Data * const d) {
    QWT_ABSTRACT_SERIESITEM *ret = NULL;
    if (d) {
        dataplotmap::iterator it_series = _series.find(d);
        if (it_series != _series.end()) {
            // data is series, get ptr
            ret = dynamic_cast<QWT_ABSTRACT_SERIESITEM *>(it_series->second);
        }
    }
    return ret;
}

std::vector<QwtPlotItem*>* MavPlot::_get_annotations(const Data * const d) {
    std::vector<QwtPlotItem*>* ret = NULL;
    if (d) {
        annotationsmap::iterator it_annot = _annotations.find(d);
        if (it_annot != _annotations.end()) {
            // data is annotation
            ret = dynamic_cast<std::vector<QwtPlotItem*>* >(it_annot->second);
        }
    }
    return ret;
}

void MavPlot::setmin_markerData(const Data *const d) {
    if (!d || !_data_marker_visible) return;
    const double markerx = _data_marker.xValue();
    double markerx_next = markerx;
#if 0
    /*******************
     * SCAN IN Data
     *******************/
#else
    /*******************
     * SCAN IN Plot
     *******************/
    // FIXME: code clone from rev_markerData()
    QPointF xy;
    QString value;
    bool found = false;
    // find out what it is, then scan for data
    const QWT_ABSTRACT_SERIESITEM*s = _get_series(d);
    if (s) {
        // it's a series
        const QwtPlotCurve *curve = dynamic_cast<const QwtPlotCurve *>(s);
        if (curve) {
            double minval = 0;
            for (unsigned int k=0; k<curve->dataSize(); k++) {
                QPointF xy = curve->sample(k);
                if (k == 0 || (xy.y() < minval)) {
                    minval = xy.y();
                    markerx_next = xy.x();
                    found = true;
                }
            }
            value = QString::number(minval);
        }
    }
#endif

    // set marker to found location
    if (found) {
        _data_marker_label = value;
        _data_marker.setValue(markerx_next, 0);
        replot();
    } else {
        _data_marker_label = "";
    }
    updateStatusbar();
}

void MavPlot::setmax_markerData(const Data *const d) {
    if (!d || !_data_marker_visible) return;
    const double markerx = _data_marker.xValue();
    double markerx_next = markerx;
#if 0
    /*******************
     * SCAN IN Data
     *******************/
#else
    /*******************
     * SCAN IN Plot
     *******************/
    // FIXME: code clone from rev_markerData()
    QPointF xy;
    QString value;
    bool found = false;
    // find out what it is, then scan for data
    const QWT_ABSTRACT_SERIESITEM*s = _get_series(d);
    if (s) {
        // it's a series
        const QwtPlotCurve *curve = dynamic_cast<const QwtPlotCurve *>(s);
        if (curve) {
            double maxval = 0;
            for (unsigned int k=0; k<curve->dataSize(); k++) {
                QPointF xy = curve->sample(k);
                if (k == 0 || (xy.y() > maxval)) {
                    maxval = xy.y();
                    markerx_next = xy.x();
                    found = true;
                }
            }
            value = QString::number(maxval);
        }
    }
#endif

    // set marker to found location
    if (found) {
        _data_marker_label = value;
        _data_marker.setValue(markerx_next, 0);
        replot();        
    } else {
        _data_marker_label = "";
    }
    updateStatusbar();
}

void MavPlot::fwd_markerData(const Data *const d) {
    if (!d || !_data_marker_visible) return;
    const double markerx = _data_marker.xValue();
    double markerx_next = markerx;
#if 0
    /*******************
     * SCAN IN Data
     *******************/
#else
    /*******************
     * SCAN IN Plot
     *******************/
    // FIXME: heavy code clone from rev_markerData()
    QPointF xy;
    QString value;
    bool found = false;
    // find out what it is, then scan for data
    const QWT_ABSTRACT_SERIESITEM*s = _get_series(d);
    if (s) {
        // it's a series
        const QwtPlotCurve *curve = dynamic_cast<const QwtPlotCurve *>(s);
        if (curve) {
            for (unsigned int k=0; k<curve->dataSize(); k++) {
                xy = curve->sample(k);
                double x = xy.x();
                if (x > markerx) {
                    found = true;
                    markerx_next = x;
                    value = QString::number(xy.y());
                    break;
                }
            }

        }
    }
    // annotation
    std::vector<QwtPlotItem*>*v = _get_annotations(d);
    if (v && !found) {
        // it's an annotation...we have to go through all of them
        for (std::vector<QwtPlotItem*>::const_iterator it = v->begin(); it != v->end(); ++it) {
            // only support marker annotations
            const QwtPlotMarker * m = dynamic_cast<const QwtPlotMarker*>(*it);
            if (m) {
                // we only handle vertical markers
                if (QwtPlotMarker::VLine == m->lineStyle()) {
                    xy = m->value();
                    double x = xy.x();
                    if (x > markerx) {
                        found = true;
                        markerx_next = x;
                        value = m->label().text();
                        break;
                    }
                }
            }
        }
    }
#endif

    // set marker to found location
    if (found) {
        _data_marker_label = value;
        _data_marker.setValue(markerx_next, 0);
        replot();        
    }
    updateStatusbar();
}

bool MavPlot::set_markerData(const Data *const d, unsigned long idx) {
    if (!d ) return false;
    if (idx >= d->size()) return false; // out of range

    bool found = false;
    double markerx = 0;
#if 0
    /*******************
     * SCAN IN Data
     *******************/
#else
    /*******************
     * SCAN IN Plot
     *******************/
    // FIXME: heavy code clone from rev_markerData()
    QPointF xy;
    QString value;
    // find out what it is, then scan for data
    const QWT_ABSTRACT_SERIESITEM*s = _get_series(d);
    if (s) {
        // it's a series
        const QwtPlotCurve *curve = dynamic_cast<const QwtPlotCurve *>(s);
        if (curve) {
            xy = curve->sample(idx);
            markerx = xy.x();
            value = QString::number(xy.y());
            found = true;
        }
    }
    // annotation
    std::vector<QwtPlotItem*>*v = _get_annotations(d);
    if (v && !found) {
        // it's an annotation...we have to go through all of them
        if (v->size() > idx) {
            QwtPlotItem*tmp1 = (*v)[idx];
            const QwtPlotMarker * m = dynamic_cast<const QwtPlotMarker*>(tmp1);
            if (m) {
                // we only handle vertical markers
                if (QwtPlotMarker::VLine == m->lineStyle()) {
                    xy = m->value();
                    markerx = xy.x();
                    found = true;
                    value = m->label().text();
                }
            }
        }
    }
#endif

    // set marker to found location
    if (found) {
        _data_marker_label = value;
        _data_marker.setValue(markerx, 0);
        if (!_data_marker_visible) {
            _data_marker.attach(this);
            _data_marker_visible = true;
        }
        replot();        
    }
    updateStatusbar();
    return found;
}

void MavPlot::set_markerData(double x) {
    _data_marker.setValue(x, 0);
    _data_marker.attach(this);
    _data_marker_visible = true;
    updateStatusbar();
    replot();
}


void MavPlot::unset_markerA() {
    _user_markers[0].detach();
    _user_markers_visible[0] = false;
    updateStatusbar();
    replot();
}

void MavPlot::unset_markerB() {
    _user_markers[1].detach();
    _user_markers_visible[1] = false;
    updateStatusbar();
    replot();
}


void MavPlot::set_markerA(double x) {
    _user_markers[0].setValue(x, 0);
    _user_markers[0].attach(this);
    _user_markers_visible[0] = true;
    updateStatusbar();
    replot();
}

void MavPlot::set_markerB(double x) {
    _user_markers[1].setValue(x, 0);
    _user_markers[1].attach(this);
    _user_markers_visible[1] = true;
    updateStatusbar();
    replot();
}

void MavPlot::updateStatusbar(void) {
    if (!_statusbar) return;
    _statusbar->showMessage(verboseMarkers());
}

QString MavPlot::verboseMarkers() const {
    QString desc;
    if (_user_markers_visible[0]) {
        desc += QString("A: ") + getReadableTime(_user_markers[0].value().x()) + " ";
    }
    if (_user_markers_visible[1]) {
        desc += QString("B: ") + getReadableTime(_user_markers[1].value().x()) + " ";
    }
    if (_user_markers_visible[0] && _user_markers_visible[1]) {
        desc += QString("difference B-A: %1").arg(_user_markers[1].value().x()-_user_markers[0].value().x()) + " ";
    }
    if (_data_marker_visible) {
        desc += QString("data cursor at: ") + getReadableTime(_data_marker.value().x()) + ", value: " + _data_marker_label;
    }
    return desc;
}

void MavPlot::removeAllData() {
    _model.model_clear();

    // remove all series
    for (dataplotmap::iterator it_series = _series.begin(); it_series != _series.end(); ++it_series) {
        const Data*const d = it_series->first;
        _removeData(d);
    }

    // remove all annotations
    for (annotationsmap::iterator it_annot = _annotations.begin(); it_annot != _annotations.end(); ++it_annot) {
        const Data*const d = it_annot->first;
        _removeData(d);
    }

    _updateDataBounds();
    replot();
}
