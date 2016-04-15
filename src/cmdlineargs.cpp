/**
 * @file cmdlineargs.cpp
 * @brief Parses command line and keeps data.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 2014-Apr-18
 *
 *  This file is part of MavLogAnalyzer, Copyright 2014 by Martin Becker.

    This program is free software: you can redistribute it and/or modify
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

#include <stdio.h> // FIXME: rem
#include <cstdlib> // atof
#include <getopt.h>
#include "cmdlineargs.h"
#include "filefun.h"

using namespace std;

void CmdlineArgs::_print_usage(FILE * stream) const {
    fprintf(stream, "mavlink_logview [options] <filename>\n");
    fprintf(stream, "Options:\n");
    fprintf(stream,
            "  -n  --headless        start without GUI\n"
            "  -j  --max-time-jumps  define max. allowed time jumps between messages (in seconds, default: 100)\n"
            "  -i  --import          Import file to Database\n"
            "  -h  --help            shows this\n"
            );
}

/**
 * @return 1 if it exists, else 0
 */
bool CmdlineArgs::_file_exists(std::string filename) {
    FILE * file = fopen(filename.c_str(), "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

int CmdlineArgs::_parse(int argc, char**argv) {
    const char *const short_options = "hnj:i"; /* A string listing valid short options letters.  */
    /* An array describing valid long options.  */
    const struct option long_options[] = {
        {"help",           0, NULL, 'h'},
        {"max-time-jumps", 1, NULL, 'j'},
        {"headless",       0, NULL, 'n'},
        {"import",         0, NULL, 'i'},   // Bernd
        {NULL, 0, NULL, 0}             /* Required at end of array.  */
    };

    if (argc < 2) {
        return 0;
    }

    //    char*t = 0;
    int next_option;
    while ((next_option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (next_option) {
        case 'n':
            headless = true;
            break;

        case 'i':
            import = true;
            break;

        case 'h':
            _print_usage(stdout);
            exit (0); // FIXME: it is a bit rude for the caller
            break;

        case 'j':
            {
                double cand = atof(optarg);
                if (cand > 0.) {
                    time_maxjump_sec = cand;
                    printf("max time jumps=%.1lf sec\n", time_maxjump_sec);
                }
            }
            break;

        default:    // null terminator etc
            printf("Unrecognized option: \"%c\" ignored.\n", next_option);
            break;
        }
    }

    if (optind < argc) {
        for (int k=optind; k<argc; k++) {
            // there is a rest pending...only handle filename
            string fname(argv[k]);
            if (!_file_exists(fname)) {
                fprintf(stderr, "Cannot open file: %s\n", fname.c_str());
            } else {
                printf("Logfile=%s\n", fname.c_str());
                filenames.push_back(getFullPath(fname));
            }            
        }
    }
    return 0;
}

CmdlineArgs::CmdlineArgs(int argc, char **argv) : valid(false), headless(false), time_maxjump_sec(100.), import(false){
    if (!_parse(argc, argv)) {
        valid=true;
    }
}
