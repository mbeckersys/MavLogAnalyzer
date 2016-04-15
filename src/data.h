/**
 * @file data.h
 * @brief Abstract data class
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 19.04.2014
 
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

#ifndef DATA_H
#define DATA_H

#include <string>
#include "treeitem.h"
#include "datagroup.h"
#include "debugtype.h"

class Data : public TreeItem
{
public:
    // CTOR
    Data(std::string name) : _valid (false), _name(name), _class(DATA_RAW), _time_epoch_datastart_usec(0), _deferredLoad(false), _dbid(0) {
        _id = _autoincrement++;
        itemtype=DATA;
        parent = NULL;
    }

    // copy CTOR
    Data(const Data & other) {
        _id = _autoincrement++; // ID is always unique
        parent = other.parent;
        itemtype = other.itemtype;
        _valid = other._valid;
        _class = other._class;
        _name = other._name;
        _time_epoch_datastart_usec = other._time_epoch_datastart_usec;
        _units = other._units;
        _deferredLoad = other._deferredLoad;
        _dbid = other._dbid;
    }

    /**
     * @brief general statistics of data
     * not all fields are valid for all data
     */
    typedef struct data_stats_s {
        double t_min;
        double t_max;
        unsigned int n_samples; ///< number of samples between t_min and t_max
        double min;  ///< min. value between t_min and t_max, considering interpolation if t_min or t_max is between samples
        double max;
        double avg;
        double stddev;
    } data_stats;

    /**
     * this enum is used to characterise the data somewhat more.
     * DATA_RAW: data from MAVlink
     * DATA_COMPUTED: data that was produced by using other data, not directly from MAV log
     */
    typedef enum {DATA_RAW=0, DATA_DERIVED, DATA_UNKNOWN} data_classifier_e;

    /****************************************************
     *  FUNCTIONS THAT EACH DATA SUBLASS HAS TO PROVIDE
     ****************************************************/

    /**
     * @brief how many data points do we have?
     * @return
     */
    virtual unsigned int size() const = 0;

    /**
     * @brief each Data subclass must implement this. It is supposed to yield human-readable information about the data.
     * @return descriptive string
     */
    virtual std::string describe_myself() const = 0;

    /**
     * @brief write data as comma-separated values (CSV) into a file
     * @param filename
     * @return true if successful, else false
     */
    virtual bool export_csv(const std::string & filename, const std::string & sep = std::string(",")) const  = 0;

    /**
     * @brief Copy CTOR for polymorphic subclasses
     * @return ptr to the copy. subclasses may return its subtype (covariant return).
     * NULL can be used to indicate, that a derived class does not support cloning.
     */
    virtual Data* Clone() const = 0;

    /**
     * @brief reset the class to initial state; such as if CTOR was processed,
     * but no data yet added.
     */
    virtual void clear() = 0;

    /**
     * @brief add data from another instance to this
     * @param other data that shall be merged in
     * @return true if merging successful, else false.
     * False can also be used to indicate, that a derived class does not support merging.
     */
    virtual bool merge_in(const Data * const other) = 0;

    /**
     * @brief get statistics in a given time window. considering interpolation if t_min or t_max is between samples.
     * @param s holds the statistics.
     * @param tmin begin of time window, in seconds UNIX epoch
     * @param tmax end of time window
     * @return true if found, only then val is valid
     */
    virtual bool get_stats_timewindow(double tmin, double tmax, data_stats & s) const = 0;

    /**
     * @brief get_typename
     * @return a human-readable string of what subclass it is, incl. template type
     */
    virtual std::string get_typename(void) const  = 0;

    /****************************************************
     *  FUNCTIONS THAT EACH DATA SUBCLASS CAN OVERRIDE
     ****************************************************/
    /**
     * @brief get_name
     * @return string that holds the name of the data
     */
    virtual const std::string &get_name() const { return _name; }

    /**
     * @brief is_present
     * @return if true, data is valid and can be used. else empty or invalid.
     */
    virtual bool is_present() const { return _valid; }        

    /**
     * @brief every data should have units...leave empty if it doesn't apply
     * @return string with units
     */
    virtual const std::string &get_units() const {
        return _units;
    }   

    /**
     * @brief here you can set the units
     * @param units
     */
    virtual void set_units(const std::string & units) {
        _units.assign(units);
    }

    /**
     * @brief get_epoch_dataend
     * @return the time reference for the most recent data, expressed in
     * microseconds since 00:00:00 UTC, Thursday, 1 January 1970
     */
    virtual unsigned long get_epoch_dataend() const = 0;

    /****************************************************
     *  FUNCTIONS THAT THIS ABSTRACT CLASS PROVIDES
     ****************************************************/

    /**
     * @brief use this to provide additional information about the data
     * @param t one of enum data_classifier_e
     */
    void set_type(data_classifier_e t) { _class = t; }

    /**
     * @brief call this function to set the offset between the time of data and local time.
     * @param epoch_usec offset in microseconds to 00:00:00 UTC, Thursday, 1 January 1970
     */
    void set_epoch_datastart(unsigned long epoch_usec) {
        if (_time_epoch_datastart_usec == 0) {
            // do not overwrite existing
            _time_epoch_datastart_usec = epoch_usec;
        }
    }

    /**
     * @brief get_epoch_datastart
     * @param epoch_usec
     * @return absolute time reference for time t=0 expressed in number of
     * microseconds since 00:00:00 UTC, Thursday, 1 January 1970
     */
    unsigned long get_epoch_datastart() const {
        return _time_epoch_datastart_usec;
    }       

    /**
     * @brief get_id
     * @return each instance has a unique number. You can get it with this.
     */
    unsigned int get_id() const { return _id; }

    /**
     * @brief set data to be loaded on demand. the parameter gives the ID in the database
     */    
    void set_deferred(unsigned long long dbid) { _deferredLoad = true; _dbid = dbid; }
    void unset_deferred(void) { _deferredLoad = false; }
    bool is_deferred(void) const { return _deferredLoad; }
    unsigned long long get_dbid(void) const { return _dbid; }

    /**
     * @brief get the name of the data, including path
     * @param src
     * @return
     */
    static std::string get_fullname(const Data*const src) {
        // get complete path
        std::string fullname;
        const DataGroup*g = src->parent;
        while (g) {
            fullname = g->groupname + "/" + fullname;
            //fullname.append(g->groupname);
            //fullname.append("/");
            g = g->parent;
        }
        fullname.append(src->get_name());
        return fullname;
    }

    DataGroup*parent;    
    void set_parent(DataGroup*p) {
        parent = p;
    }
    DataGroup* get_parent(void) {return parent;}

protected:
    /***********************************
     *  DATA MEMBERS: must not be anythign w/o copy CTOR
     ***********************************/
    unsigned int        _id; ///< unique ID that every class instance is assigned
    bool                _valid;
    std::string         _name;    
    data_classifier_e   _class;
    static unsigned int _autoincrement; ///< every data class get's a unique ID here.
    unsigned long       _time_epoch_datastart_usec; ///< absolute time when the data starts, expressed in epoch usec
    std::string         _units;

    // database
    bool                _deferredLoad; ///< if true, then the class is empty and not yet polulated... (DB)
    unsigned long long  _dbid; ///< ID in table datagroups

    /***********************************
     *  FUNCTIONS
     ***********************************/
    virtual std::string _verbose_data_class () const {
    switch (_class) {
        case DATA_RAW:
            return "raw";
        case DATA_DERIVED:
            return "derived";
        default:
            return "unknown";
        }
    }
    friend class DBConnector;
};

#endif // DATA_H
