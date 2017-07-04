/**
 * @file data_timed.h
 * @brief superclass for all timed kind of data
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 7/4/2017

    This file is part of MavLogAnalyzer, Copyright 2017 by Martin Becker.

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
#ifndef DATA_TIMED_H
#define DATA_TIMED_H

#include "data.h"

/**
 * @brief additional interface for all timed kind of data
 */
class DataTimed : public Data {
public:
    // CTOR
    DataTimed(std::string name) : Data(name), _bad_timestamps (false) { }

    // copy CTOR
    DataTimed(const DataTimed & other) : Data(other) {
        _bad_timestamps = other._bad_timestamps;
    }

    /**
     * @brief some timeseries do not have reliable time annotations
     * @return
     */
    bool has_bad_timestamps(void) const { return _bad_timestamps; }
    void set_has_bad_timestamps (void) { _bad_timestamps = true; }

    /**
     * @brief distribute data equally over time span
     */
    virtual void make_periodic() = 0;

protected:
    bool _bad_timestamps;
};

#endif // DATA_TIMED_H
