/**
 * @file config.h
 * @brief generic settings
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 28.04.2014

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

#ifndef CONFIG_H
#define CONFIG_H

// to suppres warnings for purposely not used parameters (e.g., for polymorphism)
// #define ATTR_UNUSED __attribute__((unused))
// Prefer: Just don't give those parameters a name (or comment the name) in the implementation
// => will work for all compilers and doesn't require macro


// for math: INFINITY and NAN macros
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#endif // CONFIG_H
