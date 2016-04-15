#ifndef FILTERWINDOW_H
#define FILTERWINDOW_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QtSql>
#include "mainwindow.h" // for progress bar
#include "dbconnector.h"

namespace Ui {
class FilterWindow;
}

///class for functionality of the filter window
class FilterWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief ctor
     */
    explicit FilterWindow(const DBConnector::db_props_t & dbprops, MainWindow *parent = 0);

    /// Desctructor
    ~FilterWindow();

    /**
     * @brief results are written to this model
     */
    void setResultModel(QStandardItemModel*arg);

    /**
     * @brief set up the comboboxes with values from the database
     */
    void setUpComboBox();


private:
    Ui::FilterWindow *ui;

    /**
     * @brief initialise GUI
     */
    void _init();

    /**
     * @brief compares sizes of member-arrays (just to prevent errors)
     */
    void _checkSize();

    /**
     * @brief Get the SCENARIO_IDs for given DARAGROUP_IDs
     * @param DATAGROUP_IDs IDs you want to have the SCENARIO_IDs
     * @return resulting SCENARIO_IDs
     */
    QStringList dataGroupIDs_to_ScenarioIDs(QStringList const &DATAGROUP_IDs);

    /**
     * @brief Show data in table view
     * @param Systems data to show in table view
     */
    void _showResultsTable(QStringList Systems);

    /**
     * @brief Get the name, date and description for each given ScernarioID
     * @param Scenarios a list of IDs to query
     * @return resulting Filenames
     */
    void _getScenarioDetails(const QStringList & Scenarios, QStringList & resultScenarioNames, QStringList & resultScenarioDescs, QStringList & resultScenarioDates) const;

    /**
     * @brief Filter Data according to time criterion
     * @param datagroupIDs of data rows to be filtered
     * @param datagroupIDs_nodouble same as datagroupIDs, but no double entries
     * @param IDs IDs (DB index) of data rows to be filtered
     * @param Times timestamps of data rows to be filtered
     * @param fTime Time the filter property should hold
     * @return DATAGOUP_IDs wich fulfill the time constraints
     */
    QStringList _filterTime(const QStringList &datagroupIDs, const QStringList & datagroupIDs_nodouble, const QStringList &data_IDs,
                            const QStringList &Times, const double &fTime);

     /** @brief Model for table view*/
    QStandardItemModel *filterModel;

     /** @brief Model for table view*/
    QStandardItemModel *resultModel;

     /** @brief List with Filter-Values */
    QStringList filterValues;

     /** @brief List with Filter-Times */
    QStringList filterTime;

    /** @brief List with Filter-Data */
    QStringList filterData;

    /** @brief List with Filter-Operators */
    QStringList filterComp;

    /** @brief Resulting scenario IDs */
    QStringList resultScenarioIDs;

    /** @brief Resulting scenario dates */
    QStringList resultScenarioDates;

    /** @brief resulting scenario names*/
    QStringList resultScenarioNames;

    QStringList resultScenarioDescs;

    QStandardItemModel*_dbResultModel;

    /** @brief Object to connect to Database */
    QSqlDatabase dB;

    MainWindow*_mw;

private slots:
    void on_buttonOK_clicked();
    void on_buttonCancel_clicked();
    void on_buttonPlus_clicked();
    void on_buttonMinus_clicked();
    void on_buttonApplyFilters_clicked();
    void on_buttonSavePreset_clicked();
    void on_buttonPreset_clicked();
};



#endif // FILTERWINDOW_H
