/**
 * @file data_event.h
 * @brief Event data type. An event can be absent or present. It is assumed
 * absent by default, and we only record present state here. Additionally,
 * each instant can have data associated, e.g., a descriptive string.
 *
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

#ifndef DATA_EVENT_H
#define DATA_EVENT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "data.h"

template <typename T>
class DataEvent : public Data {
private:
    /********************************************
     *  MEMBER VARIABLES
     ********************************************/
    unsigned int _n; ///< number of events stored
    std::vector<T>      _elems_data;    ///< only used if keepitems=true
    std::vector<double> _elems_time;    ///< only used if keepitems=true

    T _dummy_item; ///< needed when empty

public:    
    DataEvent(std::string name) : Data(name), _n(0) {  }

    // copy CTOR (DONE)
    DataEvent(const DataEvent & other) : Data(other) {
        _n = other._n;
        _elems_data = other._elems_data; // deep copy by STL
        _elems_time = other._elems_time;
    }

    // implements Data::clear()
    void clear() {
        _n = 0;
        _valid = false;
        _elems_data.clear();
        _elems_time.clear();
        _time_epoch_datastart_usec = 0;
    }

    // implements Data::get_typename()
    std::string get_typename(void) const {
        std::string str = "data_event:";
        str += DEBUGTYPE(_dummy_item);
        str += ">";
        return str;
    }

    void add_elem(T const &dataelem, double datatime = NAN) {
        _elems_data.push_back(dataelem);
        _elems_time.push_back(datatime);
        _n++;
        _valid = true;
    }

    bool get_stats_timewindow(double /*tmin*/, double /*tmax*/, data_stats & /*s*/) const {
        return false; // TODO
    }

    const std::vector<double>& get_time() const {
        return _elems_time;
    }

    const std::vector<T>& get_data() const {
        return _elems_data;
    }

    const T& get_latest() const {
        if (_n == 0) return _dummy_item;
        return _elems_data.back();
    }

    unsigned long get_epoch_dataend() const {
        if (_elems_time.empty()) { return get_epoch_datastart(); }
        return ((unsigned long) _elems_time.back()*1E6) + get_epoch_datastart();
    }

    std::string describe_myself() const {
        std::stringstream ss;
        ss<< "Data: EVENT" << std::endl <<
             "type: " << get_typename() << ", " << _verbose_data_class() << std::endl <<
             "name: " << Data::get_fullname(dynamic_cast<const Data*const>(this)) << std::endl <<
             "#data points: " << _n << std::endl;

        return ss.str();
    }

    /**
     * @brief how many elements are stored?
     * @return
     */
    unsigned int size() const {
        return _n;
    }

    // implements Data::export_csv()
    bool export_csv(const std::string & filename, const std::string & sep = std::string(",")) const {
        std::ofstream fout(filename.c_str());

        if (!fout.is_open()) return false;

        fout << "#time, " << _name << "[" << _units << "]" << std::endl;
        for (unsigned int k=0; k<_n; k++) {
            T d = _elems_data[k];
            double t = _elems_time[k];
            // write
            fout << t << sep << d << std::endl;
        }

        fout.close();
        return true;
    }

    /**
     * @brief implements Data::Clone()
     * @return covariant of Data*
     */
    DataEvent* Clone() const {
        return new DataEvent(*this);
    }

    // implements Data::merge_in() (DONE)
    bool merge_in(const Data * const other) {
        const DataEvent*const src = dynamic_cast<const DataEvent*const>(other);
        if (!src) return false;
        if (!src->_valid) return false;
        //if (_elems_time.empty()) return false;

        const double tmin_src = src->_elems_time.front() + src->_time_epoch_datastart_usec/1E6;
        const double tmax_src = src->_elems_time.back() + src->_time_epoch_datastart_usec/1E6;
        const double tmin_me = _elems_time.front() + _time_epoch_datastart_usec/1E6;
        const double tmax_me = _elems_time.back() + _time_epoch_datastart_usec/1E6;
        const double dt_sec = (_time_epoch_datastart_usec/1E6 - src->_time_epoch_datastart_usec / 1E6); ///< positive, if my data is more recent

        /*
         * we want no negative time stamps, so adjust all *my* relative times by
         * applying the offset between src and me to *me*
         */
        if (dt_sec > 0.) {
            _time_epoch_datastart_usec = src->_time_epoch_datastart_usec;
            for (std::vector<double>::iterator it = _elems_time.begin(); it != _elems_time.end(); ++it) {
                *it += dt_sec;  ///< correct my relative times
            }
        }

        /*****************
         *  MERGING IN
         *****************/
        const bool do_fast_merge = (tmax_src < tmin_me) || (tmin_src > tmax_me); ///< checks for non-overlapping time ranges
        if (do_fast_merge) {
            if (dt_sec > 0.) {
                // PREPEND: my data is later (other earlier). we want no negative time stamps, so adjust all my relative times by adding apply the offset from src
                _elems_time.insert(_elems_time.begin(), src->_elems_time.begin(), src->_elems_time.end()); ///< prepend time
                _elems_data.insert(_elems_data.begin(), src->_elems_data.begin(), src->_elems_data.end()); ///< prepend data
            } else {
                // APPEND: my data is older (other more recent). adjust other data's time relative time stamps by adding the offset to it
                for (std::vector<double>::const_iterator it = src->_elems_time.begin(); it != src->_elems_time.end(); ++it) {
                    _elems_time.push_back(*it - dt_sec); ///< correct other's time stamp and append at the same time
                }
                _elems_data.insert(_elems_data.end(), src->_elems_data.begin(), src->_elems_data.end()); ///< append data
            }
        } else {                        
            // INSERT: data is overlapping...we have to sort-in every single data item
            for (unsigned int k=0; k<src->_elems_time.size(); ++k) {
                double tsrc = src->_elems_time[k];
                if (dt_sec < 0.) {
                    // adjust source's time
                    tsrc = src->_elems_time[k] - dt_sec;
                }
                T datasrc = src->_elems_data[k];
                std::vector<double>::iterator first_greater_time = std::upper_bound(_elems_time.begin(), _elems_time.end(), tsrc); // returns iterator to first element t1 where t1 > t holds true
                if (first_greater_time != _elems_time.end()) {
                    // insert before first_greater
                    typename std::vector<T>::iterator first_greater_data = _elems_data.begin() + (first_greater_time - _elems_time.begin()); // calc index from iterator
                    _elems_time.insert(first_greater_time, tsrc);
                    _elems_data.insert(first_greater_data, datasrc);
                } else {
                    // there was none greater...append
                    _elems_time.push_back(tsrc);
                    _elems_data.push_back(datasrc);
                }
            }
        }
        _n+= src->_elems_data.size();
        return true;
    }
};

#endif // DATA_EVENT_H
