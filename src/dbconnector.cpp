/**
 * @file dbconnector.cpp
 * @brief transfers (store, load) data between a MySQL database and MavLinkScenario
 * @author Bernd Kast <kastbernd@gmx.de>, Martin Becker <becker@rcs.ei.tum.de>, TUM RCS, Praktikum Multicopter
 * @date 15.11.2014
  */

#include <vector>
#include <iostream>
#include <inttypes.h>
#include <QMessageBox>
#include "dbconnector.h"
#include "time_fun.h"

using namespace std;

DBConnector::DBConnector(const db_props_t & args) : _args(args), _deferredLoad(true)
{
    _db = QSqlDatabase::addDatabase( "QMYSQL" );
    //std::cout << "Verfügbare Treiber: " << QSqlDatabase::drivers().join(" ").toStdString() << std::endl;

    setDBProperties(args);
}

void DBConnector::setDBProperties(const db_props_t & props) {
    _db.setHostName( QString::fromStdString(props.dbhost) );
    _db.setDatabaseName( QString::fromStdString(props.dbname) );

    _db.setUserName( QString::fromStdString(props.username) );
    _db.setPassword( QString::fromStdString(props.password) );
}

DBConnector::db_props_t DBConnector::getDBProperties(void) const {
    db_props_t ret;

    ret.dbhost = _db.hostName().toStdString();
    ret.dbname = _db.databaseName().toStdString();
    ret.username = _db.userName().toStdString();
    ret.password = _db.password().toStdString();

    return ret;
}

bool DBConnector::selfTest(std::string & errmsg) {
    // FIXME: verify table structure
    struct dbBinder dbBind(&_db);
    if( dbBind.error ) {
        errmsg = dbBind.errmsg;
        return false;
    }
    QSqlQuery qry;
    QString strQuery = "SELECT * FROM scenarios LIMIT 1;";
    qry.prepare( strQuery );
    if( !qry.exec()) {
        errmsg = "Database is reachable, but table structure is incorrect.";
        return false;
    }

    errmsg = "success";
    return true;
}

bool DBConnector::saveScenarioToDB(const MavlinkScenario*const scen, DialogProgressBar*dlg) {
    if (!scen) return false;

    struct dbBinder dbBind(&_db);
    if( dbBind.error ) {
        return false;
    }

    double runtime = get_time_secs();
    save_res_e ret = _saveScenario2DB(*scen, dlg);
    runtime = get_time_secs() - runtime;
    std::cout << "FINISH SAVING TO DB. Time = " << runtime << "s" << flush;
    switch (ret) {
    case SAVE_ERROR:
        std::cerr << "WARNING: had error (" << ret << ") during import from file" << std::endl;
        return false;
        break;
    case SAVE_UPDATED:
        std::cout << "INFO: updated existing scenario" << std::endl;
        break;
    default:
        break;
    }
    return true;
}

/**
 * @brief reads Log to scenario,adapts startingtime and triggers recalculation of generated values
 * @param fileName
 * @param scenario Reference
 * @return 0 if everything was ok<br> <0 if something was wrong
 */
int DBConnector::_getScenarioFromFile(const std::string /*fileName*/, MavlinkScenario &/*scenario*/) {

    // TODO: decide which kind of log; instantiate according parser, then fetch scenario.

#if 0
    MavlinkParser *parser;
    parser = new MavlinkParser(fileName);
    mavlink_message_t msg;

    while(parser->get_next_msg(msg))
    {
        scenario.add_mavlink_message(msg);
    }

    _adaptStartTime(fileName, scenario);

    // process scenario from that file
    scenario.process();
    delete(parser);
    return 0;
#else
    return 1; // not implemented
#endif
}


/**
 * @brief stores scenario with all systems and their Data to the Database
 * @param scenario reference to the object that should be saved
 * @param fileName just nice to know
 * @return see enum definitions
 */
DBConnector::save_res_e DBConnector::_saveScenario2DB(const MavlinkScenario &scenario, DialogProgressBar*dlg) {


    // reject empty scenarios.
    if (scenario._seen_systems.empty()) {
        std::cerr << "ERROR: DB import skipped, because scenario is empty" << std::endl;
        return SAVE_ERROR; // nothing to do
    }

    // make sure the scenario isn't already in DB. Full compare would be nuts, so we use a heuristic: if the starttime is already there, reject.
    std::string tstart = epoch_to_datetime(scenario.get_scenario_starttime_sec(), true);
    bool exists = false;
    QString strsimilar;
    unsigned long long existsID;
    unsigned long n_similar = 0;
    {

        QSqlQuery qry;
        qry.prepare( "SELECT * FROM scenarios WHERE TIME_START=:starttime;" );
        qry.bindValue(":starttime", QString::fromStdString(tstart));


        if( qry.exec() ) {
            n_similar = qry.size();
            if (qry.next()) {
                exists = true;
                existsID = qry.value(qry.record().indexOf("ID")).toULongLong();
                strsimilar = qry.value(qry.record().indexOf("FILENAME")).toString();
            }
        } else {
            std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
            return SAVE_ERROR;
        }
    }
    if (exists) {
        // ask what to do, because a similar scenario seems to exist.


        QMessageBox msg (QMessageBox::Question, "Similar Scenario found", QString("A similar scenario with the name '") + strsimilar + QString("' is already in the database. Do you want to update that scenario with the current one (no), or insert the current one anyway (yes)?"), QMessageBox::Yes|QMessageBox::No);
        msg.setButtonText(QMessageBox::Yes, "Create a new scenario");
        msg.setButtonText(QMessageBox::No, "Overwrite/update existing scenario");

        const bool updateExisting = QMessageBox::No == msg.exec();
        if (updateExisting) {
            if (n_similar == 1) {

                // we update only the scenario description for now. FIXME: one sunny day, also merge in new data...
                std::cout << "INFO: DB already has such a scenario with ID=" << existsID << ", starttime=" << tstart << ". Updating ..."<< endl;
                int ret = _updateScenarioInDB(scenario, existsID);
                if (ret) {
                    std::cerr << "ERROR: Updating scenario with ID=" << existsID << ". Ret = " << ret << endl;
                    return SAVE_ERROR;
                }
                std::cout << "ERROR: Update of scenario with ID=" << existsID << " successful." << endl;
                return SAVE_UPDATED;
            } else {
                std::cerr << "ERROR: More than one similar scenario...cannot update." << endl;
                return SAVE_ERROR;
            }
        } else {
            // nothing to do...we just go on after this check.
        }
    }
    std::cout << "Scenario will be imported, not a duplicate." << endl;

    // create a new scenario entry in the DB
    unsigned long long scenarioID = _insertScenarioToDB(scenario);
    if(scenarioID == 0) {
        std::cerr << "Error occured during saving of MavlinkScenario: "<< scenarioID << std::endl;
        return SAVE_ERROR;
    } else {
        std::map<std::string,double> events;
        std::map<std::string,double> newEvents;
        double maxEventID;
        int success;

        success = _getEventsFromDB(events,maxEventID);
        if(success < 0) {
            std::cerr << "Error occured during select of Events from DB: "<< success << std::endl;
            return SAVE_ERROR;
        }

        // now write each system's data to DB
        unsigned int TOTAL = scenario._seen_systems.size();
        unsigned int cnt=0;
        for (MavlinkScenario::systemlist::const_iterator it = scenario._seen_systems.begin(); it != scenario._seen_systems.end(); ++it) {
            cnt++;
            if (dlg) {
                std::stringstream ss;
                ss << "Save System " << cnt << " of " << TOTAL;
                dlg->setLabel(QString().fromStdString(ss.str()));
                dlg->repaint();
            }
            success = DBConnector::_saveSystem2DB(*it->second, scenarioID,events, newEvents,maxEventID, dlg);
            if(success < 0) {
                std::cerr << "Error occured during saving of MavlinkSystem: "<< success << std::endl;
                return SAVE_ERROR;
            }
        }

        success = _saveEvents2DB(newEvents);
        if(success < 0) {
            std::cerr << "Error occured during saving of Events to DB: "<< success << std::endl;
            return SAVE_ERROR;
        }

    }

    return SAVE_SUCCESS;
}

/**
 * @brief stores system with all Data to the Database
 * @param sys Reference to the current system
 * @param scenarioID current scenarioId
 * @param events already existing events from the db
 * @param newEvents events, that had been unknown before this import and thus have to be stored later to th db
 * @param maxEventID +1 is the next free eventId
 * @return 0 on success<br><0 on error
 */
int DBConnector::_saveSystem2DB(const MavSystem &sys, const int scenarioID, std::map<std::string,double> &events, std::map<std::string,double> &newEvents, double &maxEventID, DialogProgressBar*dlg)
{
    int systemID = _insertSystemToDB(sys, scenarioID);
    int success = 0;
    int ret = 0;
    if(systemID < 0) {
        std::cerr << "Error occured during saving of System: " << systemID << std::endl;
        return -1;
    }

    // this version works flat on the data
    const unsigned int TOTAL = sys._data_from_path.size();
    unsigned int cnt=0;
    unsigned int progress = 0, progress_pre = 0;
    for (MavSystem::data_accessmap::const_iterator it = sys._data_from_path.begin(); it != sys._data_from_path.end(); ++it) {
        // -- progress
        if (dlg) {
            dlg->setValue(cnt, TOTAL);
        } else {
            progress = (int)(cnt*100 / TOTAL);
            if (progress > progress_pre) {
                std::cout << "  ..." << progress << "%" << flush << std::endl;
                progress_pre = progress;
            }
        }
        cnt++;
        // -- end progress
        success = _saveData2DB(*it->second, systemID, events, newEvents, maxEventID);
        if(success < 0) {
            std::cerr << "Error occured during saving of DataGroup: " << success << std::endl;
            ret = -2;
        }
    }
    return ret;
}

/**
 * @brief stores DataGroup with all Data(converted to double) to the Database, only leafs (no nodes) are saved
 * @detail unfortunatly there is an inconsistency in the naming scheme between the c++ data structure and the database:<br>
  <table><tr>
  <td></td><td>C++</td><td>DB</td>
  </tr>
  <tr>
  <td>Node without data</td><td>data</td><td>not stored</td>
  </tr>
  <tr>
  <td>Leaf</td><td>data</td><td>dataGroup</td>
  </tr>
  <tr>
  <td>data within a leaf</td><td>not named - vectors</td><td>data</td>
  </tr>
  </table>
 * @param dat reference to the dataGroup
 * @param systemID current systemId this dataGroup belongs to
 * @param path  current path of this dataGroup within the system
 * @param events already existing events from the db
 * @param newEvents events, that had been unknown before this import and thus have to be stored later to th db
 * @param maxEventID +1 is the next free eventId
 * @return <0 on error <br>0 on success
 */
int DBConnector::_saveData2DB(const Data &dat, const int systemID, std::map<std::string,double> &events, std::map<std::string,double> &newEvents, double &maxEventID) {
    int success = 0;
    int dataGroupID;
    std::string type;
    std::vector <double> data;
    std::vector <double> time;

    //determine type and convert Data to double vector
    success = _convertDataToDoubleVector(&dat, data, time, type, events, newEvents,maxEventID);
    if(success < 0) {
        std::cerr << "Error occured during converting of Data: " << success << std::endl;
        return -1;
    }
    //Insert DataContainer aka DataGroup to DB;
    dataGroupID = _insertDataGroupToDB(dat,systemID, type);
    if(dataGroupID < 0) {
        std::cerr << "Error occured during saving of DataGroup: " << dataGroupID << std::endl;
        return -2;
    }
    // insert the actual data
    success = _insertDataToDB(data, time, dataGroupID);
    if(success < 0) {
        std::cerr << "Error occured during saving of Data: " << success << std::endl;
        return -3;
    }
    return 0;
}

/**
 * @brief inserts unknown string events to the database
 * @param newEvents maps an id to the string of an event
 * @return <0 on error<br>0 on success
 */
int DBConnector::_saveEvents2DB(const std::map<std::string, double> &newEvents) {
    QSqlQuery qry;
    std::stringstream ss;
    ss << std::setprecision(16);
    bool start = true;

    ss<<"INSERT INTO events (ID, EVENT) VALUES";
    for (std::map<std::string, double>::const_iterator it = newEvents.begin(); it != newEvents.end(); ++it) {
        if(!start)
        {
            ss << ",";
        }
        ss << " ("
           <<it->second<<","
           <<"'"<<it->first<<"'"
           <<")";
        start = false;
    }
    ss << ";"; // FIXME: use bindVal()

    if (!start) {
        qry.prepare( QString::fromStdString(ss.str()) );
        if( !qry.exec() ) {
           std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
           return -2;
        }
    }
    return 0;
}

/**
 * @brief convertDataToDoubleVector converts limited set of types of data objects to a list of data, time and returns the type of it
 * @param dat input data object
 * @param data output vector with values
 * @param time output vector with times
 * @param type output type of input object
 * @return <0 on error 0 on success
 */
int DBConnector::_convertDataToDoubleVector(const Data *dat,std::vector <double>& data, std::vector <double>& time, std::string &type, std::map<std::string,double> &events, std::map<std::string,double> &newEvents, double &maxEventID)
{
    /** DataTimeseries*/
    {
        const DataTimeseries<float> *const tsf = dynamic_cast<DataTimeseries<float> const*>(dat);
        if (tsf)
        {
            type = "float_timed";
            _convertTimeSeriesToDoubleVectorTemplate<float>(*tsf, data,time);
            return 0;
        }

        const DataTimeseries<double> * tsd = dynamic_cast<DataTimeseries<double> const*>(dat);
        if (tsd)
        {
            type = "double_timed";
            _convertTimeSeriesToDoubleVectorTemplate<double>(*tsd, data,time);
            return 0;
        }

        const DataTimeseries<unsigned int> * tsu = dynamic_cast<DataTimeseries<unsigned int> const*>(dat);
        if (tsu)
        {
            type = "uint_timed";
            _convertTimeSeriesToDoubleVectorTemplate<unsigned int>(*tsu, data,time);
            return 0;
        }

        const DataTimeseries<int> * tsi = dynamic_cast<DataTimeseries<int> const*>(dat);
        if (tsi)
        {
            type = "int_timed";
            _convertTimeSeriesToDoubleVectorTemplate<int>(*tsi, data,time);
            return 0;
        }
    }

    /** DataUntimed*/
    {
        const DataUntimed<float> *const usf = dynamic_cast<DataUntimed<float> const*>(dat);
        if (usf)
        {
            type = "float_untimed";
            _convertUntimedDataToDoubleVectorTemplate<float>(*usf, data,time);
            return 0;
        }

        const DataUntimed<double> * usd = dynamic_cast<DataUntimed<double> const*>(dat);
        if (usd)
        {
            type = "double_untimed";
            _convertUntimedDataToDoubleVectorTemplate<double>(*usd, data,time);
            return 0;
        }

        const DataUntimed<unsigned int> * usu = dynamic_cast<DataUntimed<unsigned int> const*>(dat);
        if (usu)
        {
            type = "uint_untimed";
            _convertUntimedDataToDoubleVectorTemplate<unsigned int>(*usu, data,time);
            return 0;
        }

        const DataUntimed<int> * usi = dynamic_cast<DataUntimed<int> const*>(dat);
        if (usi)
        {
            type = "int_untimed";
            _convertUntimedDataToDoubleVectorTemplate<int>(*usi, data,time);
            return 0;
        }
    }
    /** DataEvent: push id to data and generate a map id->eventstring*/
    {
        const DataEvent<std::string> *const ess = dynamic_cast<DataEvent<std::string> const*>(dat);
        if (ess) {
            type = "string_event";
            //convertDataEventToDoubleVectorTemplate(*ess, data,time );

            std::vector<std::string> dat = ess->get_data();
            for (std::vector<std::string>::iterator it = dat.begin(); it != dat.end(); ++it) {
                std::map<std::string,double>::iterator it2 = events.find(*it);
                if(it2 != events.end()) {
                   //element found;
                   data.push_back(it2->second);
                } else {
                    maxEventID++;
                    data.push_back(maxEventID);
                    events.insert(std::pair<std::string,double>(*it,maxEventID));
                    newEvents.insert(std::pair<std::string,double>(*it,maxEventID));
                }

            }
            time = ess->get_time();
            return 0;
        }
    }
    // no match...none of the dynamic casts above applied. Polymorphism fail.
    return -1;
}

/**
 * @brief convertTimeSeriesToDoubleVectorTemplate converts timeSeries to a list of data and time and returns the type of it
 * @param dat reference to the data that should be converted
 * @param data double vector containing the data
 * @param time double vector containing the timestamps
 */
template <typename TT>
void DBConnector::_convertTimeSeriesToDoubleVectorTemplate(const DataTimeseries<TT> &dat, std::vector <double> &data,std::vector <double> &time)
{
    for (typename std::vector<TT>::const_iterator it = dat.get_data().begin(); it != dat.get_data().end(); ++it)
    {
        data.push_back((double)*it);
    }
    time = dat.get_time();
}

/**
 * @brief convertUntimedDataToDoubleVectorTemplate converst data Object to a vector of values and times
 * @param dat input Data object
 * @param data  output vector
 * @param time output vector
 */
template <typename UT>
void DBConnector::_convertUntimedDataToDoubleVectorTemplate(const DataUntimed<UT> &dat, std::vector <double> &data, std::vector <double> &time)
{
    data.push_back((double)dat.get_value());
    time.push_back(0.0);
}
/*
template <typename ET>
void DBConnector::convertDataEventToDoubleVectorTemplate(const DataEvent<ET> &dat, std::vector <double> &data, std::vector <double> &time)
{
    //Todo:
}
*/

/**
 * @brief update an existing scenario in the database
 * @param scenario
 * @param existsID the database id of the scenario we want to update
 * @return 0 on success, else error code
 */
int DBConnector::_updateScenarioInDB(const MavlinkScenario &scenario, unsigned long long existsID) {
    QSqlQuery qry;
    qry.prepare("UPDATE scenarios SET DESCRIPTION=:desc, FILENAME=:filename WHERE ID=:id;");
    qry.bindValue(":desc", QString().fromStdString(scenario.getDescription()));
    qry.bindValue(":id", QString().number(existsID));
    qry.bindValue(":filename", QString().fromStdString(scenario.getName()));
    if( !qry.exec()) {
       std::cerr << "_updateScenarioInDB: Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -1;
    }
    return 0;
}

/**
 * @brief inserts Properties of scenario to the DB
 * @param scenario Reference
 * @return ID if everything was ok <0 if something was wrong
 */
int DBConnector::_insertScenarioToDB(const MavlinkScenario &scenario)
{

    /* Members of Scenario:
    const CmdlineArgs*_args;                                                => probably shouldn't be saved, but use settings at time of import
        CmdlineArgs(int argc, char**argv);

    unsigned int _n_msgs; ///< how many messages we processed               =>don't add, but check if is correct after importing
    unsigned int _n_ignored; ///< how many messages we did not process      =>not needed

    // scenario data
    typedef std::map <uint8_t, MavSystem*> systemlist;                      =>stored by insertSystem
    systemlist _seen_systems; ///< list of all seen systems (MAVs, GCSs...) =>restored by selectSystem
    uint64_t _time_guess_epoch_usec; ///< guess when the scenario started   =>to be stored

    =>

    ID                          uint
    _time_guess_epoch_usec      uint        //Date/Time?!
    Description                 string

    */

    QSqlQuery qry;
    double scenarioID;
    std::stringstream ss;
    ss << std::setprecision(16);

    ss << "INSERT INTO scenarios (TIME_START, DESCRIPTION, FILENAME) VALUES (:timestart, :desc, :filename);";
    qry.prepare( QString::fromStdString(ss.str()) );
    QString tstart = QString().fromStdString(epoch_to_datetime(scenario.get_scenario_starttime_sec(),true));
    qry.bindValue(":timestart", tstart);
    qry.bindValue(":desc", QString().fromStdString(scenario.getDescription()));
    qry.bindValue(":filename", QString().fromStdString(scenario.getName()));

    if( !qry.exec()) {
       std::cerr << "_insertScenarioToDB: Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -2;
    }
    qry.prepare("select LAST_INSERT_ID();");
    if( !qry.exec()) {
       std::cerr << "_insertScenarioToDB: Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -3;
    }
    qry.next();
    scenarioID = qry.value(0).toUInt();
    std::cout << "Scenario done: " << scenarioID<< std::endl;

    return scenarioID;
}

/**
 * @brief inserts Properties of the system to the DB
 * @param system Reference
 * @param scenarioID id of the scenario this system belongs to
 * @return ID if everything was ok<br><0 if something was wrong
 */
unsigned long long DBConnector::_insertSystemToDB(const MavSystem &sys, const int scenarioID)
{
 //   std::cout << "insertSystemToDB" << sys.id<<std::endl;
    /*
    unsigned int id;            ///< system id                                              =>store
    unsigned int mavtype;       ///< enum MAV_TYPE                                          =>store
    std::string mavtype_str;    ///< enum MAV_TYPE as human-readable string                 =>store
    bool has_been_armed;        ///< if true, then at some point the system was armed       =>store
    typedef std::map<std::string, Data*> data_accessmap;                                    =>don't store
    data_accessmap data_from_path; ///< all data is stored flat in here, but accessing it should be done via mav_data_groups, where we have associative arrays


    double _time;                                                                           =>store
    bool   _time_valid; ///< indicates whether _time carries actual data                    =>store
    double _time_min;                                                                       =>store
    double _time_max;                                                                       =>store
    double _time_maxjump;                                                                   =>store
    bool   _have_time_update;                                                               =>store

    unsigned int _link_throughput_bytes; ///< this collects the number of bytes to be       =>?

    // more data (time series, ...)
    DataGroup::groupmap mav_data_groups;  ///< hierarchy for data...you can browse through  =>don't
 //data with associative array (map)

    // for managing the time
    typedef std::pair<uint64_t, uint64_t> timeoffset_pair;
    std::vector<timeoffset_pair > _time_offset_raw; ///< pair <relative time, epoch>.       =>store
//Used to find the relation between relative time (all data in this class) and unix time
    uint64_t _time_offset_usec; ///< add this to relative times in data, and you get unix time  =>store
    uint64_t _time_offset_guess_usec;                                                       =>store
    unsigned int ID
    unsigned int scenario ID
| Field                  | Type             | Null | Key | Default | Extra          |
+------------------------+------------------+------+-----+---------+----------------+
| ID                     | int(10) unsigned | NO   | PRI | NULL    | auto_increment |
| SCENARIO_ID            | int(10) unsigned | YES  |     | NULL    |                |
| SYSTEM_ID              | int(10) unsigned | YES  |     | NULL    |                |
| MAVTYPE                | int(10) unsigned | YES  |     | NULL    |                |
| MAVTYPE_STRING         | char(254)        | YES  |     | NULL    |                |
| APTYPE                 | smallint 5u      | YES  |     | NULL    |                |
| APTYPE_STRING          | char(254)        | YES  |     | NULL    |                |
| ARMED                  | tinyint(4)       | YES  |     | NULL    |                |
| TIME                   | double           | YES  |     | NULL    |                |
| TIME_VALID             | tinyint(4)       | YES  |     | NULL    |                |
| TIME_MIN               | double           | YES  |     | NULL    |                |
| TIME_MAX               | double           | YES  |     | NULL    |                |
| TIME_OFFSET_USEC       | bigint(20)       | YES  |     | NULL    |                |
| TIME_OFFSET_GUESS_USEC | bigint(20)       | YES  |     | NULL    |                |
+------------------------+------------------+------+-----+---------+----------------+


*/

    QSqlQuery qry;
    qry.prepare("INSERT INTO systems SET "
                "SCENARIO_ID=:sc_id, SYSTEM_ID=:sys_id, " // 1
                "APTYPE=:aptype, APTYPE_STRING=:aptypestr, MAVTYPE=:mavtype, MAVTYPE_STRING=:mavtypestr," // 2
                "ARMED=:armed, TIME=:time, TIME_VALID=:time_valid, TIME_MIN=:time_min, TIME_MAX=:time_max," // 3
                "TIME_OFFSET_USEC=:time_off_usec, TIME_OFFSET_GUESS_USEC=:time_off_usec_guess;"); // 4
    // 1:
    qry.bindValue(":sc_id", scenarioID);
    qry.bindValue(":sys_id", sys.id);
    // 2:
    qry.bindValue(":aptype", sys.aptype);
    qry.bindValue(":aptype_str", QString::fromStdString(sys.aptype_str));
    qry.bindValue(":mavtype", sys.mavtype);
    qry.bindValue(":mavtype_str", QString::fromStdString(sys.mavtype_str));
    // 3:
    qry.bindValue(":armed", sys.has_been_armed);
    qry.bindValue(":time", sys._time);
    qry.bindValue(":time_valid", sys._time_valid);
    qry.bindValue(":time_min", sys._time_min);
    qry.bindValue(":time_max", sys._time_max);
    // 4:
    qry.bindValue(":time_off_usec", (qulonglong)sys._time_offset_usec);
    qry.bindValue(":time_off_usec_guess", (qulonglong)sys._time_offset_guess_usec);

    if( !qry.exec() ) {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -2;
    }
    qry.prepare("select LAST_INSERT_ID();");
    if( !qry.exec() ) {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -3;
    }
    qry.next();
    unsigned long long systemID = qry.value(0).toULongLong();
    std::cout << "DB: added system " << sys.aptype_str << ", ID=" << systemID<< std::endl;
    return systemID;
}

/**
 * @brief inserts dataGroup and its properties to the DB
 * @param dat Object that contains (most of) the data of the dataGrouop
 * @param systemID systemId this dataGroup belongs to
 * @param type  type of the data this dataGroup stores (for example double_timeseries, string_event)
 * @return ID if everything was ok<br> <0 if something was wrong
 */
int DBConnector::_insertDataGroupToDB(const Data &dat, const int systemID,const std::string type) {

    /*
    unsigned int        _id; ///< unique ID that every class instance is assigned               =>store
    bool                _valid;                                                                 =>store
    std::string         _name;                                                                  =>store
    data_classifier_e   _class;                                                                 =>Data should only be stored if 0
    static unsigned int _autoincrement; ///< every data class get's a unique ID here.           =>store
    unsigned long       _time_epoch_datastart_usec; ///< absolute time when the data starts     =>store
    std::string         _units;                                                                 =>store
    //Event/Timed
    unsigned int _n; ///< number of events stored                                               =>don't
    std::vector<T>      _elems_data;    ///< only used if keepitems=true                        =>stored by vector
    std::vector<double> _elems_time;    ///< only used if keepitems=true                        =>stored by vector

    unsigned int    _n;
    bool            _keepitems;
    string          type
    unsigned int    dataGroupID/systemid
    unsigned int    dataID
DONT SAVE - SHOULD BE GENERATED
    double          _sum;
    double          _sqsum;
    T               _max;
    T               _min;
    double          _max_t;
    double          _min_t;
    bool            _max_valid;
    bool            _min_valid;
+----------------------+---------------------+------+-----+---------+----------------+
| Field                | Type                | Null | Key | Default | Extra          |
+----------------------+---------------------+------+-----+---------+----------------+
| ID                   | int(10) unsigned    | NO   | PRI | NULL    | auto_increment |
| SYSTEM_ID            | int(10) unsigned    | YES  |     | NULL    |                |
| VALID                | tinyint(4)          | YES  |     | NULL    |                |
| NAME                 | char(254)           | YES  |     | NULL    |                |
| FULLPATH             | text                | YES  |     | NULL    |                |
| CLASSIFIER           | int(11)             | YES  |     | NULL    |                | 0: raw data, 1: derived/computed data based on raw
| TIME_EPOCH_DATASTART | bigint(20) unsigned | YES  |     | NULL    |                |
| UNITS                | char(254)           | YES  |     | NULL    |                |
| TYPE                 | char(254)           | YES  |     | NULL    |                |
+----------------------+---------------------+------+-----+---------+----------------+

*/
    QSqlQuery qry;
    std::stringstream ss;
    ss << std::setprecision(16);
    double dataGroupID;

    ss << "INSERT INTO dataGroups (SYSTEM_ID,VALID,NAME,FULLPATH,CLASSIFIER,TIME_EPOCH_DATASTART,UNITS,TYPE) VALUES ("
       <<systemID<<","       
       <<dat._valid<<","
       <<"'"<<dat._name<<"',"
       <<"'"<<Data::get_fullname(&dat)<<"',"
       <<dat._class<<","
       <<dat._time_epoch_datastart_usec<<","
       <<"'"<<dat._units<<"',"
       <<"'"<<type<<"'"
       <<");";
    qry.prepare( QString::fromStdString(ss.str()) ); // FIXME: use bindVal()

    if( !qry.exec() ) {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -2;
    }
    qry.prepare("select LAST_INSERT_ID();");
    if( !qry.exec() ) {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -3;
    }
    qry.next();
    dataGroupID = qry.value(0).toUInt();
    return dataGroupID;
}

/**
 * @brief inserts double vector to the DB
 * @param data vector with data to be inserted to the db
 * @param time timestamps for that data
 * @param dataGroupID dataGroupId this data belongs to
 * @return ID if everything was ok<br> <0 if something was wrong
 *
 * This function is highly optimized...don't mess with it!
 */
int DBConnector::_insertDataToDB(const std::vector <double> &data, const std::vector <double> &time, const int dataGroupID) {
/*
+---------+------------------+------+-----+---------+----------------+
| Field   | Type             | Null | Key | Default | Extra          |
+---------+------------------+------+-----+---------+----------------+
| ID      | int(10) unsigned | NO   | PRI | NULL    | auto_increment |
| DATA_ID | int(10) unsigned | YES  |     | NULL    |                |
| TIME    | double           | YES  |     | NULL    |                |
| VALUE   | double           | YES  |     | NULL    |                |
+---------+------------------+------+-----+---------+----------------+

  */    
    if( data.size() != time.size() ) {
        std::cerr << "Fehler: Anzahl an Werten stimmt nicht mit Zeiten überein!" << std::endl;
        return -2;
    }
    if (data.empty()) return 0; // nothing to do

    _db.transaction(); // also helps speed
    QSqlQuery qry;

    std::stringstream ssqry, ssdataGroupID;
    ssqry << std::setprecision(16);

    // speedup: turn datagroup in string exactly once (saves the int-to-string conversion every time)
    ssdataGroupID << dataGroupID;
    const std::string strdataGroupID = ssdataGroupID.str();

    // value differencing for speedup
    std::stringstream ssvalue;
    ssvalue << std::setprecision(16);
    double lastVal;
    bool first = true;
    // ---

    std::vector<double>::const_iterator itd = data.begin();
    std::vector<double>::const_iterator itt = time.begin();
    unsigned int chunk = 0;
    const unsigned int CHUNKSIZE=5000;
    bool ret;
    for (/*above*/; itd != data.end(); /* IN LOOP */) {
        if(chunk != 0) {
            ssqry << ",";
        } else {            
            ssqry.str(std::string()); // CLEAR
            ssqry << "INSERT INTO data (DATAGROUP_ID,TIME,VALUE) VALUES ";
        }

        // differencing: only convert value to string, if value changed
        const double val = *itd;

        if (itd == data.begin() || first || val != lastVal) {
            ssvalue.str(std::string()); // CLEAR
            ssvalue << val;
            lastVal = val;
        }
        ssqry << "(" << strdataGroupID << "," << *itt << "," << ssvalue.str() << ")";

        // LOOP INCREMENT
        ++itd; ++itt;

        // we split the INSERT into smaller queries to aid performance and to not overload the DB
        if (++chunk == CHUNKSIZE || itd == data.end()) {
            ssqry << ";";
            qry.prepare( QString::fromStdString(ssqry.str()) ); // FIXME: use bindVal()
            ret = qry.exec();
            if( !ret ) {
               std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
               _db.rollback();
               return -3;
            }
            chunk = 0;
        }
        if (first) {
            first = false;
        }
    }    
    _db.commit(); // also helps speed

    return 0;
}


/**
* @brief selects all existing events from the db and stores it to a map
* @param events maps id of an event to an event string (returned)
* @param maxEventID +1 is the next free eventId
* @return <0 on error<br>0 on success
*/
int DBConnector::_getEventsFromDB(std::map<std::string, double> &events, double &maxEventID) {
    QSqlQuery qry;
    qry.prepare("SELECT * FROM events ORDER BY ID asc;");
    if( !qry.exec() )
    {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -2;
    }
    else
    {
        while(qry.next())
        {
            double id = qry.value(qry.record().indexOf("ID")).toDouble();
            std::string event = qry.value(qry.record().indexOf("EVENT")).toString().toStdString();
            events[event] = id;
            maxEventID = id;
        }
    }
    return 0;
}

/**
 * @brief put a single data row from database into the data class object
 * @param sys
 * @param qry already-executed query
 * @return rue on success, else false
 */
bool DBConnector::_populateDataItem(double time, double value, const std::map<double,std::string> & events, Data*data) {
    if (!data) return false;

    // walk through data types
    DataTimeseries<float> * dataFT = dynamic_cast<DataTimeseries<float>* >(data);
    if(dataFT) {
        dataFT->add_elem((float)value,time);
        return true;
    }
    DataTimeseries<double> *dataDT = dynamic_cast<DataTimeseries<double> *>(data);
    if (dataDT) {
        dataDT->add_elem(value,time);
        return true;
    }
    DataTimeseries<unsigned int> *dataUT = dynamic_cast<DataTimeseries<unsigned int> *>(data);
    if(dataUT) {
        dataUT->add_elem((unsigned int)value,time);
        return true;
    }
    DataTimeseries<int> *dataIT = dynamic_cast<DataTimeseries<int> *>(data);
    if(dataIT) {
        dataIT->add_elem((int)value,time);
        return true;
    }
    DataUntimed<float> *dataFU = dynamic_cast<DataUntimed<float> *>(data);
    if(dataFU) {
        dataFU->add_elem((float)value);
        return true;
    }
    DataUntimed<double> *dataDU = dynamic_cast<DataUntimed<double> *>(data);
    if (dataDU) {
        dataDU->add_elem(value);
        return true;
    }
    DataUntimed<unsigned int> *dataUU = dynamic_cast<DataUntimed<unsigned int> *>(data);
    if (dataUU) {
        dataUU->add_elem((unsigned int)value);
        return true;
    }
    DataUntimed<int> *dataUI  = dynamic_cast<DataUntimed<int> *>(data);
    if (dataUI) {
        dataUI->add_elem((int)value);
        return true;
    }
    DataEvent<std::string> *dataSE = dynamic_cast<DataEvent<std::string> *>(data);
    if (dataSE) {
        std::map<double, std::string>::const_iterator it = events.find(value);
        if(it != events.end()) {
            dataSE->add_elem(it->second,time);
            return true;
        } else {
            std::cerr << "Error during import of DataEvent: EventID not Found in events Database"<<std::endl;
            return false;
        }
    }

    //const int indexTYPE = qry.record().indexOf("TYPE");
    //const std::string t = qry.value(indexTYPE).toString().toStdString();
    std::cerr << "_populateDataItem: Error during import: unhandeled type" << std::endl;
    return false;
}

/**
 * @brief get or create a datagroup within a mav system
 * @param sys the system where teh data group is
 * @param type
 * @param path
 * @param units
 * @return pointer to data group in mav system, or NULL on error
 */
Data* DBConnector::_fetchDataGroup(MavSystem*sys, const std::string& type, const std::string& path, const std::string& units) {
    Data*data = NULL;

    if(type == "float_timed") {
        data = sys->_get_and_possibly_create_data< DataTimeseries<float> > (path, units);
    } else if(type == "double_timed") {
        data = sys->_get_and_possibly_create_data< DataTimeseries<double> > (path, units);
    } else if(type == "uint_timed") {
        data = sys->_get_and_possibly_create_data< DataTimeseries<unsigned int> > (path, units);
    } else if(type == "int_timed") {
        data = sys->_get_and_possibly_create_data< DataTimeseries<int> > (path, units);
    } else if(type == "float_untimed") {
        data = sys->_get_and_possibly_create_data< DataUntimed<float> > (path, units);
    } else if(type == "double_untimed") {
        data = sys->_get_and_possibly_create_data< DataUntimed<double> > (path, units);
    } else if(type == "uint_untimed") {
        data = sys->_get_and_possibly_create_data< DataUntimed<unsigned int> > (path, units);
    } else if(type == "int_untimed") {
        data = sys->_get_and_possibly_create_data< DataUntimed<int> > (path, units);
    } else if(type == "string_event") {
        data = sys->_get_and_possibly_create_data< DataEvent<std::string> > (path, units);
    }
    if (!data) {  /* because there could be something with same name but different type */
        std::cerr << "ERROR writing to data " << path << " with type=" << type <<". Is there a type mismatch?" << std::endl;
        return NULL;
    }
    return data;
}


int DBConnector::loadScenarioFromDB(const int id, MavlinkScenario &scenario, DialogProgressBar*progress) {
    struct dbBinder dbBind(&_db);
    if( dbBind.error ) {
        return -1;
    }
    return _loadScenarioFromDB(id, scenario, progress);
}

/**
 * @brief fetch information about a system from database, and populate the class object
 * @param sys the class object to populate
 * @param qry the already-executed query holding the informatin about the MAV system
 */
bool DBConnector::_populateSystem(MavSystem* sys, QSqlQuery& qry) {
    if (!sys) return false;
    // populate system class instance. FIXME: modularize:
    sys->deferredLoad = _deferredLoad;
    sys->_dbid = qry.value(qry.record().indexOf("ID")).toString().toULongLong();
    sys->mavtype_str = qry.value(qry.record().indexOf("MAVTYPE_STRING")).toString().toStdString();
    sys->aptype_str = qry.value(qry.record().indexOf("APTYPE_STRING")).toString().toStdString();
    sys->has_been_armed = qry.value(qry.record().indexOf("ARMED")).toBool();
    sys->mavtype = qry.value(qry.record().indexOf("MAVTYPE")).toUInt();
    sys->_time_offset_usec = qry.value(qry.record().indexOf("TIME_OFFSET_USEC")).toULongLong();
    sys->_time_offset_guess_usec = qry.value(qry.record().indexOf("TIME_OFFSET_GUESS_USEC")).toULongLong(); // IS required, because of scenario.process()
    sys->_time_min = qry.value(qry.record().indexOf("TIME_MIN")).toDouble();
    sys->_time_max = qry.value(qry.record().indexOf("TIME_MAX")).toDouble();
    // not needed:  sys->_time_offset_guess_usec, sys->_time_offset_raw, sys->_have_time_update, sys->_time_maxjump
    return true;
}

bool DBConnector::_populateScenario(MavlinkScenario&scenario, QSqlQuery& qry) {
    scenario.setDescription(qry.value(qry.record().indexOf("DESCRIPTION")).toString().toStdString());
    scenario.setName(qry.value(qry.record().indexOf("FILENAME")).toString().toStdString());
    scenario.setDatabaseID(qry.value(qry.record().indexOf("ID")).toULongLong());
    return true;
}


/**
 * @brief mark data to be loaded later
 * @param d the data row
 * @param datagroup_id the ID in table dataGroups
 * @return true on success, else false
 */
bool DBConnector::_populateDataGroup_deferred(MavSystem*/*sys*/, Data*d, unsigned long long datagroup_id) {
    d->set_deferred(datagroup_id);
    return true;
}

int maybe_later_useful() {
#if 0
    QSqlQuery qry;
    qry.prepare("SELECT * FROM data WHERE DATAGROUP_ID=:did;");
    qry.bindValue(":did", datagroup_id);
    if( !qry.exec() ) {
        std::cerr << "Error occured during execution of data query: "<< qry.lastError().text().toStdString() << std::endl;
        return false;
    }
    cout << "processing..." << flush;
    while (qry.next()) {
        if (!_populateDataItem(sys, qry, events, d)) {
            std::cerr << "Error occured during populating data of row " << d->get_name() << ": "<< qry.lastError().text().toStdString() << std::endl;
            return false;
        }
    }
    cout << "done." << endl << flush;
#endif
    return 0;

}


bool DBConnector::loadDataGroup(Data*d, DialogProgressBar*dlgprogress) {
    if (!d) return false;
    struct dbBinder dbBind(&_db);
    if( dbBind.error ) {
        return false;
    };

    QSqlQuery qry;
    qry.setForwardOnly(true);

    /***********************************************
     * need the event map only if data type is event
     ***********************************************/
    std::map<double,std::string> revents;
    if (dynamic_cast<DataEvent<std::string> *>(d)) {
        cerr << "Data is of type EVENT...fetching event map." << endl;
        if (_getReverseEventsFromDB(revents)) {
            cerr << "ERROR fetching event map from database." << std::endl;
            return -1;
        }
    }

    /*****************************
     * fetch data of the group
     *****************************/
    const long long datagroupID = d->_dbid;
    double runtime = get_time_secs();
    qry.prepare("SELECT * from data WHERE DATAGROUP_ID=:did;");
    qry.bindValue(":did", datagroupID);
    if (!qry.exec()) {
        cerr << "Could not retrieve data for datagroup " << d->get_name() << " from database" << endl;
        return false;
    }

    const unsigned int indexTIME = qry.record().indexOf("TIME");
    const unsigned int indexVALUE = qry.record().indexOf("VALUE");

    unsigned long cnt=0;
    unsigned int progress = 0, progress_pre = 0;
    const unsigned long TOTAL = qry.size();
    while (qry.next()) {
        // update progress
        progress = (int)(cnt*100 / TOTAL);
        if ((progress > progress_pre) || (cnt==0)) {
            if (dlgprogress) {
                dlgprogress->setValue(progress, 100);
            } else {
                cout << "  ..." << progress << "%" << flush << std::endl;
                progress_pre = progress;
            }
        }
        cnt++;
        // --
        const double time = qry.value(indexTIME).toDouble();
        const double value = qry.value(indexVALUE).toDouble();
        if (!_populateDataItem(time, value, revents, d)) {
            cerr << "ERROR populating data item " << d->get_name() << std::endl;
            continue;
        }
    }
    runtime = get_time_secs() - runtime;
    cout << d->get_name() << ": " << cnt << " rows fetched in "<< runtime << "s" << endl;

    return true;
}

bool DBConnector::loadDataGroup(MavSystem*sys, unsigned long long datagroupID, DialogProgressBar*dlgprogress) {
    if (!sys) return false;
    struct dbBinder dbBind(&_db);
    if( dbBind.error ) {
        return false;
    };

    /********************************************
     * fetch details of the group and create it
     ********************************************/
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.prepare("SELECT * from dataGroups WHERE ID=:did LIMIT 1;");
    qry.bindValue(":did", datagroupID);
    if (!qry.exec()) {
        return false;
    }
    if (!qry.first()) {
        cerr << "Could not retrieve datagroup #" << datagroupID << " for system " << sys->id << " from database" << endl;
        return false;
    }

    const unsigned int idxPATH = qry.record().indexOf("FULLPATH");
    const unsigned int idxTYPE = qry.record().indexOf("TYPE");
    const unsigned int idxUNITS = qry.record().indexOf("UNITS");
    const unsigned int idxTIMESTART = qry.record().indexOf("TIME_EPOCH_DATASTART");
    const string path =  qry.value(idxPATH).toString().toStdString();
    const string type =  qry.value(idxTYPE).toString().toStdString();
    const string units =  qry.value(idxUNITS).toString().toStdString();
    const uint64_t timeEpochDatastart = qry.value(idxTIMESTART).toULongLong();
    qry.finish();

    Data*d = _fetchDataGroup(sys, type, path, units);
    if (!d) {
        cerr << "Could not get/create datagroup " << path << " for system " << sys->id << endl;
        return false;
    }
    d->set_epoch_datastart(timeEpochDatastart);


    /********************************************
     * populate the group
     ********************************************/
    return loadDataGroup(d, dlgprogress);

#if 0
    /***********************************************
     * need the event map only if data type is event
     ***********************************************/
    std::map<double,std::string> revents;
    if (dynamic_cast<DataEvent<std::string> *>(d)) {
        cerr << "Data is of type EVENT...fetching event map." << endl;
        if (_getReverseEventsFromDB(revents)) {
            cerr << "ERROR fetching event map from database." << std::endl;
            return -1;
        }
    }

    /*****************************
     * fetch data of the group
     *****************************/
    double runtime = get_time_secs();
    const unsigned int indexTIME = qry.record().indexOf("TIME");
    const unsigned int indexVALUE = qry.record().indexOf("VALUE");
    qry.prepare("SELECT * from data WHERE DATAGROUP_ID=:did;");
    qry.bindValue(":did", datagroupID);
    if (!qry.exec()) {
        cerr << "Could not retrieve data for datagroup " << path << " and system " << sys->id << " from database" << endl;
        return false;
    }
    unsigned long cnt=0;
    unsigned int progress = 0, progress_pre = 0;
    const unsigned long TOTAL = qry.size();
    while (qry.next()) {
        // update progress
        progress = (int)(cnt*100 / TOTAL);
        if ((progress > progress_pre) || (cnt==0)) {
            if (dlgprogress) {
                dlgprogress->setValue(progress, 100);
            } else {
                cout << "  ..." << progress << "%" << flush << std::endl;
                progress_pre = progress;
            }
        }
        cnt++;
        // --
        const double time = qry.value(indexTIME).toDouble();
        const double value = qry.value(indexVALUE).toDouble();
        if (!_populateDataItem(sys, time, value, revents, d)) {
            cerr << "ERROR populating data item " << path << " for MAV system #" << sys->id << std::endl;
            continue;
        }
    }
    runtime = get_time_secs() - runtime;
    cout << path << ": " << cnt << " rows fetched in "<< runtime << "s" << endl;

    return true;
#endif
}

/**
 * @brief get data for a specific datagroup from database
 * @param d
 * @param qry2 already-executed query holding a join of dataGroups and data belonging to sys
 * @return tue on success, else false
 */
bool DBConnector::_populateAllDataGroups_immediate(MavSystem*sys, const std::map<double,std::string>& events) {
    if (!sys) return false;

    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.prepare("SELECT * from dataGroups INNER JOIN data on data.DATAGROUP_ID=dataGroups.ID where dataGroups.SYSTEM_ID=:sid;");
    qry.bindValue(":sid", (qulonglong)sys->_dbid);
    if (!qry.exec()) {
        return false;
    }
    cout << "Fetched " << qry.size() << " data rows for system #" << sys->id << endl;

    const unsigned int idxPATH = qry.record().indexOf("FULLPATH");
    const unsigned int idxTYPE = qry.record().indexOf("TYPE");
    const unsigned int idxUNITS = qry.record().indexOf("UNITS");
    const unsigned int idxTIMESTART = qry.record().indexOf("TIME_EPOCH_DATASTART");
    const unsigned int idxGROUPID = qry.record().indexOf("DATAGROUP_ID"); // in data
    const unsigned int indexTIME = qry.record().indexOf("TIME");
    const unsigned int indexVALUE = qry.record().indexOf("VALUE");

    unsigned long long last_datagroup_id=0;
    unsigned long nrec=0;

    Data*d = NULL;
    while (qry.next()) {
        const unsigned long long datagroup_id = qry.value(idxGROUPID).toULongLong();
        string path =  qry.value(idxPATH).toString().toStdString();
        if ((nrec==0) || ( datagroup_id != last_datagroup_id)) {
            const string type =  qry.value(idxTYPE).toString().toStdString();
            const string units =  qry.value(idxUNITS).toString().toStdString();
            const uint64_t timeEpochDatastart = qry.value(idxTIMESTART).toULongLong();
            cout << "Loading group " << path << "..." << endl;
            d = _fetchDataGroup(sys, type, path, units);
            if (!d) {
                cerr << "ERROR getting/creating data group " << path << " for MAV system #" << sys->id << std::endl;
                continue;
            }
            d->set_epoch_datastart(timeEpochDatastart);
            last_datagroup_id = datagroup_id;
        }
        if (!d) continue;

        const double time = qry.value(indexTIME).toDouble();
        const double value = qry.value(indexVALUE).toDouble();
        if (!_populateDataItem(time, value, events, d)) {
            cerr << "ERROR populating data item " << path << " for MAV system #" << sys->id << std::endl;
            continue;
        }
        nrec++;
    }
    cout << nrec << " records done." << endl << flush;
    return true;
}


/**
 * @brief get the event table from the databse and reverse it: yields a map double -> string
 * @param events
 * @return 0 on success, else error code
 */
int DBConnector::_getReverseEventsFromDB(std::map<double,std::string> &events) {
    // need events
    double maxEventID;
    std::map<std::string, double> eventsTmp;

    int success = _getEventsFromDB(eventsTmp,maxEventID);
    if(success < 0) {
        std::cerr << "WARNING: error occured during select of Events from DB: "<< success << std::endl;
        return -1;
    }
    //rebuild map TODO: what is happening here????
    for(std::map<std::string,double>::const_iterator it = eventsTmp.begin(); it != eventsTmp.end(); ++it) {
        events.insert(std::pair<double,std::string>(it->second,it->first));
    }
    return 0;
}

/**
 * @brief populates a MavSystem's datagroups with rows from the database
 * @param sys the system fo fill
 * @param qry2 the already-excuted query containing the datagroups
 * @param deferred if true, lazy loading is active, otherwise full load
 * @return number of groups fetched
 */
int DBConnector::_populateDataGroups(MavSystem*sys, QSqlQuery& qry2) {
    if (!sys) return -1;

    std::map<double,std::string> events;
    if (!_deferredLoad) {
        if (_getReverseEventsFromDB(events)) {
            cerr << "ERROR fetching event map from database." << std::endl;
            return -1;
        }
    }

    unsigned int ngrps = 0;
    if (_deferredLoad) {
        while (qry2.next()) { // for all groups...
            const string path =  qry2.value(qry2.record().indexOf("FULLPATH")).toString().toStdString();
            const string type =  qry2.value(qry2.record().indexOf("TYPE")).toString().toStdString();
            const string units =  qry2.value(qry2.record().indexOf("UNITS")).toString().toStdString();            
            const uint64_t timeEpochDatastart = qry2.value(qry2.record().indexOf("TIME_EPOCH_DATASTART")).toULongLong();
            const unsigned long long datagroup_id = qry2.value(qry2.record().indexOf("ID")).toULongLong();
            //const int classifier =  qry2.value(qry2.record().indexOf("CLASSIFIER")).toInt();
            //if (((Data::data_classifier_e) classifier) != Data::DATA_RAW) continue; // skip computed data, only show raw

            cout << "Loading group " << path << "..." << endl;

            Data*d = _fetchDataGroup(sys, type, path, units);
            if (!d) {
                cerr << "ERROR getting/creating data group " << path << " for MAV system #" << sys->id << std::endl;
                continue;
            }
            d->set_epoch_datastart(timeEpochDatastart);            

            if (!_populateDataGroup_deferred(sys, d, datagroup_id)) {
                cerr << "ERROR: could not set up lazy load for data \"" << d->get_name() << "\"" << endl;
            }
            ngrps++;
        }
    } else {
        // immediate load -> combine all datagroups into one query, which is much faster than iterating through groups
        if (!_populateAllDataGroups_immediate(sys, events)) {
            cerr << "ERROR: could not load data for system #" << sys->id << endl;
        }
    }
    return ngrps;
}

/**
 * @brief called by loadScenarioFromDB. Faster, because uses lazy loading.
 */
int DBConnector::_loadScenarioFromDB(const int id, MavlinkScenario &scenario, DialogProgressBar*dlgprogress) {    

    /*****************************
     * LOAD SCENARIO INFO
     *****************************/    
    QSqlQuery qry, qry2;
    qry.prepare("SELECT * FROM scenarios WHERE ID=:id LIMIT 1;");
    qry.bindValue(":id", id);
    if( !qry.exec() ) {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -3;
    }
    if (qry.next()) {
        if (!_populateScenario(scenario, qry)) {
            std::cerr << "Error populating scenario" << std::endl;
        }
    } else {
        std::cerr << "Error occured during select on scenarios" << std::endl;
        return -4;
    }

    double runtime = get_time_secs();
    std::cout << "START FETCHING FROM DB..." << flush;

    /*******************************
     * FETCH ALL SYSTEMS IN SCENARIO
     *******************************/
    qry.setForwardOnly(true);
    qry.prepare("SELECT * FROM systems WHERE SCENARIO_ID=:id;");
    qry.bindValue(":id", id);
    if( !qry.exec() ) {
       std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
       return -3;
    }
    unsigned int cnt=0;
    unsigned int progress = 0, progress_pre = 0;
    const unsigned int TOTAL = qry.size();
    while (qry.next()) {
        // update progress
        progress = (int)(cnt*100 / TOTAL);
        if ((progress > progress_pre) || (cnt==0)) {
            if (dlgprogress) {
                dlgprogress->setValue(progress, 100);
            } else {
                cout << "  ..." << progress << "%" << flush << std::endl;
                progress_pre = progress;
            }
        }
        cnt++;
        // --
        const uint8_t dbid = qry.value(qry.record().indexOf("ID")).toInt(); // id in database
        const uint8_t mid = qry.value(qry.record().indexOf("SYSTEM_ID")).toInt(); // mavlink id
        MavSystem* const sys = scenario._get_or_add_system_byid(mid);
        if (!sys) continue;

        if (!_populateSystem(sys, qry)) {
            cerr << "ERROR populating MAV system " << mid << endl;
        }

        /***************************************************
         * fetch datagroups of that MAV system
         ***************************************************/        
        qry2.setForwardOnly(true);
        qry2.prepare("SELECT * FROM dataGroups WHERE SYSTEM_ID=:dbid;");
        qry2.bindValue(":dbid", dbid);
        if( !qry2.exec() ) {
            std::cerr << "Error obtaining datagroups of system: "<< qry2.lastError().text().toStdString() << std::endl;
            continue;
        }
        cout << "system id in db: " << ((unsigned int)dbid) << endl;
        int ret = _populateDataGroups(sys, qry2);
        if (ret < 0) {
            cerr << "ERROR fetching data for MAV system " << mid << endl;
        }
    }

    runtime = get_time_secs() - runtime;
    std::cout << "END FETCHING FROM DB. Time = " << runtime << "s" << flush;
    scenario.process(false);

    return 0;
}

QSqlDatabase DBConnector::getDB() {
    return _db;
}
