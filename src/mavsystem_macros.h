/**
 * @file mavsystem_macros.h
 * @brief A handy set of macros used in MavSystem class. Need it here because of templates.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 13.11.2014
 
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

#ifndef MAVSYSTEM_MACROS_H
#define MAVSYSTEM_MACROS_H

/**
 * Convenience macro as a shorthand for conditionally creating and remembering data. E.g.:
 *   MAVSYSTEM_DATA_ITEM(DataTimeseries<float>, data_autopilot_load, "general/autopilot_load");
 * unrolls to:
 *   DataTimeseries<float> *data_autopilot_load = _get_data<DataTimeseries<float> >(string("general/autopilot_load"));
 */
#define MAVSYSTEM_DATA_ITEM(DTYPE, VARNAME, STR_FULLPATH, STR_UNITS) \
    DTYPE *const VARNAME = _get_and_possibly_create_data< DTYPE > (std::string( STR_FULLPATH ), std::string (STR_UNITS)); \
    if (!VARNAME) { /* because there could be something with same name but different type */ \
        _log(MSG_ERR, stringbuilder() << "(#" << id << ") writing to data " << STR_FULLPATH << " at " << __FILE__ << ":" << __LINE__  << ". Is there a type mismatch?"); \
        return; \
    }
// FIXME: string could be made static...does it help?

#define MAVSYSTEM_DATA_ITEM_HAVEVAR(DTYPE, STR_FULLPATH, STR_UNITS) \
    _get_and_possibly_create_data< DTYPE > (std::string( STR_FULLPATH ), std::string (STR_UNITS))

/**
 * Convenience macro to get data without knowing units
 */
#define MAVSYSTEM_READ_DATA(DTYPE, VARNAME, STR_FULLPATH) \
    const DTYPE * const VARNAME = get_data< DTYPE > ( STR_FULLPATH );

/**
 * @brief convenience macro to obtain data if available, else return
 */
#define MAVSYSTEM_REQUIRE_DATA(DTYPE, VARNAME, STR_FULLPATH) \
    MAVSYSTEM_READ_DATA(DTYPE, VARNAME, STR_FULLPATH);\
    if (!VARNAME) { \
        _log(MSG_WARN, stringbuilder() << "WARNING (#" << id << ") could not find required data " << STR_FULLPATH << " at " << __FILE__ << ":" << __LINE__  << ". Data does not exist or there is a type mismatch."); \
        return; \
    }

#endif // MAVSYSTEM_MACROS_H
