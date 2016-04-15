/**
 * @file main.cpp
 * @brief Opens and analyzes logfiles with MAVLink data from ardupilot.
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 2014-Apr-18
 *
 *  This file is part of MavLogAnalyzer, Copyright 2014 by Martin Becker.

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

#include <iostream>
#include <list>
#include <string>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "filefun.h"
#include "mainwindow.h"
#include "cmdlineargs.h"
#include "mavlinkparser.h"
#include "mavlinkscenario.h"
#include "dbconnector.h"

using namespace std;

int main(int argc, char *argv[]) {
    cout << "****************************************************************" << endl <<
            "*        MAV Log Analyzer for apm:copter                       *" << endl <<
            "****************************************************************" << endl <<
            " Copyright (C) 2013-2015 M. Becker et.al. <becker@rcs.ei.tum.de>" << endl <<
            " This program comes with ABSOLUTELY NO WARRANTY." << endl <<
            " This is free software, and you are welcome to redistribute it" << endl <<
            " under certain conditions; type `show c' for details." << endl << endl;

    CmdlineArgs args(argc, argv); // parse command line
    if (!args.valid) {
        cout << "Error parsing command line" << endl;
        exit(1);
    }

	if(args.import) {
        // FIXME: that assumes we have only mavlog files. but there are also onboard logs.
        printf("Import\n");
        std::string filename;
        DBConnector::db_props_t dbprops; // FIXME: from command line??
        filename = *args.filenames.begin();
        DBConnector* dbCon = new DBConnector(dbprops);
        //dbCon->importFromFile(filename);
        printf("NOT SUPPORTED");
        delete (dbCon);
    } else {
		// instantiate parsers for all files
		list<MavlinkParser*> parsers;
		for (list<string>::iterator it = args.filenames.begin(); it != args.filenames.end(); ++it) {
		    // TODO: check that no file is added twice...
		    MavlinkParser*tmp = new MavlinkParser(*it);
		    if (tmp->valid) {
		        parsers.push_back(tmp);
		        cout << "Opened file " << *it << "..." <<endl;
		    } else {
		        cout << "Skipping file " << *it << " due to errors" <<endl;
		        delete tmp;
		    }
		}

		if (!args.headless) {
		    // start GUI
		    QApplication a(argc, argv);
		    QFile stylesheet(":/darkorange.stylesheet");
		    stylesheet.open(QIODevice::ReadOnly | QIODevice::Text);
		    QTextStream ts(&stylesheet);
		    a.setStyleSheet(ts.readAll());
		    stylesheet.close();
		    MainWindow w(parsers, &args);
		    w.show();
		    return a.exec();
		} else {
		    /* give a rudimentary overview by putting all mavlink logs into
		     * ONE scenario and then let it dump an overview
		     */
		    MavlinkScenario onescenario(&args);            
		    for (list<MavlinkParser*>::iterator it=parsers.begin(); it != parsers.end(); ++it) {
		        // for each parser, start a new scenario
		        MavlinkScenario*curscenario = new MavlinkScenario(&args);
		        if (curscenario) {
		            mavlink_message_t msg;
		            while ((*it)->get_next_msg(msg)) {
		                curscenario->add_mavlink_message(msg);
		            }
		            // guess the start time of the logfile by looking at the file name
		            //  - basename usually looks like "2014-04-10 15-41-26".
		            {
		                string f_basename = getBasename((*it)->get_filename());
		                string str_datetime = f_basename;
		                // cut off extension from basename if there is one
		                size_t pos = f_basename.rfind('.');
		                if (pos != f_basename.npos) {
		                    str_datetime = str_datetime.substr(0,pos);
		                }
		                uint64_t time_epoch_usec = 0;
		                if (string_to_epoch_usec(str_datetime, time_epoch_usec)) {
		                    curscenario->set_starttime_guess(time_epoch_usec);
		                }
                        if (onescenario.getName().empty()) {
                            onescenario.setName(f_basename);
                        }
                        curscenario->setName(f_basename);
		            }
		            // process scneario from that file
		            curscenario->process();
		            // now merge the current scenario into the ONE
		            onescenario.merge_in(*curscenario);
		            // can forget about current scenario now
		            delete curscenario;
		        } else {
		            cerr << "ERROR allocating a scenario...running out of memory?" << endl;
		        }
		    } // for parser
		    onescenario.process();
		    onescenario.dump_overview(cout);
		}

		// clean parsers
		for (list<MavlinkParser*>::iterator it=parsers.begin(); it != parsers.end(); ++it) {
		    delete *it;
		}
		parsers.clear();
	}
    cout << endl << "BYE!" << endl;
    return 0;
}
