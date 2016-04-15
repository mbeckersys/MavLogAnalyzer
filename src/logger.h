/**
 * @file logger.h
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

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "logtablemodel.h"
#include "logmsg.h"

class Logger {
public:
    static Logger& Instance(void) {
        static Logger theLog;
        return theLog;
    }

    typedef unsigned long logchannel;

    /************************************
     * FOR THOSE WHO ARE WRITING LOGS
     ************************************/

    /**
     * @brief call this first to create a new channel.
     * @param chname
     * @param to_file if true, a persistent file will be created
     * @return channel to be used with write()
     */
    logchannel createChannel(const std::string & chname, bool keep=false);

    /**
     * @brief add a new log message
     * @param typ type of message
     * @param msg content of message
     * @param ch channel from chreateChannel, or 0 (default) for generic channel     
     */
    void write(logmsgtype_e typ, const std::string & msg, logchannel ch = 0);

    /**
     * @brief drop all messages from given channel
     * @param ch
     */
    void deleteChannel(logchannel ch);

    /**
     * @brief remove a single message from the log
     * @param id the ID of the message
     */
    void deleteMessage(const QModelIndex &id);
    void deleteMessages(const QModelIndexList &lid);

    /************************************
     * FOR THOSE WHO ARE DISPLAYING LOGS
     ************************************/
    /**
     * @brief widgets can obtain the model to show the log messages
     * @return
     */
    LogTableModel*getModel(void) { return &_model; }

private:

    /***********************************
     * TYPEDEFS
     ***********************************/
    /**
     * @brief properties of a channel
     */
    typedef struct channelprops_s {
        std::string name;
        bool has_file;
        std::ofstream*ofile;
    } channelprops_t;

    typedef std::map<logchannel, channelprops_t> channelmap_t;

    /***********************************
     * METHODS
     ***********************************/

    Logger(); // ctor hidden
    Logger(Logger const&); // copy ctor hidden
    Logger& operator=(Logger const&); // assign op. hidden
    ~Logger(); // dtor hidden


    std::ofstream* _createLogfile(std::string filename);
    logchannel _create_channel(channelprops_t & p);
    void _cleanup_stream(std::ofstream*ofs);
    void _cleanup(void);

    /************************************
     * ATTRIBUTES
     ************************************/

    LogTableModel _model; ///< carrying the data, which can be show by tablewidgets
    logchannel _nextid;
    channelmap_t _channels;

    std::vector<std::string> _files;
    std::vector<std::ofstream*> _streams;
};

#endif // LOGGER_H
