/**
 * @file dbconnector.h
 * @brief   stores scenarios with all systems and their data to the database
 *          generates scenarios from database
 *          imports files with mavlog to database
 * @author Bernd Kast <kastbernd@gmx.de>, Martin Becker <becker@rcs.ei.tum.de>
 * @date 15.11.2014
 */


#ifndef DBCONNECTOR_H
#define DBCONNECTOR_H

#include <string>
#include <QtSql>
#include <iomanip>
#include "mavlink.h"
#include "mavsystem.h"
#include "mavlinkparser.h"
#include "mavlinkscenario.h"
#include "filefun.h"
#include "data_untimed.h"
#include "data_event.h"
#include "dialogprogressbar.h"

/// Class to import and export scenarios from and to the db
class DBConnector {
public:

    /********************************
     *  TYPEDEFS
     ********************************/
    typedef struct db_props_s {
        std::string username;
        std::string password;
        std::string dbhost;
        std::string dbname;
    } db_props_t;

    typedef enum {
        SAVE_ERROR = -1,    ///< error during save
        SAVE_SUCCESS = 0,   ///< saved new entry
        SAVE_UPDATED = 1    ///< updated existing entry
    } save_res_e;

    /********************************
     *  METHODS
     ********************************/

    /**
     * @brief ctor
     * @param args contains login data and maxTimeJump
     */
    DBConnector(const db_props_t& args);
    ~DBConnector() {}
    /**
     * @brief getter for db object, that contains all login data
     * @return db object, that contains all login data
     */
    QSqlDatabase getDB();

    /**
     * @brief imports Mavlog file to scenario in Order to save to DB
     * @param fileName with path
     * @return true if Import was OK
     */
    bool importFromFile(std::string fileName);

    /**
     * @brief import an existing scenario to the DB
     * @param scen
     * @return true on success, else false
     */
    bool saveScenarioToDB(const MavlinkScenario*const scen, DialogProgressBar*dlg=NULL);

    /**
     * @brief Returns scenario with all systems and their data
     * @param id scenario id in database
     * @param scenario this object will be filled with data from the database
     * @return <0: on error <br>0: else
     */
    int loadScenarioFromDB(const int id,MavlinkScenario &scenario, DialogProgressBar*dlg=NULL);

    /**
     * @brief handover the hostname, username etc. for the database we shall use
     * @param props
     */
    void setDBProperties(const db_props_t & props);

    db_props_t getDBProperties(void) const;

    /**
     * @brief verify whether database is reachable and that tables have correct structure etc.
     * @param errmsg
     * @return true if database is reachable and has correct structure
     */
    bool selfTest(std::string & errmsg);

    /**
     * @brief loads a single data group into the system
     * @param sys
     * @param datagroupID the field ID in table dataGroups
     * @return true on success
     */
    bool loadDataGroup(Data*d, DialogProgressBar*dlgprogress);
    bool loadDataGroup(MavSystem*sys, unsigned long long datagroupID, DialogProgressBar*progress);

    /**
     * @brief if true, then we use a faster but incomplete loading techique, where data may not be postprocessed fully
     * @param yesno
     */
    void setLazyLoad(bool yesno) { _deferredLoad = yesno; }
    bool getLazyLoad(void) const { return _deferredLoad; }

private:

    /*******************************************
     * METHODS
     *******************************************/

    int _getScenarioFromFile(const std::string fileName, MavlinkScenario &scenario);  
    int _dbresult2completescenario(QSqlQuery & qry, MavlinkScenario &scenario, const std::map<double,std::string> & events, DialogProgressBar*dlg=NULL);
    int _getEventsFromDB(std::map<std::string,double> &events, double &maxEventID);    
    save_res_e _saveScenario2DB(const MavlinkScenario &scenario, DialogProgressBar *dlg=NULL);
    int _saveSystem2DB(const MavSystem &sys, const int scenarioID, std::map<std::string, double> &events, std::map<std::string, double> &newEvents, double &maxEventID, DialogProgressBar*dlg=NULL);
    int _saveData2DB(const Data &dat, const int systemID, std::map<std::string, double> &events, std::map<std::string, double> &newEvents, double &maxEventID);
    int _saveEvents2DB(const std::map<std::string, double> &newEvents);
    int _insertScenarioToDB(const MavlinkScenario &scenario);
    int _updateScenarioInDB(const MavlinkScenario &scenario, unsigned long long existsID);
    unsigned long long _insertSystemToDB(const MavSystem &sys, const int scenarioID);
    int _insertDataGroupToDB(const Data &dat, const int systemID, const std::string type);
    int _insertDataToDB(const std::vector<double> &data, const std::vector<double> &time, const int dataGroupID);
    template <typename TT>
    void _convertTimeSeriesToDoubleVectorTemplate(const DataTimeseries<TT> &dat, std::vector<double> &data, std::vector<double> &time);
    int _convertDataToDoubleVector(const Data *dat,std::vector <double>& data, std::vector <double>& time, std::string &type, std::map<std::string,double> &events, std::map<std::string,double> &newEvents, double &maxEventID);
    template <typename UT>
    void _convertUntimedDataToDoubleVectorTemplate(const DataUntimed<UT> &dat, std::vector<double> &data, std::vector<double> &time);
    int _loadScenarioFromDB(const int id, MavlinkScenario &scenario, DialogProgressBar*progress);
    int _loadCompleteScenarioFromDB(const int id, MavlinkScenario &scenario, DialogProgressBar*progress);
    int _getReverseEventsFromDB(std::map<double,std::string> &events);
    Data* _fetchDataGroup(MavSystem*sys, const std::string &type, const std::string &path, const std::string &units);
    bool _populateScenario(MavlinkScenario&scenario, QSqlQuery& qry);
    bool _populateSystem(MavSystem* sys, QSqlQuery& qry);
    int  _populateDataGroups(MavSystem*sys, QSqlQuery& qry2);
    bool _populateDataItem(double time, double value, const std::map<double,std::string> &events, Data*data);
    bool _populateDataGroup_deferred(MavSystem*sys, Data*d, unsigned long long datagroup_id);
    bool _populateAllDataGroups_immediate(MavSystem*sys, const std::map<double,std::string>& events);

    /*******************************************
     * ATTRIBUTES
     *******************************************/
    db_props_t _args;   ///< the database information (hostname etc)
    QSqlDatabase _db;   ///< Object to connect to Database
    bool _deferredLoad; ///< if true, loads only those parts of a scenario which the user requests (lazy loading)

    /**
     * @brief The dbBinder struct ensures db connection is properly opend and closed
     */
    struct dbBinder {
        QSqlDatabase *db;
        bool error;
        std::string errmsg;
        dbBinder(QSqlDatabase *dbPtr)
        {
            error = false;
            db = dbPtr;            
            db->setConnectOptions("CLIENT_COMPRESS=1");
            if(!db->open())
            {
                error = true;
                errmsg = "Could not connect to DB. Reason: " + db->lastError().text().toStdString();
                std::cout << errmsg << std::endl;
            }            
        }
        ~dbBinder()
        {
            db->close();
        }
    };

};

#endif // DBCONNECTOR_H
