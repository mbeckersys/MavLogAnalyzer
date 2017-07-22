/**
 * @file mavlinkscenario.h
 * @brief Processes already parseed mavlink messages and stores the data internally in a more abstract and usable fashion.
 *        This class should only be used with one single logfile to form what is called a "scenario".
 *        If you want to merge multiple logfiles into one scenario, use the merge_in() function.
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

#ifndef MAVLINKSCENARIO_H
#define MAVLINKSCENARIO_H

#include <map>
#include <inttypes.h>
#include <fstream>
#include <ostream>
#include "mavlink.h"
#include "mavsystem.h"
#include "cmdlineargs.h"
#include "onboarddata.h"
#include "logger.h"

class MavlinkScenario
{
public:
    MavlinkScenario(const CmdlineArgs * const args);
    ~MavlinkScenario();

    /**
     * @brief Feed MavLink messages into here. analyzes the handed-over message and stores the data internally
     * @param msg
     * @param allowJumps if true, time jumps are tolerated
     * @return 0 if message was handeled (ignored is also handled), else error code: -1 there was a backward time jump, message ignored, +1: forward time jump
     */
    int add_mavlink_message(const mavlink_message_t &msg, bool allowJumps=false);

    /**
     * @brief Feed onboard messages into here. analyzes the handed-over message and stores
     *        the data internally.
     * XXX: do *not* multiplex logs of difference vehicles. Feed one after the other, otherwise
     *      data is messed up!
     * @param msg
     * @return true if message was handeled, else false
     */
    bool add_onboard_message(const OnboardData &msg);

    /**
     * XXX! do not use add_mavlink_message and add_mavlink_message in the same scenario. Rather use
     * two distinct scenarios and merge them using merge_in().
     */

    /**
     * @brief without arguments, writes to logfile, with argument, writes fo the given stream
     */
    void dump_overview(void);
    void dump_overview(std::ostream& ofs) const;

    /**
     * @brief process does some data crunching, trying to enhance data and generate
     * computed data on top.
     */
    void process(bool calculate_time_offset = true);

    /**
     * @brief get pointer to mavlink system with systemid=id
     * @param id
     * @return NULL on error, else ptr
     */
    const MavSystem*get_system_byid(uint8_t id) const;

    /**
     * @brief merges data from another class instance into this one
     *        Makes a deep copy of all data contained in other
     * @param other the other instance, whose data is added to this instance
     * @return true on success, else false (in this case, this instance's data might be inconsistent)
     */
    bool merge_in(const MavlinkScenario &other);

    /**
     * @brief shift all data in scenario by specifed amount of seconds. negative will make the data earlier.
     * @param delay
     */
    void shift_time(double delay_sec);

    /**
     * @brief use this to make a guess about the start time of the log. If timing
     * data is found in the logfile, this os overridden, otherwise it takes effect.
     * @param time_epoch_usec
     */
    void set_starttime_guess(uint64_t time_epoch_usec);

    /**
     * @brief considering all seen systems, tell me when scenario started
     * (that is more precise than _time_guess_epoch_usec, because the systems each provide more data)
     * @return  UNIX epoch in microseconds
     */
    unsigned long get_scenario_starttime_usec(void) const;
    double get_scenario_starttime_sec(void) const;

    /**
     * @brief getSystems
     * @return
     */
    std::vector<const MavSystem*> getSystems() const;

    /**
     * @brief getName
     * @return name of the scenario.
     */
    std::string getName(void) const { return _name; }
    void setName(const std::string & nam) { _name = nam; }

    std::string getDescription(void) const { return _desc; }
    void setDescription(const std::string & desc) { _desc = desc; }

    void setDatabaseID(unsigned long long id) { _dbid = id; _havedb = true;}
    bool getDatabaseID(unsigned long long& id) { id = _dbid; return _havedb; }

    // logging
    Logger::logchannel*getLogChannel(void) { return &_logchannel; }
    void log(logmsgtype_e t, const std::string &ss);

private:

    MavSystem* _get_or_add_system_byid(uint8_t id);

    /********************************************
     *  DATA MEMBERS
     ********************************************/
    const CmdlineArgs*_args;

    unsigned int _n_msgs; ///< how many messages we processed in this scenarios; this is for ALL systems. Per-system stats are within MavSystem
    unsigned int _n_ignored; ///< how many messages we did not process (no decoding was programmed)

    // scenario data
    typedef std::map <uint8_t, MavSystem*> systemlist;
    systemlist _seen_systems; ///< list of all seen systems (MAVs, GCSs...)
    uint64_t _time_guess_epoch_usec; ///< guess when the scenario started

    friend class SystemTableViewModel; // FIXME: this avoids that we can compile w/o GUI
    friend class DataTreeViewModel; // FIXME: this avoids that we can compile w/o GUI
	friend class DBConnector;

    // #### temps for onboardparser
    int _onboard_sysid;
    // for building a time basis
    struct  {
        bool have_last;
        // APM:
        uint32_t last_gps_week_ms; ///< previous gps time we have seen. will be used to construct relative time
        uint64_t last_nowtime_usec; ///< rel. time when we have seen it
        // PX4:
        uint64_t last_gps_time;
    } _onboard_gps_time;
    struct  {
        bool have_last;
        // PX4:
        uint64_t last_time;
        uint64_t last_nowtime_usec; ///< rel. time when we have seen it
    } _onboard_time_time;

    Logger::logchannel _logchannel;
    std::string _name; ///< descriptive name of the scenario
    std::string _desc; ///< comments on the scenario
    std::string _last_onboard_parser;

    // database
    bool _havedb;
    unsigned long long _dbid;

};

#endif // MAVLINKSCENARIO_H
