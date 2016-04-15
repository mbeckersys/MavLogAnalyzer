/**
 * @file onboardlogparser.cpp
 * @brief TODO: describe this file
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 1/20/2016
 
    This file is part of MavLogAnalyzer, Copyright 2016 by Martin Becker.
    
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

#include "onboardlogparser.h"

/**
 * @brief assign a more readable name to the messages, s.g. "CTUN" becomes "Controller Tuning"
 * @param msgname original name as in log
 * @return readable name
 */
std::string OnboardLogParser::_make_readable_name(std::string msgname) {
    if (msgname.compare("PARM")==0) {
        return "System/Parameters";
    } else if (msgname.compare("ATT")==0) {
        return "Airstate/Attitude";
    } else if (msgname.compare("ATUN")==0) {
        return "Controller/Auto-tune";
    } else if (msgname.compare("ATDE")==0) {
        return "Controller/Auto-tune details";
    } else if (msgname.compare("CAMERA")==0) {
        return "Sensors/Camera";
    } else if (msgname.compare("CMD")==0) {
        return "Commands";
    } else if (msgname.compare("COMPASS")==0) {
        return "Sensors/Compass raw";
    } else if (msgname.compare("CURR")==0 || msgname.compare("CURRENT")==0) {
        return "Power";
    } else if (msgname.compare("CTUN")==0) {
        return "Airstate/Throttle+Alt";
    } else if (msgname.compare("D32")==0) {
        return "Airstate/Simple Mode Heading";
    } else if (msgname.compare("DU32")==0) {
        return "System/Internal State";
    } else if (msgname.compare("ERR")==0) {
        return "System/Errors";
    } else if (msgname.compare("EV")==0) {
        return "System/Events";
    } else if (msgname.compare("INAV")==0) {
        return "Navigation/Estimates";
    } else if (msgname.compare("MODE")==0) {
        return "System/Mode";
    } else if (msgname.compare("MSG")==0) {
        return "System/Messages";
    } else if (msgname.compare("NTUN")==0) {
        return "Navigation/Lateral";
    } else if (msgname.compare("OPTFLOW")==0) {
        return "Sensors/Optical Flow";
    } else if (msgname.compare("PID")==0) {
        return "Controller/PID outputs";
    } else if (msgname.compare("PM")==0) {
        return "System/Performance Monitor";
    } else if (msgname.compare("RCOUT")==0) {
        return "Controller/PWM outputs";
    } else if (msgname.compare("LPSP")==0) {
        return "Setpoints/Local Position";
    } else if (msgname.compare("GVSP")==0) {
        return "Setpoints/Global Velocity";
    } else if (msgname.compare("GPSP")==0) {
        return "Setpoints/Global Position";
    } else if (msgname.compare("ATSP")==0) {
        return "Setpoints/Attitude";
    } else if (msgname.compare("IMU")==0) {
        return "Airstate/IMU";
    } else if (msgname.compare("IMU1")==0) {
        return "Airstate/IMU1";
    } else if (msgname.compare("PWR")==0) {
        return "Power";
    } else if (msgname.compare("GPOS")==0) {
        return "Airstate/Position Global";
    } else if (msgname.compare("LPOS")==0) {
        return "Airstate/Position Local";
    } else if (msgname.compare("BATT")==0) {
        return "Power/Battery";
    } else if (msgname.compare("STAT")==0) {
        return "System/State";
    } else if (msgname.compare("VER")==0) {
        return "System/Version";
    } else if (msgname.compare("VISN")==0) {
        return "Vision/Position Estimate";
    } else if (msgname.compare("OUT0")==0) {
        return "Actuator/Output";
    } else if (msgname.compare("MACS")==0) {
        return "Controller/Multirotor Attitude";
    } else if (msgname.compare("DIST")==0) {
        return "Sensors/Distance";
    } else if (msgname.compare("SENS")==0) {
        return "Sensors/Barometric";
    } else if (msgname.compare("TEL0")==0) {
        return "Radio/Unit 0";
    } else if (msgname.compare("FLOW")==0) {
        return "Sensors/Optical Flow";
    } else if (msgname.compare("ATTC")==0) {
        return "Controller/Actuator";
    } else if (msgname.compare("ATC1")==0) {
        return "Controller/Actuator FW VTOL";
    } else if (msgname.compare("AIRS")==0) {
        return "Airstate/Airspeed";
    } else if (msgname.compare("ARSP")==0) {
        return "Setpoints/Rates";
    }
    return msgname;
}
