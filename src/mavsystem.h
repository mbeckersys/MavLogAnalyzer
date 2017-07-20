/**
 * @file mavsystem.h
 * @brief Holds data of a mavlog from one scenario.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 18.04.2014
 
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

#ifndef MAVSYSTEM_H
#define MAVSYSTEM_H

#include <iostream>
#include <string>
#include <inttypes.h>
#include <climits>
#include <ostream>
#include <typeinfo>
#include <set>
#include "data_timeseries.h" // FIXME: use data_timed and data_untimed
#include "data_param.h"
#include "data_event.h"
#include "data.h"
#include "datagroup.h"
#include "mavsystem_macros.h"
#include "debugtype.h"
#include "logger.h"

// this enables regular expressions for get_data(), but it also
// instroduces a dependency on Qt.
#ifdef WITH_DATAREGEX
    #include <QRegExp>
#endif

#define MAVTYPE_INIT 0x0
#define MAVAPTYPE_INIT 0x0

class MavSystem
{
private:    
    /*************************
     * FUNCTIONS
     *************************/
    void _defaults();

    /**
     * @brief Make data available: sort a new item into the group hierarchy, create groups if necessary.
     * One item can be registered multiple times, i.e., in multiple groups
     *
     * Call this after you have created a class instance with data. Storing the pointer is also
     * done here, to have an automatic cleanup of data when this class is destroyed.
     *
     * @param fullpath: a string describing the grouping, paths separated with "/".
     * E.g. "main/nav/heading" will create a group "main", create another group "nav" as
     * child and finally register "heading" as data in "nav"
     */
    void _data_register_hierarchy(const std::string &fullpath, Data *item);
    void _data_unregister_hierarchy(Data*const src);

    /**
     * @brief compute synthetic data based on raw data: takeoffs, landings, flight time
     */
    void _postprocess_bad_timing();
    void _postprocess_flightbook();
    void _postprocess_powerstats();
    void _postprocess_glideperf_vel();
    void _postprocess_glideperf_pos();

    void _log(logmsgtype_e t, const std::string & str);

#if 0
    /**
     * @brief shortcut to create new data
     */
    template <typename T>
    inline T*  _new_data(const std::string &fullname, const std::string &units);
    template <typename DT>
    inline DT* _get_and_possibly_create_data(const std::string &fullpath, const std::string & units);
#endif

    /**
     * @brief insert data from somwhere else to this; merges src into this, if the data already exists.
     * @param d data to be added
     * @return true if data was inserted, else false
     */
    bool _add_data(const Data*const src);
    void _del_data(Data*const src);

    void _data_cleanup();   

    /**
     * @brief this internal function will return ptr to a data item.
     * if not exists, it gets created.
     */
    template <typename DT>
    inline DT *_get_and_possibly_create_data(const std::string &fullpath, const std::string & units) {
        // look in map if exists, else register.
        data_accessmap::iterator it = _data_from_path.find(fullpath);
        if (it != _data_from_path.end()) {            
            DT*ret = dynamic_cast< DT *> (it->second);
            if (!ret) {
                std::cerr << "ERROR: type mismatch. Data " << fullpath << " exists with type=" << it->second->get_typename() << ", but asked for a different type" << std::endl;
            }
            return ret;
        }
        return _new_data<DT>(fullpath, units);
    }

    /**
     * @brief this internal function returns a ptr to a data item.
     * @return non-const ptr, where data can be modified
     * fullpath can be a regex, but this is slower
     */
    template <typename DT>
    DT *_get_data(const std::string &fullpath, bool is_regex=false) const {
        // look in map if exists, else register.
        if (!is_regex) {
            data_accessmap::const_iterator it = _data_from_path.find(fullpath);
            if (it != _data_from_path.end()) {
                return dynamic_cast< DT *> (it->second);
            }
        }
#ifdef WITH_DATAREGEX
        else {
            QRegExp rx(QString::fromStdString(fullpath));
            for (data_accessmap::const_iterator it = _data_from_path.begin(); it != _data_from_path.end(); ++it) {
                bool match = rx.indexIn(QString::fromStdString(it->first)) >= 0;
                if (match) {
                    return dynamic_cast< DT *> (it->second);
                }
            }
        }
#endif
        return NULL;
    }



    /**
     * @brief create new data item
     */
    template <typename T>
    inline T* _new_data(const std::string &fullname, const std::string &units) {
        // separate basename
        std::string basename;
        size_t basenamestart = fullname.rfind("/");
        if (std::string::npos == basenamestart) {
            basename = fullname;
        } else {
            basename = fullname.substr(basenamestart+1);
        }

        T*tmp = new T(basename);
        tmp->set_units(units);
        _data_register_hierarchy(fullname, dynamic_cast<Data*>(tmp));
        return tmp;
    }

public:

    /**********************
     * TYPEDEFS
     **********************/

    /**
     * @brief summarizing information about a mavlink connection
     */
    typedef struct mavlink_summary_s {
        unsigned long          num_received; ///< total number of incoming messages = sum (recognized + error + uninterpreted)
        unsigned long          num_interpreted; ///< number of messages that MavLogANalyzer did implement
        unsigned long          num_uninterpreted; ///< number of messages that MavLogAnalyzer doesn't implement, i.e., they were parsed but ignored.
        unsigned long          num_error; ///< number of broken messages (e.g., invalid CRC)        
        std::set<unsigned int> mavlink_msgids_interpreted; ///< set of all evaluated MAVLink message IDs
        std::set<unsigned int> mavlink_msgids_uninterpreted; ///< set of all unknown and thusly ignored MAVLink message IDs
        // more internal stuff:
        unsigned int           _link_throughput_bytes; ///< this collects the number of bytes that were sent between two time references; so it is NOT the actual throughput
    } mavlink_summary_t;

    /**
     * @brief for track_link_throughput
     */
    typedef enum  {
        MAVLINK_INTERPRETED, ///< could be parsed, and was evaluated with track_()
        MAVLINK_UNINTERPRETED, ///< could be parsed, but was NOT processed
        MAVLINK_ERROR ///< problem parsing
    } mavlink_parsed_e;

    /**********************
     * FUNCTIONS
     **********************/
    MavSystem(unsigned int sysid); ///< CTOR
    MavSystem(const MavSystem* other); ///< copy CTOR
    ~MavSystem();
    std::string _mavtype2str(uint8_t mtype);
    std::string _aptype2str(uint8_t atype);
    void track_system(uint8_t stype, uint8_t status, uint8_t autopilot, uint8_t basemode, uint8_t custmode);
    void track_system_errors(uint16_t errors_count [4]);
    void track_statustext(const char*text, uint8_t severity);
    void track_system_sensors(uint32_t present, uint32_t enabled, uint32_t health);
    void track_sysperf(float load, float bat_V, float bat_A);
    void track_ambient(float temp_degC, float press_hPa);
    void track_flightperf(float airspeed_ms, float groundspeed_ms, float alt_MSL_m, float climb_ms, float throttle_percent);
    void track_paths(double lat, double lon, float alt_rel_m, float alt_msl_m, float heading_deg = 65535u);
    void track_paths_attitude(const float roll_pitch_yaw[3], const float speed_roll_pitch_yaw[3]);
    void track_paths_speed(const float v[3]);
    void track_gps_status(uint8_t n_sat, uint8_t fix_type = 255u);
    void track_gps_status(double lat, double lon, float alt_wgs, float hdop, float vdop, float vel_ms, float groundcourse);
    void track_imu1(const int16_t acc_mg[3], const int16_t gyr_mrs[3], const int16_t mag_mT[3]); ///< FIXME: weak interface
    void track_imu2(const int16_t acc_mg[3], const int16_t gyr_mrs[3], const int16_t mag_mT[3]); ///< FIXME: weak interface
    void track_actuators(const uint16_t servo_raw[8]);
    void track_rc(const uint16_t channels[8]);
    void track_mission_current(uint16_t seq);
    void track_mission_item(uint8_t target_system_id, uint8_t target_comp_id, uint16_t seq, uint8_t frame, uint16_t command, uint8_t current, uint8_t autocontinue, float param1, float param2, float param3, float param4, float x, float y, float z);
    void track_power(float Vcc, float Vservo, uint16_t flags);
    void track_radio(uint8_t rssi, uint8_t noise, uint16_t rxerr, uint16_t rxerr_corrected, uint8_t txbuf_percent, uint8_t rem_rssi, uint8_t rem_noise );
    void track_radio(uint8_t rssi);
    void track_radio_droprate(float percent);
    void track_nav(float nav_roll_deg, float nav_pitch_deg, float nav_bear_deg, float tar_bear_deg, float wp_dist_m, float err_alt_m, float err_airspeed_ms, float err_xtrack_m);
    void track_mavlink(unsigned int data_length_bytes, unsigned int msgid, mavlink_parsed_e whatwasdone);
    // FIXME: ugly, but one message can have invalid fields, so we need separate fcns for HIGHRES_IMU:
    void track_imu_highres_acc(const float acc_ms[]);
    void track_imu_highres_gyr(const float gyr_rs[]);
    void track_imu_highres_mag(const float mag_gauss[]);
    void track_imu_highres_temp(float temp_degC);
    void track_imu_highres_pressabs(const float press_mbar);
    void track_imu_highres_pressalt(float alt_m);
    void track_imu_highres_pressdiff(float press_mbar);

    std::string get_mavtype(void) const {
        return mavtype_str;
    }

    std::string get_aptype(void) const {
        return aptype_str;
    }

    unsigned int get_id(void) const {
        return id;
    }

    /**
     * @brief this function will directly enqueue the data under the given path
     * @param fullname
     * @param data
     * @return ptr to data series (ifneeded)
     */
    template <typename T1>
    DataTimeseries<T1>* track_generic_timeseries(const std::string & fullname, const T1 & arg_data, const std::string &units = "") {
        MAVSYSTEM_DATA_ITEM(DataTimeseries<T1>, data, fullname, units);
        if (!data) return NULL;
        data->add_elem(arg_data, _time);
        return data;
    }

    template <typename T2>
    DataParam<T2>* track_generic_untimed(const std::string & fullname, const T2 & arg_data, const std::string &units = "") {
        MAVSYSTEM_DATA_ITEM(DataParam<T2>, data, fullname, units);
        if (!data) return NULL;
        data->add_elem(arg_data, _time);
        return data;
    }

    template <typename T3>
    DataEvent<T3>* track_generic_event(const std::string & fullname, const T3 & arg_data, const std::string &units = "") {
        MAVSYSTEM_DATA_ITEM(DataEvent<T3>, data, fullname, units);
        if (!data) return NULL;
        data->add_elem(arg_data, _time);
        return data;
    }

    /**
     * @brief same as _get_data(), but for external use, where
     * the returned pointer is const.
     */
    template <typename DT>
    const DT *get_data(const char *fullpath, bool is_regex=false) const {
        return get_data< DT >(std::string(fullpath), is_regex);
    }

    /**
     * @brief same as get_data(), but with string argument
     * (overloaded for convenience)
     */
    template <typename DT>
    const DT *get_data(const std::string &fullpath, bool is_regex = false) const {
        return const_cast<const DT *>(_get_data < DT > (fullpath, is_regex));
    }

#if 0
    /**
     * @brief this function will return reference to a data item.
     * which lies in the given hierarchy.
     * @return ptr to data or NULL if none found
     */
    template <typename DT>
    const DT* get_data(const std::string &fullpath) const;
    template <typename DT>
    const DT *get_data(const char* fullpath) const; // convenience overloading
#endif

    /**
     * @brief postprocess creates additional data items, that are computed based on the existing ones.
     */
    void postprocess();    

    /**
     * @brief call this to indicate current relative time. all subsequent calls to track_*() will assume this time.
     * @param nowtime_relative_usec current time since system boot (or whatever) in microsecs
     * @param epoch_usec if available,
     * @param if true, we allow for huge forward-jumps in relative time.
     * @return 0 if update was accepted, otherwise error code: -1 if backward jump was rejected, +1 if forward jump was rejected.
     */
    int update_rel_time(uint64_t nowtime_relative_usec, bool allowjumps=false);

    uint64_t get_rel_time() const {
        return ((uint64_t)(_time * 1E6));
    }

    /**
     * @brief call this before you track any new message.
     */
    void nextmsg(void) {
        _have_time_update = false;
    }

    /**
     * @brief use this to help correlate relative time since boot with absolute time.
     * invokes update_rel_time() with the first parameter internally
     * @param nowtime_relative_usec current time since system boot (or whatever) in microsecs
     * @param epoch_usec epoch (local time) in usec
     * @param if true, we allow for huge forward-jumps in relative time.
     */
    void update_time_offset(uint64_t nowtime_relative_usec, uint64_t epoch_usec, bool allowjumps=false);

    /**
     * @brief guess whether timestamp is relative time or absolute_time
     * @param timestamp
     * @return
     */
    bool is_absolute_time(uint64_t timestamp_usec) const ;

    /**
     * @brief You can use this instead of update_time_offset() to handover a guess for the time.
     * If update_time_offset() is called later, the guess is irrelevant, otherwise it takes effect.
     */
    void update_time_offset_guess(uint64_t nowtime_relative_usec, uint64_t epoch_usec);

    /**
     * @brief goes through all the data in this system and decides on the time offset between relative time and unix time,
     * based on the values from update_time_offset().
     */
    void determine_absolute_time();

    /**
     * @brief artificially add an offset to the timeline
     * @param delay. negative values = make earlier
     */
    void shift_time(double delay);

    /**
     * @brief if two successive messages exhibit large differences in their time stamps, they get ignored.
     *        This function can be used to set the margin for the time difference.
     *        To take effect, function must be called *before* any data is processed
     * @param seconds the margin
     */
    void set_max_timejumps_fwd(double seconds) {
        if (seconds > 0.) _time_maxfwdjump_sec = seconds;
    }
    void set_max_timejumps_back(double seconds) {
        if (seconds > 0.) _time_maxbackjump_sec = seconds;
    }

    double get_max_timejumps_fwd(void) {
        return _time_maxfwdjump_sec;
    }
    double get_max_timejumps_back(void) {
        return _time_maxbackjump_sec;
    }

    /**
     * @brief returns a textual summary of the system
     * @param buf where to write the text
     */
    void get_summary(std::string &buf) const;

    /**     
     * @return absolute time of system boot/shutdown as unix epoch given in double with decimals
     */
    double get_time_active_begin(void) const;
    double get_time_active_end(void) const;
    /**
     * @return absolute time of system boot/shutdown as unix epoch given in microseconds
     */
    unsigned long get_time_active_begin_usec(void) const;
    unsigned long get_time_active_end_usec(void) const;



    /**
     * @brief information about MAVLink packets.
     */
    void get_mavlink_summary(mavlink_summary_t & ret) const;

    /**
     * @brief add data to this mav system from somewhere else. Timestamps etc are synchronized.
     * @param other data from somewhere else
     * @return true if successful, else false
     */
    bool merge_in(const MavSystem *const other);

    /**
     * @brief return information about mavlink and what was ignored by MavLogAnalyzer
     * @param ret reference to variable where information is returned
     */
    void get_mavlink_stats(mavlink_summary_t & ret) const;

    /********************************************
     *    DATA MEMBERS
     ********************************************/       
    // general MAV info
    unsigned int id;            ///< system id
    unsigned int mavtype;       ///< enum MAV_TYPE
    std::string mavtype_str;    ///< enum MAV_TYPE as human-readable string
    unsigned int aptype;        ///< enum MAV_AUTOPILOT
    std::string aptype_str;     ///< enum MAV_AUTOPILOT as human-readable string
    bool has_been_armed;        ///< if true, then at some point the system was armed

    // database
    bool deferredLoad;          ///< if true, the data in here is not complete and needs to be loaded when requested

private:
    /********************************************
     *    DATA MEMBERS
     ********************************************/
    // we need this however: fullpath-to-Data mapping
    typedef std::map<std::string, Data*> data_accessmap;
    data_accessmap _data_from_path; ///< all data is stored flat in here, but accessing it should be done via mav_data_groups, where we have associative arrays


    double _time; ///< this is a relative time...later we need to call update_time_offset() and apply_time_offset() to establish a binding to absolute time
    bool   _time_valid; ///< indicates whether _time carries actual data, or if it is uninitialized
    double _time_min;
    double _time_max;
    double _time_maxfwdjump_sec; ///< how much seconds may pass between two data points to be regarded as connected
    double _time_maxbackjump_sec; ///< same but other direction...must be positive
    bool   _have_time_update;

    // more data (time series, ...)
    DataGroup::groupmap mav_data_groups;  ///< hierarchy for data...you can browse through data with associative array (map)

    // for managing the time
    typedef std::pair<uint64_t, uint64_t> timeoffset_pair;
    std::vector<timeoffset_pair > _time_offset_raw; ///< pair <relative time, epoch>. Used to find the relation between relative time (all data in this class) and unix time
    uint64_t _time_offset_usec; ///< add this to relative times in data, and you get unix time
    uint64_t _time_offset_guess_usec;

    Logger::logchannel _logchannel; ///< all couts go to there

    mavlink_summary_t _mavlink_summary; ///< holds information about all mavlink packets emitted by this system

    // DB stuff
    unsigned long long _dbid;
    friend class DataTreeViewModel;
    friend class DBConnector;
};

#endif // MAVSYSTEM_H
