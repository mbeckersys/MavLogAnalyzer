/**
 * @file data_untimed.h
 * @brief Data class for untimed non-list, only one, data point. E.g., total flight time
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 26.04.2014
 
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

#ifndef DATA_PARAM_H
#define DATA_PARAM_H

#include "config.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include "data_untimed.h"

/**
 * @brief class for parameter data
 * parameters are untimed. They are valid for any point in time.
 */
template <typename T>
class DataParam : public DataUntimed {
private:
    /********************************************
     *  MEMBER VARIABLES
     ********************************************/
    T _elem; ///< the data

public:
    DataParam(std::string name) : DataUntimed(name) { /* FIXME: cannot initialize data, because of templating */ }

    // copy CTOR (DONE)
    DataParam(const DataParam & other) : DataUntimed(other) {
        _elem = other._elem;
    }

    // implements Data::clear()
    void clear() {
        _valid = false;
        // cannot reset _elem because of templating
        _time_epoch_datastart_usec = 0;
    }    

    // implements Data::size()
    unsigned int size(void) const {
        return _valid ? 1 : 0;
    }

    // implements Data::get_typename()
    std::string get_typename(void) const {
        std::string str = "data_untimed:";
        str += DEBUGTYPE(_elem);
        str += ">";
        return str;
    }

    void add_elem(T const &dataelem) {
        _elem = dataelem;
        _valid = true;
    }

    std::string describe_myself() const {
        std::stringstream ss;
        ss<< "Data: UNTIMED SINGLE" << std::endl <<             
             "type: " << get_typename() << ", " << _verbose_data_class() << std::endl <<
             "name: " << Data::get_fullname(dynamic_cast<const Data*const>(this)) << std::endl <<
             "data: " << _elem << std::endl <<
             "units: " << get_units() << std::endl;
        return ss.str();
    }

    bool get_stats_timewindow(double /*tmin*/, double /*tmax*/, data_stats & /*s*/) const {
        return false; // FIXME: stats for params
    }

    T get_value() const {
        return _elem;
    }

    unsigned long get_epoch_dataend() const {
        return get_epoch_datastart(); // because it is untimed
    }

    // implements Data::export_csv()
    bool export_csv(const std::string & filename, const std::string & /*sep*/ = std::string(",")) const {
        std::ofstream fout(filename.c_str());

        if (!fout.is_open()) return false;

        fout << "# untimed single data point \"" << _name << "\" [" << _units << "]" << std::endl;
        fout << std::setprecision(9);
        fout << _elem << std::endl;
        fout.close();
        return true;
    }


    /**
     * @brief implements Data::Clone()
     * @return covariant of Data*
     */    
    DataParam* Clone() const {
        return new DataParam(*this); ///< call copy ctor
    }

    // implements Data::merge_in()
    virtual bool merge_in(const Data * const /*other*/) {
        /**
         * This class does not support merging. It could be minimum, maximum, sum, average ,...
         * if you need this functionality, then derive this class and override it
         */
        return false;
    }
};

#endif // DATA_PARAM_H
