/**
 * @file mavlnkscenario.cpp
 * @brief Processes already parseed mavlink messages and stores the data internally.
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
    along with this program. If not, see <http://www.gnu.org/licenses/>.
    
 */

#include <string>
#include <iostream>
#include <inttypes.h>
#include <stdio.h>
#include "mavlinkscenario.h"
#include "logger.h"

using namespace std;

MavlinkScenario::MavlinkScenario(const CmdlineArgs *const args) : _args(args), _n_msgs(0), _n_ignored(0),
    _time_guess_epoch_usec(0), _onboard_sysid(-1), _havedb(false), _dbid(0) {

    _onboard_gps_time.have_last = false;

    _logchannel = Logger::Instance().createChannel("scenario");
}

MavlinkScenario::~MavlinkScenario() {    
    for (systemlist::iterator it = _seen_systems.begin(); it != _seen_systems.end(); ++it) {
        MavSystem*s = it->second;
        if (s) delete s;
    }
    _seen_systems.clear();
    Logger::Instance().deleteChannel(_logchannel);
}

/**
 * @brief returns a pointer to mavsystem with given ID. if not existing, it is created.
 * @param id
 * @return pointer or NULL on error.
 */
MavSystem* MavlinkScenario::_get_or_add_system_byid(uint8_t id) {
    systemlist::iterator it = _seen_systems.find(id);
    if (it == _seen_systems.end()) {
        // new one, add it
        it = _seen_systems.insert(_seen_systems.begin(), std::pair<uint8_t,MavSystem*>(id,new MavSystem(id))); // add new
        // hand over options from cmd line
        if (_args) {
            it->second->set_max_timejumps_fwd(_args->time_maxjump_sec);
            it->second->set_max_timejumps_back(_args->time_maxjump_sec); // FIXME: spend separate argument
        }
    }
    MavSystem*sys = it->second;
    return sys;
}

void MavlinkScenario::log(logmsgtype_e t, const std::string & str) {
    Logger::Instance().write(t, str, _logchannel);
}

// FIXME: refactor similar to add_mavlink_message (int return) and make polymorphic
bool MavlinkScenario::add_onboard_message(const OnboardData &msg) {
    if (!msg.is_valid()) return true;

    // find MAV system ID and cache it
    if (msg.get_message_origname().compare("PARM")==0) {
        const OnboardData::floatdata d = msg.get_floatdata();
        OnboardData::floatdata::const_iterator ir = d.find("SYSID_THISMAV");
        if (ir != d.end()) {
            _onboard_sysid = ((int) ir->second);                        
            log(MSG_INFO, stringbuilder() << "Onboard Log: Sysid=" << _onboard_sysid);
        }
    }
    if (_onboard_sysid < 0) {
        _onboard_sysid = 1;
        log(MSG_WARN, stringbuilder() << "No system id known...assuming value: " << _onboard_sysid);
        //return false; // don't know yet for which system ID this is
    }

    /****************************
     *  ADD/FIND SYSTEM
     ****************************/
    MavSystem* const sys = _get_or_add_system_byid(((uint8_t)_onboard_sysid));
    if (!sys) return false;

    /*
     * Guess the current time. Only the following messages carry timing:
     *  - GPS: TimeMS (ms since week start), week (week number), T=APM time (hal.scheduler.millis)
     *  - IMU: TimeMS (time_week_ms)
     *  - CAM: GPSTime
     */

    // to preserve order when no time is given.
    uint64_t nowtime_us = sys->get_rel_time() + 1;
    sys->update_rel_time(nowtime_us); // if someone knows better below, it can update just again    

    /****************************
     *  ABSOLUTE TIMESTAMPS
     ****************************/
    if (msg.get_message_origname().compare("GPS")==0) {
        const OnboardData::uintdata d = msg.get_uintdata();                
        /**
         * Get GPS time and use it as time reference. Unfortunately
         * gps_week_ms can have huge jumps. Namely, this happens when the GPS gots from no fix to fix.
         * Therefore, we have to ignore everything that has no fix.
         * FIXME: don't igore rel. time where we are waiting for a fix, but back-date it.
         */

        bool valid = true;
        uint16_t gps_week;
        uint32_t gps_week_ms;

        // difference between APM and PX4
        bool is_apm = false;
        bool is_px4 = false;
        OnboardData::uintdata::const_iterator ir = d.find("Status");
        if (ir != d.end()) {
            is_apm = true;
        } else {
            ir = d.find("Fix");
            if (ir != d.end()) {
                is_px4 = true;
            }
        }

        valid = valid && (is_px4 || is_apm);
        if (valid) {
            bool has_gps_fix;
            if (is_apm) {
                unsigned int gpsfix = ir->second;
                const unsigned int GPS_HAS_FIX_CONSTANT = 2; // see APM:Copter GPS status codes
                has_gps_fix = (gpsfix >= GPS_HAS_FIX_CONSTANT);
            }
            if (is_px4) {
                unsigned int gpsfix = ir->second;
                const unsigned int GPS_HAS_FIX_CONSTANT = 2;
                has_gps_fix = (gpsfix >= GPS_HAS_FIX_CONSTANT);
            }
            valid = valid && has_gps_fix;
        }

        if (is_apm) {
            /**************************
             * APM time reference
             **************************/
            ir = d.find("TimeMS");
            valid = valid && (ir != d.end());
            if (valid) {
                gps_week_ms = ir->second;
            }
            ir = d.find("Week");
            valid = valid && (ir != d.end());
            if (valid) {
                gps_week = ir->second;
            }
            if (valid) {
                if (_onboard_gps_time.have_last) {
                    // update rel. time based on the time lapsed since last GPS message
                    // assumption: GPS week does not wrap during flight
                    uint64_t timepassed_usec = (gps_week_ms - _onboard_gps_time.last_gps_week_ms)*1000UL;
                    uint64_t cand_nowtime_us = _onboard_gps_time.last_nowtime_usec + timepassed_usec;// - _onboard_gps_time.initial_gps_week_us;
                    nowtime_us = cand_nowtime_us;
                    uint64_t time_unix_usec = gpsepoch2unixepoch_usec(gps_week, gps_week_ms);
                    sys->update_time_offset(nowtime_us, time_unix_usec, /*allowjumps=*/true); // OK -- the only place where we accept epoch/absolute time
                    //cout << "onboardparser: rel.time=" << nowtime_us << "us, unix=" << time_unix_usec/1E6 << "s, passed = " << timepassed_usec/1000 << "ms" << endl;

                }
                _onboard_gps_time.last_nowtime_usec = nowtime_us;
                _onboard_gps_time.last_gps_week_ms = gps_week_ms;
                _onboard_gps_time.have_last = true;
            } else {
                // FIXME: no guess what the time is...increment by one to keep ordering of messages
                log(MSG_WARN, stringbuilder() << "heavy guess mode at t_rel=" << nowtime_us/1000 << " ... no time because no GPS fix");
                _onboard_gps_time.last_nowtime_usec += 100E3; // 100 msec between two positions?
                sys->update_rel_time(_onboard_gps_time.last_nowtime_usec,false);
            }
        }
        else if (is_px4) {
            /**************************
             * PX4 time reference
             **************************/
            ir = d.find("GPSTime");
            valid = valid && (ir != d.end());

            if (valid) {
                uint64_t gps_time_usec = ir->second; // microseconds UTC time
                uint64_t reltime_us = 0;
                if (_onboard_gps_time.have_last) {                    
                    // update rel. time based on the time lapsed since last GPS message                    
                    uint64_t timepassed_usec = (gps_time_usec - _onboard_gps_time.last_gps_time);
                    reltime_us = _onboard_gps_time.last_nowtime_usec + timepassed_usec;// - _onboard_gps_time.initial_gps_week_us;

                    // inx PX4 absolute time and offset are the same, because this message has UTC
                    uint64_t time_unix_usec = gps_time_usec;

                    sys->update_time_offset(reltime_us, time_unix_usec, /*allowjumps=*/true); // OK -- the only place where we accept epoch/absolute time
                } else {
                    log(MSG_INFO, stringbuilder() << "Initial time from GPS: " << gps_time_usec/1E6  << " s");
                }
                _onboard_gps_time.last_nowtime_usec = reltime_us;
                _onboard_gps_time.last_gps_time = gps_time_usec;
                _onboard_gps_time.have_last = true;
            } else {
                // FIXME: no guess what the time is...increment by one to keep ordering of messages
                uint64_t tnow = sys->get_rel_time();
                tnow +=500; // 500usec
                sys->update_rel_time(tnow,false);
            }
        }
    } else if (msg.get_message_origname().compare("TIME")==0) {
        // PX4: uint64_t hrt_absolute_time
        const OnboardData::uintdata d = msg.get_uintdata();
        OnboardData::uintdata::const_iterator ir = d.find("StartTime");
        bool valid = ir != d.end();
        if (valid) {
            uint64_t gps_time_usec = ir->second; // microseconds UTC time
            uint64_t reltime_us = 0;
            if (_onboard_time_time.have_last) {
                // update rel. time based on the time lapsed since last GPS message
                uint64_t timepassed_usec = (gps_time_usec - _onboard_time_time.last_time);
                reltime_us = _onboard_time_time.last_nowtime_usec + timepassed_usec;// - _onboard_gps_time.initial_gps_week_us;

                //sys->update_time_offset(reltime_us, time_unix_usec, /*allowjumps=*/true); // OK -- the only place where we accept epoch/absolute time
                //sys->update_rel_time(reltime_us,false);
            } else {
                log(MSG_INFO, stringbuilder() << "Initial time: " << gps_time_usec/1E6  << " s");
            }
            _onboard_time_time.last_nowtime_usec = reltime_us;
            _onboard_time_time.last_time = gps_time_usec;
            _onboard_time_time.have_last = true;
        }      
    } else {
        //cout << "generic onboard message: " <<  msg.get_message_origname() << endl;
        const OnboardData::uintdata d = msg.get_uintdata();
        OnboardData::uintdata::const_iterator ir = d.find("TimeUS");
        if (ir == d.end()) ir = d.find("t");
        if (ir != d.end()) {
            uint64_t tnow = ir->second;
            if (sys->is_absolute_time(tnow)) {
                sys->update_time_offset(sys->get_rel_time(), tnow);
            } else {
                sys->update_rel_time(tnow, false);
            }
        } else {
            uint64_t tnow = sys->get_rel_time();
            tnow +=50; // 50usec to keep order
            sys->update_rel_time(tnow,false);
        }

        /*
         * Unlike APM onboard messages, PX4 messages don't carry a timestamp. So we are left guessing here.
         */

    }

    /****************************
     *  RELATIVE TIMESTAMPS
     ****************************/
    /* TODO
     * We could probably interpolate some relative times based on the periods of the following periodic messages:
     *  (careful, periods are user-defined!)
     *  - ATT: can be 10Hz or 50Hz
     *  - CTUN: 10Hz
     *  - CURRENT: 10Hz
     *  - PM: 0.1Hz (every 10 sec)
     *
     * The rest seems aperiodic.
     */

    //cout << "onboard rel. time=" << nowtime_us << "us"<< endl;
    // feed data directly into system class
    for (OnboardData::booldata::const_iterator dit = msg.get_booldata().begin(); dit != msg.get_booldata().end(); ++dit) {
        if (dit->first == "t") continue;
        string fullname = "onboard log/" + msg.get_message_name() + "/" + dit->first;
        sys->track_generic_timeseries<bool>(fullname, dit->second);
    }
    for (OnboardData::intdata::const_iterator dit = msg.get_intdata().begin(); dit != msg.get_intdata().end(); ++dit) {
        if (dit->first == "t") continue;
        string fullname = "onboard log/" + msg.get_message_name() + "/" + dit->first;
        sys->track_generic_timeseries<int>(fullname, dit->second);
    }
    for (OnboardData::uintdata::const_iterator dit = msg.get_uintdata().begin(); dit != msg.get_uintdata().end(); ++dit) {
        if (dit->first == "t") continue;
        string fullname = "onboard log/" + msg.get_message_name() + "/" + dit->first;
        sys->track_generic_timeseries<unsigned int>(fullname, dit->second);
    }
    for (OnboardData::floatdata::const_iterator dit = msg.get_floatdata().begin(); dit != msg.get_floatdata().end(); ++dit) {
        if (dit->first == "t") continue;
        string fullname = "onboard log/" + msg.get_message_name() + "/" + dit->first;
        sys->track_generic_timeseries<float>(fullname, dit->second);
    }
    for (OnboardData::stringdata::const_iterator dit = msg.get_stringdata().begin(); dit != msg.get_stringdata().end(); ++dit) {
        if (dit->first == "t") continue;
        string fullname = "onboard log/" + msg.get_message_name() + "/" + dit->first;
        sys->track_generic_event<std::string>(fullname, dit->second);
    }

    return true;
}

int MavlinkScenario::add_mavlink_message(const mavlink_message_t &msg, bool allow_time_jumps) {
    /**********************************************************
     * This is a gateway function. It translates Mavlink to
     * internal formats and forwards the data for processing
     * to the individual systems. For a description of the in-
     * dividual packets see https://pixhawk.ethz.ch/mavlink/
     **********************************************************/
    int ret = 0;

    /****************************
     *  ADD/FIND SYSTEM
     ****************************/
    MavSystem* const sys = _get_or_add_system_byid(msg.sysid);    
    if (!sys) return 0; // ignore; internal error because we could neither find nor add system

    // FIXME: messages have no timestamps in their header...we need to be creative here
    uint64_t nowtime_us = sys->get_rel_time() + 1; // to preserve order when no time is given  FIXME: estimate from link rate?
    sys->update_rel_time(nowtime_us, allow_time_jumps); // if someone knows better below, it can update just again

    // that is a boundary that helps analyzing
    sys->nextmsg();

    // translate and forward begins here
    bool ignored = false;
    switch (msg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: // #0
    {
        uint8_t stype = mavlink_msg_heartbeat_get_type(&msg);
        uint8_t status = mavlink_msg_heartbeat_get_system_status(&msg);
        uint8_t basemode = mavlink_msg_heartbeat_get_base_mode(&msg);
        uint8_t custmode = mavlink_msg_heartbeat_get_base_mode(&msg);
        uint8_t autopilot = mavlink_msg_heartbeat_get_autopilot(&msg);

        // --
        sys->track_system(stype, status, autopilot, basemode, custmode);
    }
        break;

    case MAVLINK_MSG_ID_SYS_STATUS: // #1
    {
        uint16_t bat_voltage_mV = mavlink_msg_sys_status_get_voltage_battery(&msg);
        uint16_t bat_current_10mA = mavlink_msg_sys_status_get_current_battery(&msg);
        uint16_t load_promille =  mavlink_msg_sys_status_get_load(&msg);
        uint16_t drop_rate_comm = mavlink_msg_sys_status_get_drop_rate_comm(&msg);
        uint32_t enabled= mavlink_msg_sys_status_get_onboard_control_sensors_enabled(&msg);
        uint32_t present = mavlink_msg_sys_status_get_onboard_control_sensors_present(&msg);
        uint32_t health = mavlink_msg_sys_status_get_onboard_control_sensors_health(&msg);
        uint16_t errors_count[4];
        errors_count[0] = mavlink_msg_sys_status_get_errors_count1(&msg);
        errors_count[1] = mavlink_msg_sys_status_get_errors_count2(&msg);
        errors_count[2] = mavlink_msg_sys_status_get_errors_count3(&msg);
        errors_count[3] = mavlink_msg_sys_status_get_errors_count4(&msg);
        // --
        sys->track_sysperf(load_promille/10.f, bat_voltage_mV/1000.f, bat_current_10mA/100.f);
        sys->track_radio_droprate(drop_rate_comm/10.);
        sys->track_system_sensors(present, enabled, health);
        sys->track_system_errors(errors_count);
    }
        break;

    case MAVLINK_MSG_ID_SYSTEM_TIME: // #2
    {
        uint64_t time_unix_usec = mavlink_msg_system_time_get_time_unix_usec(&msg);
        uint32_t time_boot_ms = mavlink_msg_system_time_get_time_boot_ms(&msg);
        nowtime_us = time_boot_ms * 1000;        
        sys->update_time_offset(nowtime_us, time_unix_usec, allow_time_jumps); // OK -- here we establish a reference between boot time and /absolute time
    }
        break;

    case MAVLINK_MSG_ID_SET_MODE: // #11
    {
        const string fullname = "mission/set mode/";
        unsigned int target = mavlink_msg_set_mode_get_target_system(&msg);
        unsigned int base = mavlink_msg_set_mode_get_base_mode(&msg);
        unsigned int custom = mavlink_msg_set_mode_get_custom_mode(&msg);
        sys->track_generic_timeseries<unsigned int>(fullname + "target", target);
        sys->track_generic_timeseries<unsigned int>(fullname + "base", base);
        sys->track_generic_timeseries<unsigned int>(fullname + "custom", custom);
    }
        break;

    case MAVLINK_MSG_ID_PARAM_REQUEST_READ: // #20
    {
        const string fullname = "param/request_read/";
        unsigned int system = mavlink_msg_param_request_read_get_target_system(&msg);
        unsigned int component = mavlink_msg_param_request_read_get_target_component(&msg);
        int pidx = mavlink_msg_param_request_read_get_param_index(&msg);
        char buf[16];
        mavlink_msg_param_request_read_get_param_id(&msg, buf);
        string pid = buf;
        sys->track_generic_timeseries<unsigned int>(fullname + "target_system", system);
        sys->track_generic_timeseries<unsigned int>(fullname + "target_component", component);
        sys->track_generic_event<string>(fullname + "param_id", pid);
        sys->track_generic_timeseries<int>(fullname + "param_index", pidx);
    }
        break;

    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: // #21
    {
        const string fullname = "param/request_list/";
        unsigned int component = mavlink_msg_param_request_list_get_target_component(&msg);
        unsigned int system = mavlink_msg_param_request_list_get_target_system(&msg);
        sys->track_generic_timeseries<unsigned int>(fullname + "target_system", system);
        sys->track_generic_timeseries<unsigned int>(fullname + "target_component", component);
    }
        break;

    case MAVLINK_MSG_ID_PARAM_VALUE: // #22
    {
        char param_id[17]; param_id[16]=0;
        mavlink_msg_param_value_get_param_id(&msg, param_id);
        float param_value = mavlink_msg_param_value_get_param_value(&msg);
        uint8_t param_type = mavlink_msg_param_value_get_param_type(&msg);
        uint16_t param_count = mavlink_msg_param_value_get_param_count(&msg);
        uint16_t param_index = mavlink_msg_param_value_get_param_index(&msg);
        // --                
        const string fullname = "param/emit/";
        sys->track_generic_timeseries<float>(fullname + "value", param_value);
        sys->track_generic_timeseries<unsigned int>(fullname + "type", param_type);
        sys->track_generic_timeseries<unsigned int>(fullname + "count", param_count);
        sys->track_generic_timeseries<unsigned int>(fullname + "index", param_index);
    }
        break;

    case MAVLINK_MSG_ID_PARAM_SET: // #23
    {
        uint8_t target = mavlink_msg_param_set_get_target_system(&msg);
        uint8_t component = mavlink_msg_param_set_get_target_component(&msg);
        char param_id[16];
        mavlink_msg_param_set_get_param_id(&msg, param_id);
        float param_value = mavlink_msg_param_set_get_param_value(&msg);
        uint8_t param_type = mavlink_msg_param_set_get_param_type(&msg);
        const string fullname = "param/set/";
        sys->track_generic_timeseries<float>(fullname + "value", param_value);
        sys->track_generic_timeseries<unsigned int>(fullname + "target system", target);
        sys->track_generic_timeseries<unsigned int>(fullname + "target component", component);
        sys->track_generic_timeseries<unsigned int>(fullname + "type", param_type);
        sys->track_generic_event<string>(fullname + "id", param_id);
    }
        break;

    case MAVLINK_MSG_ID_GPS_RAW_INT: // #24
    {
        // RAW gps sensor values
        uint64_t timestamp_usec = mavlink_msg_gps_raw_int_get_time_usec(&msg); // since boot or!! epoch
        int upd = 0;
        if (sys->is_absolute_time(timestamp_usec)) {
            sys->update_time_offset(nowtime_us, timestamp_usec, allow_time_jumps); // OK -- here we establish a reference between boot time and /absolute time
        } else {
            upd = sys->update_rel_time(timestamp_usec, allow_time_jumps);
        }

        if (0 == upd) {
            uint8_t fix_type = mavlink_msg_gps_raw_int_get_fix_type(&msg); // 	0-1: no fix, 2: 2D fix, 3: 3D fix
            double lat = mavlink_msg_gps_raw_int_get_lat(&msg)*1E-7;
            double lon = mavlink_msg_gps_raw_int_get_lon(&msg)*1E-7;
            float alt_wgs84_m = mavlink_msg_gps_raw_int_get_alt(&msg)/1000.f;
            float hdop_m = mavlink_msg_gps_raw_int_get_eph(&msg)/100.f;
            float vdop_m = mavlink_msg_gps_raw_int_get_epv(&msg)/100.f;
            float vel_ms = mavlink_msg_gps_raw_int_get_vel(&msg)/100.f;
            float groundcourse_deg = mavlink_msg_gps_raw_int_get_cog(&msg)/100.f;
            uint8_t n_sat = mavlink_msg_gps_raw_int_get_satellites_visible(&msg);
            // --
            sys->track_gps_status(lat, lon, alt_wgs84_m, hdop_m, vdop_m, vel_ms, groundcourse_deg );
            sys->track_gps_status(n_sat, fix_type);
        }  else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_GPS_STATUS: // #25
    {
        // GPS status data
        uint8_t n_sat = mavlink_msg_gps_status_get_satellites_visible(&msg);        
        uint8_t sat_prn[20]; mavlink_msg_gps_status_get_satellite_prn(&msg, sat_prn);
        uint8_t sat_used[20]; mavlink_msg_gps_status_get_satellite_prn(&msg,sat_used);
        uint8_t sat_elev[20]; mavlink_msg_gps_status_get_satellite_elevation(&msg, sat_elev);
        uint8_t sat_azi[20]; mavlink_msg_gps_status_get_satellite_azimuth(&msg, sat_azi);
        uint8_t sat_snr[20]; mavlink_msg_gps_status_get_satellite_snr(&msg, sat_snr);                
        // --
        const string fullname = "GPS/";
        for (unsigned int k=0; k<20; k++) {
            stringstream strnum;
            strnum << k;
            sys->track_generic_timeseries<unsigned int>(fullname + "prn_" + strnum.str(), sat_prn[k], "id");
            sys->track_generic_timeseries<unsigned int>(fullname + "used_" + strnum.str(), sat_used[k]);
            sys->track_generic_timeseries<unsigned int>(fullname + "elev_" + strnum.str(), sat_elev[k], "0: right on top of receiver, 90: on the horizon");
            sys->track_generic_timeseries<unsigned int>(fullname + "azi_" + strnum.str(), sat_azi[k], "deg");
            sys->track_generic_timeseries<unsigned int>(fullname + "snr_" + strnum.str(), sat_snr[k]);
        }
        sys->track_gps_status(n_sat);
    }
        break;

    // MAVLINK_MSG_ID_RAW_IMU #26 does not appear

    case MAVLINK_MSG_ID_RAW_IMU: // #27
    {
        //uint64_t time_unix_or_boot_usec = mavlink_msg_raw_imu_get_time_usec(&msg); // OK
        int16_t acc_mg[3]; ///< acceleration milli-g x-y-z
        acc_mg[0] = mavlink_msg_raw_imu_get_xacc(&msg);
        acc_mg[1] = mavlink_msg_raw_imu_get_yacc(&msg);
        acc_mg[2] = mavlink_msg_raw_imu_get_zacc(&msg);
        int16_t gyr_mrs[3]; ///< gyro millirad/sec x-y-z
        gyr_mrs[0] = mavlink_msg_raw_imu_get_xgyro(&msg);
        gyr_mrs[1] = mavlink_msg_raw_imu_get_ygyro(&msg);
        gyr_mrs[2] = mavlink_msg_raw_imu_get_zgyro(&msg);
        int16_t mag_mT[3]; ///< magnetic field, millitesla x-y-z
        mag_mT[0] = mavlink_msg_raw_imu_get_xmag(&msg);
        mag_mT[1] = mavlink_msg_raw_imu_get_ymag(&msg);
        mag_mT[2] = mavlink_msg_raw_imu_get_zmag(&msg);
        // --
        sys->track_imu1(acc_mg, gyr_mrs, mag_mT);
    }
        break;

    case MAVLINK_MSG_ID_SCALED_PRESSURE: // #29
    {
        uint32_t time_boot_ms = mavlink_msg_scaled_pressure_get_time_boot_ms(&msg); // OK
        int16_t temp = mavlink_msg_scaled_pressure_get_temperature(&msg);
        float press = mavlink_msg_scaled_pressure_get_press_abs(&msg);
        nowtime_us = time_boot_ms * 1000;
        int upd = sys->update_rel_time(nowtime_us, allow_time_jumps); // sometimes goes backwards
        // --
        if (0 == upd) {
            sys->track_ambient(temp/100., press);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_ATTITUDE: // #30
    {
        uint32_t time_boot_ms = mavlink_msg_attitude_get_time_boot_ms(&msg); // OK
        float rpy_rad[3];
        rpy_rad[0] = mavlink_msg_attitude_get_roll(&msg);
        rpy_rad[1] = mavlink_msg_attitude_get_pitch(&msg);
        rpy_rad[2] = mavlink_msg_attitude_get_yaw(&msg);
        float speed_rpy_radsec[3];
        speed_rpy_radsec[0] = mavlink_msg_attitude_get_rollspeed(&msg);
        speed_rpy_radsec[1] = mavlink_msg_attitude_get_pitchspeed(&msg);
        speed_rpy_radsec[2] = mavlink_msg_attitude_get_yawspeed(&msg);
        // update time
        nowtime_us = time_boot_ms * 1000;
        int upd = sys->update_rel_time(nowtime_us, allow_time_jumps); // BUG: goes backwards *every* time
        // --
        if (0 == upd) {
            sys->track_paths_attitude(rpy_rad, speed_rpy_radsec);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_LOCAL_POSITION_NED: // #32
    {
        uint32_t time_boot_ms = mavlink_msg_local_position_ned_get_time_boot_ms(&msg);
        float x = mavlink_msg_local_position_ned_get_x(&msg);
        float y = mavlink_msg_local_position_ned_get_y(&msg);
        float z = mavlink_msg_local_position_ned_get_z(&msg);
        float vx = mavlink_msg_local_position_ned_get_vx(&msg);
        float vy = mavlink_msg_local_position_ned_get_vy(&msg);
        float vz = mavlink_msg_local_position_ned_get_vz(&msg);
        // update time
        nowtime_us = time_boot_ms * 1000;
        int upd = sys->update_rel_time(nowtime_us, allow_time_jumps);
        if (0==upd) {
            const string fullname = "airstate/local pos ned/";
            sys->track_generic_timeseries<float>(fullname + "x", x);
            sys->track_generic_timeseries<float>(fullname + "y", y);
            sys->track_generic_timeseries<float>(fullname + "z", z);
            sys->track_generic_timeseries<float>(fullname + "vx", vx);
            sys->track_generic_timeseries<float>(fullname + "vy", vy);
            sys->track_generic_timeseries<float>(fullname + "vz", vz);
        } else {
            ret = upd;
        }
        // --
    }
        break;

    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: // #33
    {
        uint32_t time_boot_ms = mavlink_msg_global_position_int_get_time_boot_ms(&msg);
        double lat = mavlink_msg_global_position_int_get_lat(&msg)*1E-7;
        double lon = mavlink_msg_global_position_int_get_lon(&msg)*1E-7;
        float alt_msl = mavlink_msg_global_position_int_get_alt(&msg)/1000.f;
        float alt_rel = mavlink_msg_global_position_int_get_relative_alt(&msg)/1000.f;
        float v_ms[3];
        v_ms[0] = mavlink_msg_global_position_int_get_vx(&msg)/100.f;
        v_ms[1] = mavlink_msg_global_position_int_get_vy(&msg)/100.f;
        v_ms[2] = mavlink_msg_global_position_int_get_vz(&msg)/100.f;
        float heading_deg = mavlink_msg_global_position_int_get_hdg(&msg)/100.f;
        // update time
        nowtime_us = time_boot_ms * 1000;
        int upd = sys->update_rel_time(nowtime_us, allow_time_jumps);
        // --
        if (0==upd) {
            sys->track_paths(lat, lon, alt_rel, alt_msl, heading_deg);
            sys->track_paths_speed(v_ms);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_RC_CHANNELS_SCALED: // #34
    {
        uint32_t time_boot_ms = mavlink_msg_rc_channels_scaled_get_time_boot_ms(&msg); // OK
        /*
        uint8_t port = mavlink_msg_rc_channels_scaled_get_port(&msg);
        uint16_t channel[8];
        channel[0] = mavlink_msg_rc_channels_scaled_get_chan1_scaled(&msg);
        channel[1] = mavlink_msg_rc_channels_scaled_get_chan2_scaled(&msg);
        channel[2] = mavlink_msg_rc_channels_scaled_get_chan3_scaled(&msg);
        channel[3] = mavlink_msg_rc_channels_scaled_get_chan4_scaled(&msg);
        channel[4] = mavlink_msg_rc_channels_scaled_get_chan5_scaled(&msg);
        channel[5] = mavlink_msg_rc_channels_scaled_get_chan6_scaled(&msg);
        channel[6] = mavlink_msg_rc_channels_scaled_get_chan7_scaled(&msg);
        channel[7] = mavlink_msg_rc_channels_scaled_get_chan8_scaled(&msg);
        */
        uint8_t rssi = mavlink_msg_rc_channels_scaled_get_rssi(&msg);
        // update time
        nowtime_us = time_boot_ms * 1000;

        int upd = sys->update_rel_time(nowtime_us, allow_time_jumps); // BUG: goes backwards
        // --
        if (0 == upd) {
            sys->track_radio(rssi);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_RC_CHANNELS_RAW: // #35
    {
        uint32_t time_boot_ms = mavlink_msg_rc_channels_raw_get_time_boot_ms(&msg); // OK
        uint16_t chan_raw[8];
        chan_raw[0] = mavlink_msg_rc_channels_raw_get_chan1_raw(&msg);
        chan_raw[1] = mavlink_msg_rc_channels_raw_get_chan2_raw(&msg);
        chan_raw[2] = mavlink_msg_rc_channels_raw_get_chan3_raw(&msg);
        chan_raw[3] = mavlink_msg_rc_channels_raw_get_chan4_raw(&msg);
        chan_raw[4] = mavlink_msg_rc_channels_raw_get_chan5_raw(&msg);
        chan_raw[5] = mavlink_msg_rc_channels_raw_get_chan6_raw(&msg);
        chan_raw[6] = mavlink_msg_rc_channels_raw_get_chan7_raw(&msg);
        chan_raw[7] = mavlink_msg_rc_channels_raw_get_chan8_raw(&msg);
        // update time
        nowtime_us = time_boot_ms * 1000;
        int upd = sys->update_rel_time(nowtime_us, allow_time_jumps);
        // --
        if (0 == upd) {
            sys->track_rc(chan_raw);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW: // #36
    {
        uint32_t time_boot_usec = mavlink_msg_servo_output_raw_get_time_usec(&msg); // OK
        //uint8_t port = mavlink_msg_servo_output_raw_get_port(&msg);
        uint16_t servo_raw[8];
        servo_raw[0] = mavlink_msg_servo_output_raw_get_servo1_raw(&msg);
        servo_raw[1] = mavlink_msg_servo_output_raw_get_servo2_raw(&msg);
        servo_raw[2] = mavlink_msg_servo_output_raw_get_servo3_raw(&msg);
        servo_raw[3] = mavlink_msg_servo_output_raw_get_servo4_raw(&msg);
        servo_raw[4] = mavlink_msg_servo_output_raw_get_servo5_raw(&msg);
        servo_raw[5] = mavlink_msg_servo_output_raw_get_servo6_raw(&msg);
        servo_raw[6] = mavlink_msg_servo_output_raw_get_servo7_raw(&msg);
        servo_raw[7] = mavlink_msg_servo_output_raw_get_servo8_raw(&msg);
        // update time        
        int upd = sys->update_rel_time(time_boot_usec, allow_time_jumps);
        // --
        if ( 0 == upd) {
            sys->track_actuators(servo_raw);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_MISSION_ITEM: // #39
    {
        uint8_t target_system_id = mavlink_msg_mission_item_get_target_system(&msg);
        uint8_t target_comp_id = mavlink_msg_mission_item_get_target_component(&msg);
        uint16_t seq = mavlink_msg_mission_item_get_seq(&msg);
        uint8_t frame = mavlink_msg_mission_item_get_frame(&msg);
        uint16_t cmd = mavlink_msg_mission_item_get_command(&msg);
        uint8_t current = mavlink_msg_mission_item_get_current(&msg);
        uint8_t autocontinue = mavlink_msg_mission_item_get_autocontinue(&msg);
        float param1 = mavlink_msg_mission_item_get_param1(&msg);
        float param2 = mavlink_msg_mission_item_get_param2(&msg);
        float param3 = mavlink_msg_mission_item_get_param3(&msg);
        float param4 = mavlink_msg_mission_item_get_param4(&msg);
        float x = mavlink_msg_mission_item_get_x(&msg);
        float y = mavlink_msg_mission_item_get_y(&msg);
        float z = mavlink_msg_mission_item_get_z(&msg);
        // --
        sys->track_mission_item(target_system_id, target_comp_id, seq, frame, cmd, current, autocontinue, param1, param2, param3, param4, x, y, z);
    }
        break;

    case MAVLINK_MSG_ID_MISSION_REQUEST: // #40
    {
        uint8_t target_system = mavlink_msg_mission_request_get_target_system(&msg);
        uint8_t target_comp = mavlink_msg_mission_request_get_target_component(&msg);
        uint16_t seq = mavlink_msg_mission_request_get_seq(&msg);
        const string fullname = "mission/request/";
        sys->track_generic_timeseries<unsigned int>(fullname + "target system", target_system);
        sys->track_generic_timeseries<unsigned int>(fullname + "target component", target_comp);
        sys->track_generic_timeseries<unsigned int>(fullname + "seq", seq);
    }
        break;


    case MAVLINK_MSG_ID_MISSION_CURRENT: // #42
    {
        uint16_t seq = mavlink_msg_mission_current_get_seq(&msg);
        sys->track_mission_current(seq);
    }
        break;

    case MAVLINK_MSG_ID_MISSION_COUNT: // #44
    {
        uint8_t target_system = mavlink_msg_mission_count_get_target_system(&msg);
        uint8_t target_comp = mavlink_msg_mission_count_get_target_component(&msg);
        uint16_t count = mavlink_msg_mission_count_get_count(&msg);
        const string fullname = "mission/count/";
        sys->track_generic_timeseries<unsigned int>(fullname + "target system", target_system);
        sys->track_generic_timeseries<unsigned int>(fullname + "target component", target_comp);
        sys->track_generic_timeseries<unsigned int>(fullname + "count", count);
    }
        break;

    case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT: // #62
    {
        float nav_roll_deg = mavlink_msg_nav_controller_output_get_nav_roll(&msg);
        float nav_pitch_deg = mavlink_msg_nav_controller_output_get_nav_pitch(&msg);
        float nav_bear_deg = (float) mavlink_msg_nav_controller_output_get_nav_bearing(&msg);
        float tar_bear_deg = (float) mavlink_msg_nav_controller_output_get_target_bearing(&msg);
        float wp_dist_m = (float) mavlink_msg_nav_controller_output_get_wp_dist(&msg);
        float err_alt_m = mavlink_msg_nav_controller_output_get_alt_error(&msg);
        float err_airspeed_ms = mavlink_msg_nav_controller_output_get_aspd_error(&msg);
        float err_xtrack_m = mavlink_msg_nav_controller_output_get_xtrack_error(&msg);
        // --
        sys->track_nav(nav_roll_deg, nav_pitch_deg, nav_bear_deg, tar_bear_deg, wp_dist_m, err_alt_m, err_airspeed_ms, err_xtrack_m);
    }
        break;

    case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: // #66
    {
        unsigned int target_system = mavlink_msg_request_data_stream_get_target_system(&msg);
        unsigned int target_component = mavlink_msg_request_data_stream_get_target_component(&msg);
        unsigned int req_stream_id = mavlink_msg_request_data_stream_get_req_stream_id(&msg);
        unsigned int req_msg_rate = mavlink_msg_request_data_stream_get_req_message_rate(&msg);
        unsigned int start_stop = mavlink_msg_request_data_stream_get_start_stop(&msg);
        const string fullname = "datastream/request/";
        sys->track_generic_timeseries<unsigned int>(fullname + "target system", target_system);
        sys->track_generic_timeseries<unsigned int>(fullname + "target component", target_component);
        sys->track_generic_timeseries<unsigned int>(fullname + "requested stream id", req_stream_id);
        sys->track_generic_timeseries<unsigned int>(fullname + "requeted message rate", req_msg_rate);
        sys->track_generic_timeseries<unsigned int>(fullname + "start stop", start_stop);
    }
        break;

    case MAVLINK_MSG_ID_VFR_HUD: // #74
    {
        float airspeed_ms = mavlink_msg_vfr_hud_get_airspeed(&msg);
        float groundspeed_ms = mavlink_msg_vfr_hud_get_groundspeed(&msg);
        float alt_MSL_m = mavlink_msg_vfr_hud_get_alt(&msg);
        float climb_ms = mavlink_msg_vfr_hud_get_climb(&msg);
        uint16_t throttle_percent = mavlink_msg_vfr_hud_get_throttle(&msg);
        // --
        sys->track_flightperf(airspeed_ms, groundspeed_ms, alt_MSL_m, climb_ms, (float) throttle_percent);
    }
        break;

    case MAVLINK_MSG_ID_COMMAND_LONG: // #76
    {
        const string fullname = "mission/command_long/";
        unsigned int target_system = mavlink_msg_command_long_get_target_system(&msg);
        unsigned int target_component = mavlink_msg_command_long_get_target_component(&msg);
        unsigned int cmd = mavlink_msg_command_long_get_command(&msg);
        unsigned int conf = mavlink_msg_command_long_get_confirmation(&msg);
        float p1 = mavlink_msg_command_long_get_param1(&msg);
        float p2 = mavlink_msg_command_long_get_param2(&msg);
        float p3 = mavlink_msg_command_long_get_param3(&msg);
        float p4 = mavlink_msg_command_long_get_param4(&msg);
        float p5 = mavlink_msg_command_long_get_param5(&msg);
        float p6 = mavlink_msg_command_long_get_param6(&msg);
        float p7 = mavlink_msg_command_long_get_param7(&msg);
        sys->track_generic_timeseries<unsigned int>(fullname + "target system", target_system);
        sys->track_generic_timeseries<unsigned int>(fullname + "target component", target_component);
        sys->track_generic_timeseries<unsigned int>(fullname + "command", cmd, "MAV_CMD enum");
        sys->track_generic_timeseries<unsigned int>(fullname + "confirmation", conf, "0: first transmission. 1-255: confirm transmissions");
        sys->track_generic_timeseries<float>(fullname + "param1", p1, "param1 of MAV_CMD enum");
        sys->track_generic_timeseries<float>(fullname + "param2", p2, "param2 of MAV_CMD enum");
        sys->track_generic_timeseries<float>(fullname + "param3", p3, "param3 of MAV_CMD enum");
        sys->track_generic_timeseries<float>(fullname + "param4", p4, "param4 of MAV_CMD enum");
        sys->track_generic_timeseries<float>(fullname + "param5", p5, "param5 of MAV_CMD enum");
        sys->track_generic_timeseries<float>(fullname + "param6", p6, "param6 of MAV_CMD enum");
        sys->track_generic_timeseries<float>(fullname + "param7", p7, "param7 of MAV_CMD enum");
    }
        break;

    case MAVLINK_MSG_ID_COMMAND_ACK: // #77
    {
        unsigned int cmd = mavlink_msg_command_ack_get_command(&msg);
        unsigned int res = mavlink_msg_command_ack_get_result(&msg);
        const string fullname = "mission/command ack/";
        sys->track_generic_timeseries<unsigned int>(fullname + "command", cmd, "MAV_CMD enum");
        sys->track_generic_timeseries<unsigned int>(fullname + "result", res, "MAV_RESULT enum");
    }
        break;

    case MAVLINK_MSG_ID_ATTITUDE_TARGET: // #83
    {
        const string fullname = "mission/attitude target/";
        uint32_t time_boot_ms = mavlink_msg_attitude_target_get_time_boot_ms(&msg);        
        int upd = sys->update_rel_time(time_boot_ms*1000, allow_time_jumps);
        if (0 == upd) {
            uint8_t type_mask = mavlink_msg_attitude_target_get_type_mask(&msg);
            float q[4];
            mavlink_msg_attitude_target_get_q(&msg, q);
            float body_rollrate = mavlink_msg_attitude_target_get_body_roll_rate(&msg);
            float body_pitchrate = mavlink_msg_attitude_target_get_body_pitch_rate(&msg);
            float body_yawrate = mavlink_msg_attitude_target_get_body_yaw_rate(&msg);
            float thrust = mavlink_msg_attitude_target_get_thrust(&msg);
            sys->track_generic_timeseries<unsigned int>(fullname + "type mask", type_mask, "If any of these bits are set, the corresponding input should be ignored: bit 1: body roll rate, bit 2: body pitch rate, bit 3: body yaw rate. bit 4-bit 7: reserved, bit 8: attitude");
            sys->track_generic_timeseries<float>(fullname + "attitude/q0", q[0], "w");
            sys->track_generic_timeseries<float>(fullname + "attitude/q1", q[1], "x");
            sys->track_generic_timeseries<float>(fullname + "attitude/q2", q[2], "y");
            sys->track_generic_timeseries<float>(fullname + "attitude/q3", q[3], "z");
            sys->track_generic_timeseries<float>(fullname + "thrust", thrust, "collective, normalized 0..1");
            sys->track_generic_timeseries<float>(fullname + "rates/roll", body_rollrate, "rad/s");
            sys->track_generic_timeseries<float>(fullname + "rates/pitch", body_pitchrate, "rad/s");
            sys->track_generic_timeseries<float>(fullname + "rates/yaw", body_yawrate, "rad/s");
        } else {
            ret = upd;
        }

    }

    case MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT: // #87
    {
        const string fullname = "mission/pos target global int/";
        uint32_t time_boot_ms = mavlink_msg_position_target_global_int_get_time_boot_ms(&msg);

        int upd = sys->update_rel_time(time_boot_ms * 1000, allow_time_jumps);
        if (0 == upd) {

            uint8_t frame = mavlink_msg_position_target_global_int_get_coordinate_frame(&msg);
            uint16_t typemask = mavlink_msg_position_target_global_int_get_type_mask(&msg);
            int32_t lat = mavlink_msg_position_target_global_int_get_lat_int(&msg);
            int32_t lon = mavlink_msg_position_target_global_int_get_lon_int(&msg);
            float alt = mavlink_msg_position_target_global_int_get_alt(&msg);
            float v[3];
            v[0] = mavlink_msg_position_target_global_int_get_vx(&msg);
            v[1] = mavlink_msg_position_target_global_int_get_vy(&msg);
            v[2] = mavlink_msg_position_target_global_int_get_vz(&msg);
            float af[3];
            af[0] = mavlink_msg_position_target_global_int_get_afx(&msg);
            af[1] = mavlink_msg_position_target_global_int_get_afy(&msg);
            af[2] = mavlink_msg_position_target_global_int_get_afz(&msg);
            float yaw = mavlink_msg_position_target_global_int_get_yaw(&msg);
            float yaw_rate = mavlink_msg_position_target_global_int_get_yaw_rate(&msg);

            sys->track_generic_timeseries<float>(fullname + "type mask", typemask, "Bitmask to indicate which dimensions should be ignored by the vehicle. If bit 10 is set the floats afx afy afz should be interpreted as force instead of acceleration. Mapping: bit 1: x, bit 2: y, bit 3: z, bit 4: vx, bit 5: vy, bit 6: vz, bit 7: ax, bit 8: ay, bit 9: az, bit 10: is force setpoint, bit 11: yaw, bit 12: yaw rate");
            sys->track_generic_timeseries<float>(fullname + "speed/vx", v[0], "m/s");
            sys->track_generic_timeseries<float>(fullname + "speed/vy", v[1], "m/s");
            sys->track_generic_timeseries<float>(fullname + "speed/vz", v[2], "m/s");
            sys->track_generic_timeseries<float>(fullname + "accel or force/afx", af[0], "m/s/s");
            sys->track_generic_timeseries<float>(fullname + "accel or force/afy", af[1], "m/s/s");
            sys->track_generic_timeseries<float>(fullname + "accel or force/afz", af[2], "m/s/s");
            sys->track_generic_timeseries<float>(fullname + "lat", lat*1E-7, "WGS84 pos in m");
            sys->track_generic_timeseries<float>(fullname + "lon", lon*1E-7, "WGS84 pos in m");
            sys->track_generic_timeseries<float>(fullname + "alt", alt, "m AMSL");
            sys->track_generic_timeseries<float>(fullname + "yaw", yaw, "rad");
            sys->track_generic_timeseries<float>(fullname + "yaw rate", yaw_rate, "rad/s");
            sys->track_generic_timeseries<unsigned int>(fullname + "frame", frame, "MAV_FRAME_GLOBAL_INT = 5, MAV_FRAME_GLOBAL_RELATIVE_ALT_INT = 6, MAV_FRAME_GLOBAL_TERRAIN_ALT_INT = 11");
        }  else {
            ret = upd;
        }
    }

    case MAVLINK_MSG_ID_RADIO_STATUS: // #109
    {
        // local
        uint8_t  rssi = mavlink_msg_radio_status_get_rssi(&msg);
        uint8_t  noise = mavlink_msg_radio_status_get_noise(&msg);
        uint16_t rxerr = mavlink_msg_radio_status_get_rxerrors(&msg);
        uint16_t rxerr_corrected = mavlink_msg_radio_status_get_fixed(&msg);
        uint8_t  txbuf_percent = mavlink_msg_radio_status_get_txbuf(&msg);
        // remote
        uint8_t  rem_rssi = mavlink_msg_radio_status_get_remrssi(&msg);
        uint8_t  rem_noise = mavlink_msg_radio_status_get_remnoise(&msg);
        // --
        sys->track_radio(rssi, noise, rxerr, rxerr_corrected, txbuf_percent, rem_rssi, rem_noise );
    }
        break;

    case MAVLINK_MSG_ID_HIGHRES_IMU: // #105:
    {
        uint64_t timestamp_usec = mavlink_msg_highres_imu_get_time_usec(&msg); // since boot or!! epoch

        int upd = 0;
        if (sys->is_absolute_time(timestamp_usec)) {
            sys->update_time_offset(nowtime_us, timestamp_usec, allow_time_jumps); // OK -- here we establish a reference between boot time and /absolute time
        } else {
            upd = sys->update_rel_time(timestamp_usec, allow_time_jumps);
        }

        if (0 == upd) {

            float acc_mss[3]; ///< acceleration in m/s/s
            acc_mss[0] = mavlink_msg_highres_imu_get_xacc(&msg);
            acc_mss[1] = mavlink_msg_highres_imu_get_yacc(&msg);
            acc_mss[2] = mavlink_msg_highres_imu_get_zacc(&msg);
            float gyro_rad[3]; ///< gyro in rad/s
            gyro_rad[0] = mavlink_msg_highres_imu_get_xgyro(&msg);
            gyro_rad[1] = mavlink_msg_highres_imu_get_ygyro(&msg);
            gyro_rad[2] = mavlink_msg_highres_imu_get_zgyro(&msg);
            float mag_gauss[3]; ///< magnetometer in gauss
            mag_gauss[0] = mavlink_msg_highres_imu_get_xmag(&msg);
            mag_gauss[1] = mavlink_msg_highres_imu_get_ymag(&msg);
            mag_gauss[2] = mavlink_msg_highres_imu_get_zmag(&msg);
            float abs_press_mbar = mavlink_msg_highres_imu_get_abs_pressure(&msg);
            float diff_press_mbar = mavlink_msg_highres_imu_get_diff_pressure(&msg);
            float pressure_alt = mavlink_msg_highres_imu_get_pressure_alt(&msg); // units??
            float temp_degC = mavlink_msg_highres_imu_get_temperature(&msg);
            uint16_t updated = mavlink_msg_highres_imu_get_fields_updated(&msg);
            // --
            if (updated & (1     )) sys->track_imu_highres_acc(acc_mss); // assuming that y and z are also updated...
            if (updated & (1 << 3)) sys->track_imu_highres_gyr(gyro_rad); // assuming that y and z are also updated...
            if (updated & (1 << 6)) sys->track_imu_highres_mag(mag_gauss); // assuming that y and z are also updated...
            if (updated & (1 << 9)) sys->track_imu_highres_pressabs(abs_press_mbar);
            if (updated & (1 << 10)) sys->track_imu_highres_pressdiff(diff_press_mbar);
            if (updated & (1 << 11)) sys->track_imu_highres_pressalt(pressure_alt);
            if (updated & (1 << 12)) sys->track_imu_highres_temp(temp_degC);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_SCALED_IMU2: // #116
    {
        uint64_t timestamp_usec = mavlink_msg_raw_imu_get_time_usec(&msg); // OK

        int upd = 0;
        if (sys->is_absolute_time(timestamp_usec)) {
            sys->update_time_offset(nowtime_us, timestamp_usec, allow_time_jumps); // OK -- here we establish a reference between boot time and /absolute time
        } else {
            upd = sys->update_rel_time(timestamp_usec, allow_time_jumps);
        }

        if (0 == upd) {
            int16_t acc_mg[3]; ///< acceleration milli-g x-y-z
            acc_mg[0] = mavlink_msg_raw_imu_get_xacc(&msg);
            acc_mg[1] = mavlink_msg_raw_imu_get_yacc(&msg);
            acc_mg[2] = mavlink_msg_raw_imu_get_zacc(&msg);
            int16_t gyr_mrs[3]; ///< gyro millirad/sec x-y-z
            gyr_mrs[0] = mavlink_msg_raw_imu_get_xgyro(&msg);
            gyr_mrs[1] = mavlink_msg_raw_imu_get_ygyro(&msg);
            gyr_mrs[2] = mavlink_msg_raw_imu_get_zgyro(&msg);
            int16_t mag_mT[3]; ///< magnetic field, millitesla x-y-z
            mag_mT[0] = mavlink_msg_raw_imu_get_xmag(&msg);
            mag_mT[1] = mavlink_msg_raw_imu_get_ymag(&msg);
            mag_mT[2] = mavlink_msg_raw_imu_get_zmag(&msg);
            // --
            sys->track_imu2(acc_mg, gyr_mrs, mag_mT);
        } else {
            ret = upd;
        }
    }
        break;

    case MAVLINK_MSG_ID_POWER_STATUS: // #125
    {
        uint16_t Vcc = mavlink_msg_power_status_get_Vcc(&msg);
        uint16_t Vservo = mavlink_msg_power_status_get_Vservo(&msg);
        uint16_t flags = mavlink_msg_power_status_get_flags(&msg);
        // --
        sys->track_power(Vcc/1000.f, Vservo/1000.f, flags);
    }
        break;


    case MAVLINK_MSG_ID_BATTERY_STATUS: // #147
    {
        unsigned int id = mavlink_msg_battery_status_get_id(&msg);
        int bat_rem = mavlink_msg_battery_status_get_battery_remaining(&msg);
        float bat_cur = mavlink_msg_battery_status_get_current_battery(&msg)/100.; // 10mA -> A
        float bat_cur_consumed = mavlink_msg_battery_status_get_current_consumed(&msg)/1000.; // mAh -> Ah
        float bat_energy_consumed = mavlink_msg_battery_status_get_energy_consumed(&msg)/100.; // 100J -> J
        float v[10];
        // --
        const string fullname = "power/battery/";
        {
            uint16_t u16v[10];
            mavlink_msg_battery_status_get_voltages(&msg, u16v);
            for (unsigned int k=0; k<10; k++) {
                v[k] = ((float) u16v[k]) / 1000.; // mv (uint) -> V (float)
                stringstream num;
                num << k;
                sys->track_generic_timeseries<unsigned int>(fullname + "volts cell " + num.str(), v[k], "V");
            }
        }
        sys->track_generic_timeseries<float>(fullname + "accu id", id);
        sys->track_generic_timeseries<int>(fullname + "percent remaining", bat_rem, "%");
        sys->track_generic_timeseries<float>(fullname + "current", bat_cur, "A");
        sys->track_generic_timeseries<float>(fullname + "current consumed", bat_cur_consumed, "Ah");
        sys->track_generic_timeseries<float>(fullname + "energy consumed", bat_energy_consumed, "J");
    }
        break;

    case MAVLINK_MSG_ID_STATUSTEXT: // #253
    {
        char text[51]; text[50]=0;
        mavlink_msg_statustext_get_text(&msg, text);
        uint8_t severity = mavlink_msg_statustext_get_severity(&msg);
        // --
        sys->track_statustext(text, severity);
    }
        break;

    default:
        // unknown        
        ignored = true;
        break;
    }

    // debug:
    //log(MSG_DBG, stringbuilder() << "Sys=" << (unsigned int)msg.sysid << ", comp=" << (unsigned int)msg.compid);

    // track link stats
    _n_msgs++; // scenario-wide
    MavSystem::mavlink_parsed_e whatwasdone = MavSystem::MAVLINK_INTERPRETED;
    if (ignored) {
        whatwasdone = MavSystem::MAVLINK_UNINTERPRETED;
        _n_ignored++; // scenario-wide
        ret = 0; // because return value is only to indicate time jumps.
    }
    sys->track_mavlink(msg.len + MAVLINK_NUM_NON_PAYLOAD_BYTES, msg.msgid, whatwasdone);

    return ret;
}

void MavlinkScenario::shift_time(double delay_sec) {
    if (delay_sec == 0.0) return;

    int64_t udelay = delay_sec*1E6;
    for (systemlist::iterator it = _seen_systems.begin(); it != _seen_systems.end(); ++it) {
        it->second->shift_time(delay_sec);
    }
    _time_guess_epoch_usec += udelay;
}

void MavlinkScenario::process(bool calculate_time_offset) {
    /* TODO: here is a bug: if we merge other scenarios into this, and if this was empty before,
     * then _time_guess_epoch_usec is 0. This then overwrites the guess of the systems of the scenario,
     * which is being merged in. Result: When no time info was in the data, it looses its time refernce.
     */
    for (systemlist::iterator it = _seen_systems.begin(); it != _seen_systems.end(); ++it) {
        if (calculate_time_offset) it->second->update_time_offset_guess(0, _time_guess_epoch_usec); // for each system take a guess
        it->second->postprocess(); // compute more data based on the raw data...
        if (calculate_time_offset) it->second->determine_absolute_time();
    }
}

void MavlinkScenario::dump_overview(void) {
    dump_overview(std::cout); // FIXME
}

void MavlinkScenario::dump_overview(ostream& ofs) const {
    ofs << "SUMMARY:" << endl <<
            "Number of systems: " << _seen_systems.size() << endl << endl;

    stringstream ss;
    for (systemlist::const_iterator it = _seen_systems.begin(); it != _seen_systems.end(); ++it) {
        string tmp;
        it->second->get_summary(tmp);
        ss << tmp << endl;
    }

    ofs << ss.str();
    ofs << "Processed " << _n_msgs << " messages." << endl;
}

const MavSystem *MavlinkScenario::get_system_byid(uint8_t id) const {
    systemlist::const_iterator it = _seen_systems.find(id);
    if (it == _seen_systems.end())  return NULL;
    const MavSystem* sys = it->second;
    return sys;
}

bool MavlinkScenario::merge_in(const MavlinkScenario & other) {
    // for each system in there: see if we have it. If so, merge its data in. Else, copy it.
    bool success = true;
    for (systemlist::const_iterator ito = other._seen_systems.begin(); ito != other._seen_systems.end(); ++ito) {
        systemlist::iterator mine = _seen_systems.find(ito->first);
        if (mine == _seen_systems.end()) {
            // do not have it, take a copy
            _seen_systems.insert(_seen_systems.begin(), std::pair<uint8_t,MavSystem*>(ito->first,new MavSystem(ito->second))); ///< copy CTOR
        } else {
            // do have it, merge!
            if (!mine->second->merge_in(ito->second)) {
                log(MSG_ERR, stringbuilder() << "ERROR merging two MavSystems");
                success = false;
            }
        }
    }
    return success;
}

std::vector<const MavSystem*> MavlinkScenario::getSystems() const {
    std::vector<const MavSystem*> ret;
    for (systemlist::const_iterator it = _seen_systems.begin(); it != _seen_systems.end(); ++it) {
        const MavSystem*sys = it->second;
        ret.push_back(sys);
    }
    return ret;
}

double MavlinkScenario::get_scenario_starttime_sec(void) const {
    unsigned long t = get_scenario_starttime_usec();
    double ret = t / 1E6;
    return ret;
}

unsigned long MavlinkScenario::get_scenario_starttime_usec(void) const {
    unsigned long tstart = 0;
    for (systemlist::const_iterator it = _seen_systems.begin(); it != _seen_systems.end(); ++it) {
        const MavSystem*sys = it->second;
        unsigned long thisstart = sys->get_time_active_begin_usec();
        if (thisstart > 0) {
            // find lower bound
            if ((thisstart < tstart) || (tstart == 0)) tstart = thisstart;
        }
    }
    return tstart;
}

void MavlinkScenario::set_starttime_guess(uint64_t time_epoch_usec) {
    _time_guess_epoch_usec = time_epoch_usec;
}
