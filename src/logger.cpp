/**
 * @file logger.cpp
 * @brief Provides logfile support (create, delete after application shutdown)
 * @author Martin Becker <becker@rcs.ei.tum.de>
 * @date 08.02.2015
 
    This file is part of MavLogAnalyzer, Copyright 2015 by Martin Becker.
    
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

#include <sys/time.h>
#include <sstream>
//#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "logger.h"
#include "filefun.h"

using namespace std;

Logger::Logger() : _nextid(0) {
    // create default channel
    channelprops_t props;
    props.name = "general";
    props.has_file = false;
    (void) _create_channel(props);
}

Logger::logchannel Logger::createChannel(const std::string & chname, bool to_file) {
    channelprops_t props;
    props.name = chname;
    props.has_file = to_file;
    return _create_channel(props);
}

void Logger::deleteMessages(const QModelIndexList &lid) {
    if (lid.empty()) return;


    // filter the list
    QModelIndexList filtered;
    for (QModelIndexList::const_iterator it = lid.begin(); it != lid.end(); ++it) {
        if (it->column() > 0) continue; // because we select full rows..would result in removing X rows when X columns are selected
        filtered.push_back(*it);
    }
    if (filtered.empty()) return;
    int cnt = filtered.last().row() - filtered.first().row() + 1;
    if (cnt < 1) return; // woaha!
    _model.removeRows(filtered.first().row(), cnt, filtered.first().parent());
}

void Logger::deleteMessage(const QModelIndex &id) {
    if (!id.isValid()) return;
    _model.removeRow(id.row(), id.parent());
}

void Logger::deleteChannel(Logger::logchannel ch) {
    // find channel, and remove
    channelmap_t::iterator it = _channels.find(ch);
    if (it != _channels.end()) {
        if (it->second.has_file) {
            it->second.has_file = false;
            _cleanup_stream(it->second.ofile);
            it->second.ofile = NULL;
        }
        // TODO: update model: remove channel.
        _channels.erase(it);
    }
}

Logger::logchannel Logger::_create_channel(Logger::channelprops_t & p) {
    if (p.has_file) {
        p.ofile = _createLogfile(p.name);
    } else {
        p.ofile = NULL;
    }
    logchannel ch = _nextid++;
    _channels[ch] = p;
    return ch;
}

Logger::~Logger() {
    _cleanup();
}

#if 0
void Logger::deleteLogfile(std::ofstream*stream) {
    if (!stream) return;
    unsigned int k=0;
    for (vector<ofstream*>::iterator it = _streams.begin(); it != _streams.end(); ++it, ++k) {
        if (stream == *it) {
            _cleanup_stream(*it);            
            return;
        }
    }
}
#endif

void Logger::write(logmsgtype_e typ, const std::string & msg, logchannel ch) {
    // find channel, then write to there
    // if not found, revert to default channel
    channelmap_t::const_iterator it = _channels.find(ch);
    channelprops_t props;
    if (it != _channels.end()) {
        props = it->second;
    } else {
        props = _channels[0]; // default channel
    }
    std::string emitter = props.name;
    _model.add_message(emitter, typ, msg);
}

/**
 * @brief createLogfile
 * @param filename
 * @return
 */
ofstream* Logger:: _createLogfile(std::string filename) {
    std::string fullname;

    stringstream _fname;
    // gamble filename
    _fname << filename << "_";

    struct timeval now;
    long millitime;
    gettimeofday(&now, NULL);
    millitime = (now.tv_sec * 1000 + now.tv_usec/1000.0);
    _fname << millitime << ".txt";
    fullname = _fname.str();

    ofstream*log = new ofstream;
    // check if it exists
    if (fileExists(fullname)) {
        cerr << "ERROR creating log file (file exists): " << fullname << endl;
        return NULL;
    } else {
        log->open(fullname.c_str());
        cout << "Log file created: " << fullname << endl;        
    }
    usleep(5E-3);// FIXME: because of time resolution, we have to wait here for a while. otherwise we could be asked to create a log of logfiles, and it would oftentimes fail
    return log;
}

#if 0
void Logger::_logfiles_delete(void) {
    for (vector<string>::iterator it = _files.begin(); it != _files.end(); ++it) {
        std::string fname = *it;
        if (remove(fname.c_str())  != 0 ) {
            cerr << "ERROR deleting file: " << fname << endl;
        } else {
            cout << "Log file deleted: " << fname << endl;
        }
    }
}
#endif

void Logger::_cleanup_stream(ofstream*ofs) {
    if (ofs->is_open()) {
        ofs->close();
    }
    delete ofs;
}

void Logger::_cleanup(void) {
    for (channelmap_t::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        deleteChannel(it->first);
    }
    _channels.clear();
}
