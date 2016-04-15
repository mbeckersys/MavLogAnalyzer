#include "filterwindow.h"
#include "ui_filterwindow.h"
#include <iostream>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDebug>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qsqlerror.h>
#include <dbconnector.h>
#include <qinputdialog.h>

using namespace std;

FilterWindow::FilterWindow(const DBConnector::db_props_t & dbprops, MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::FilterWindow),
    _dbResultModel(NULL),
    _mw(parent)
{
    ui->setupUi(this);
    _init();
    DBConnector dbcon(dbprops);
    dB=dbcon.getDB();
    ui->tableResults->horizontalHeader()->setStretchLastSection(true);
    ui->tableResults->setSortingEnabled(true);
}

void FilterWindow::setResultModel(QStandardItemModel*arg) {
    _dbResultModel = arg;
}

FilterWindow::~FilterWindow() {
    delete ui; // FIXME: is this necessary?
}

void FilterWindow::on_buttonOK_clicked(){
    //setUp Model for QTableView in Mainwindow
    if (!_dbResultModel) return;
    _dbResultModel->removeRows(0, _dbResultModel->rowCount());
    for (int i=0; i<resultScenarioIDs.size(); i++) {
        QList<QStandardItem *> newRow;
        QStandardItem *item = new QStandardItem(resultScenarioIDs[i]);
        QStandardItem *item2= new QStandardItem(resultScenarioDates[i]);
        QStandardItem *item3= new QStandardItem(resultScenarioNames[i]);
        QStandardItem *item4= new QStandardItem(resultScenarioDescs[i]);

        newRow << item << item2 << item3 << item4;
        _dbResultModel->appendRow(newRow);
    }
    //close Window
    this->done(1);
}

void FilterWindow::on_buttonCancel_clicked() {
    this->done(1);
}

// TODO: escape!
void FilterWindow::on_buttonApplyFilters_clicked() {
    if(!dB.open()) {
        cerr << "Cannot open DB connection!" <<   dB.lastError().text().toStdString() << endl;
        return;
    }

    QStringList ScenarioIDsResult;
    QString stringQry;
    const int max = filterValues.size();    

    if (max == 0) { // if there are no filters, then list all
        QMessageBox msgbox(QMessageBox::Information, "Whoops", QString("No filter added...showing all database entries."));  msgbox.exec();
        QSqlQuery qry;
        QString strQueryAll;
        strQueryAll="select ID from scenarios;";
        qry.prepare(strQueryAll);
        if(!qry.exec()) {
           cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
           return;
        }
        while(qry.next()) {
            qulonglong ID=qry.value(0).toULongLong();
            ScenarioIDsResult.append(QString().number(ID));
        }
    } else {    // otherwise run filters

        if (_mw) {
            _mw->showProgressBar();
            _mw->updateProgressBarTitle("Searching...");
            _mw->updateProgressBarValue(0,max);
        }

        // ## for each Filter
        for (int i=0; i<max; i++) {            

            QStringList DATA_GROUP_ID;
            QString strQueryGroups;
            QSqlQuery qry;

            const QString thisFilter_operator = filterComp[i];
            const QString thisFilter_value = filterValues[i];
            const double thisFilter_time = filterTime[i].toDouble();

            //get DATA_GROUP_ID of current condition
            strQueryGroups="select ID from dataGroups where FULLPATH=:datapath && VALID=1;";
            qry.prepare(strQueryGroups);            
            qry.bindValue(":datapath", filterData[i]);

            if(!qry.exec()) {
               cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
               if (_mw) _mw->hideProgressBar();
               return;
            }
            while(qry.next()) {
                DATA_GROUP_ID.append(qry.value(0).toString());
            }

            // ## now get all data that fulfills the condition of current filter
            stringQry="select * from data where (";
            for(int a=0; a<DATA_GROUP_ID.size(); a++) {
                if (a>0) {
                    stringQry+= " OR ";
                }
                stringQry+="DATAGROUP_ID=?";
            }
            // FIXME: sanitize thisFilter_operator. we cannot bind it, unfortunately
            stringQry+=") AND (VALUE " + thisFilter_operator +" ?);";
            QSqlQuery qry2;
            qry2.prepare(stringQry);
            // positional bindings
            for(int a=0; a<DATA_GROUP_ID.size(); a++) {
                qry2.bindValue(a, DATA_GROUP_ID[a]);
            }
            qry2.bindValue(DATA_GROUP_ID.size() ,thisFilter_value);
            // do it!
            if(!qry2.exec()) {
               cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;               
               if (_mw) _mw->hideProgressBar();
               return;
            }            

            // crunch results w.r.t. time criterion
            QSqlRecord recqry2 = qry2.record();
            const int indexDATAGROUP_ID = recqry2.indexOf("DATAGROUP_ID");
            const int indexID = recqry2.indexOf("ID");
            const int indexTime = recqry2.indexOf("TIME");
            QStringList data_dataGroupIDs; // all data rows that fulfill value criteria: get their datagroup ID
            QStringList data_IDs; // ...get their own ID
            QStringList data_timestamp; // ... get their timestamp FIXME: why stringlist??
            while(qry2.next()) {
                 data_dataGroupIDs.append(qry2.value(indexDATAGROUP_ID).toString());
                 data_IDs.append(qry2.value(indexID).toString());
                 data_timestamp.append(qry2.value(indexTime).toString());
            }

            // filter duplicates by building a set, then check whether time criterion applies
            QStringList dataGroup_IDs_notimefilter_set = data_dataGroupIDs.toSet().toList();
            QStringList dataGroup_IDs_timefiltered = _filterTime(data_dataGroupIDs, dataGroup_IDs_notimefilter_set, data_IDs, data_timestamp, thisFilter_time);

            // finally...as search result only store the scenario IDs
            QStringList ScenarioIDs;
            ScenarioIDs = dataGroupIDs_to_ScenarioIDs(dataGroup_IDs_timefiltered); //get Scenario IDs from DataGroup_IDs
            ScenarioIDsResult.append(ScenarioIDs);
            // update progress
            if (_mw) _mw->updateProgressBarValue(i+1,max);
        }
    }

    // resulting scenario IDs => make a set;
    ScenarioIDsResult=ScenarioIDsResult.toSet().toList();//filter duplicates

    // how are filters combined?
    if(ui->radioOr->isChecked()) {
        // OR: take all results <=> nothing to do
    } else {
        // AND: take only data which fullfills conditions of all filters <=> must be available 'max' times in the list:
        const int nrFilter=filterComp.size();
        QStringList result;
        for (int ii=0; ii<ScenarioIDsResult.size(); ii++) {
            if(ScenarioIDsResult.count(ScenarioIDsResult[ii]) == nrFilter) {
                result.append(ScenarioIDsResult[ii]);
            }
        }
        ScenarioIDsResult=result;
    }

    _showResultsTable(ScenarioIDsResult);
    dB.close();
    if (_mw) _mw->hideProgressBar();
}

void FilterWindow::on_buttonPlus_clicked(){
    bool OK1, OK2;
    //double value1, value2;
    ui->lineEditValue->text().toDouble(&OK1);
    ui->lineEditTime->text().toDouble(&OK2);
    if (OK1&OK2){
        QList<QStandardItem *> newRow;
        QStandardItem *item = new QStandardItem(ui->comboBoxData->currentText());
        QStandardItem *item2 = new QStandardItem(ui->comboBoxComp->currentText());
        QStandardItem *item3 = new QStandardItem(ui->lineEditValue->text());
        QStandardItem *item4 = new QStandardItem(ui->lineEditTime->text());
        newRow<<item << item2 << item3 << item4;
        filterModel->appendRow(newRow);

        filterValues.append(ui->lineEditValue->text());
        filterTime.append(ui->lineEditTime->text());
        filterData.append(ui->comboBoxData->currentText());
        filterComp.append(ui->comboBoxComp->currentText());
        _checkSize(); //check size of arrays
        //qDebug()<<*filterData<<*filterComp<<*filterValues<<*filterTime<<endl;
    }
    else{
        QMessageBox::warning( NULL, "Warning","Please provide a numeric expression", QMessageBox::Ok);
    }
}

void FilterWindow::on_buttonMinus_clicked(){
    int row;
    row=ui->tableFilters->currentIndex().row();
    if (filterModel->removeRow(row)){
        filterValues.removeAt(row);
        filterData.removeAt(row);
        filterComp.removeAt(row);
        filterTime.removeAt(row);
        _checkSize();
    }
}

void FilterWindow::on_buttonSavePreset_clicked()
{
    if (!filterData.isEmpty())
    {
        //save selected filter
        bool ok = false;
        QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("Preset-Name:"), QLineEdit::Normal, "", &ok);

        if (ok && !text.isEmpty()) {
            if( !dB.open() ) {
                std::cerr << "Error: Could not open DB connection!" <<   dB.lastError().text().toStdString() << std::endl;
                return;
            }

            // save preset name
            QString presetID;
            {
                QSqlQuery qry;
                QString str;
                str= "insert into presetsName (PRESET_NAME) value (:text);";
                qry.prepare(str);
                qry.bindValue(":text", text);
                if( !qry.exec() ) {
                   cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
                   return;
                }

                // get ID of the new preset
                qry.prepare("select LAST_INSERT_ID();");
                if( !qry.exec() ) {
                   std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
                   dB.close();
                   return;
                }
                qry.next();
                presetID = qry.value(0).toString();
            }

            // save filters for the new preset
            for (int i=0; i<filterData.size(); i++) {
                QSqlQuery qry;
                QString str;
                str = "INSERT INTO presetsData (PRESET_ID, filterValues, filterOperator, filterData, filterTime) values ";
                str+= "(:presetID, :filterValues, :filterComp, :filterData, :filterTime);";
                qry.prepare(str);
                qry.bindValue(":presetID", presetID);
                qry.bindValue(":filterValues", filterValues[i]);
                qry.bindValue(":filterComp", filterComp[i]);
                qry.bindValue(":filterData", filterData[i]);
                qry.bindValue(":filterTime", filterTime[i]);
                if( !qry.exec() ) {
                   std::cerr << "Error occured during execution of Query: "<<qry.lastError().text().toStdString() << std::endl;
                   dB.close();
                   return;
                }
            }
            ui->comboBoxPresets->addItem(text);
            QMessageBox::information( NULL, "OK","Preset saved", QMessageBox::Ok);
            dB.close();
        } else {
            QMessageBox::warning( NULL, "Warning","No Preset Name", QMessageBox::Ok);
        }
    } else {
        QMessageBox::warning( NULL, "Warning","No filter in selection!", QMessageBox::Ok);
    }
}

void FilterWindow::on_buttonPreset_clicked()
{    
    if( !dB.open()) {
        std::cerr << "Error: Could not open DB connection!" << dB.lastError().text().toStdString() << std::endl;
        return;
    }

    const QString preset = ui->comboBoxPresets->currentText();
    cout << "Selected Preset: "<< preset.toStdString() << endl;

    // get preset ID: FIXME: use JOIN
    QString ID;
    {
        QSqlQuery qry;
        QString str="SELECT ID FROM presetsName WHERE PRESET_NAME=:name;";
        qry.prepare(str);
        qry.bindValue(":name", preset);
        if( !qry.exec() )  {
            cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
            return;
        }
        qry.next();
        ID = qry.value(0).toString();
    }

    // get preset filters
    {
        QSqlQuery qry;
        QString str ="SELECT * FROM presetsData WHERE PRESET_ID = :ID;";
        qry.prepare(str);
        qry.bindValue(":ID", ID);
        if( !qry.exec() ) {
            cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
            return;
        }

        int filterValuesNo = qry.record().indexOf("filterValues");
        int filterOperatorNo = qry.record().indexOf("filterOperator");
        int filterDataNo = qry.record().indexOf("filterData");
        int filterTimeNo = qry.record().indexOf("filterTime");

        while (qry.next()) {
            QString fV=qry.value(filterValuesNo).toString();
            QString fO=qry.value(filterOperatorNo).toString();
            QString fD=qry.value(filterDataNo).toString();
            QString fT=qry.value(filterTimeNo).toString();
            //qDebug() << fV<<fO<<fD<<fT<< endl;

            QList<QStandardItem *> newRow;
            QStandardItem *item = new QStandardItem(fD);
            QStandardItem *item2 = new QStandardItem(fO);
            QStandardItem *item3 = new QStandardItem(fV);
            QStandardItem *item4 = new QStandardItem(fT);
            newRow<<item << item2 << item3 << item4;
            filterModel->appendRow(newRow);

            filterValues.append(fV);
            filterTime.append(fT);
            filterData.append(fD);
            filterComp.append(fO);
        }
        //qDebug()<<*filterData<<*filterComp<<*filterValues<<*filterTime<<endl;
        _checkSize(); //check size of arrays
    }

    dB.close();
}


void FilterWindow::_checkSize() {
    if(filterComp.size()!=filterData.size() || filterData.size() != filterValues.size()
                                            || filterValues.size() != filterTime.size()){
        QMessageBox::warning( NULL, "Warning","Size Missmatch", QMessageBox::Ok);
        this->done(1);
    }
}

void FilterWindow::_init() {
    //Set style
    QList<QLabel*> list = this->findChildren<QLabel*>();
    foreach(QLabel *l, list) {
        if (l->fontInfo().bold()) {
            l->setProperty("class", "bold");
            l->style()->unpolish(l);
            l->ensurePolished();
        }
    }

    //pre-check radio-button
    ui->radioOr->setChecked(true);

    //set up filter model
    filterModel = new QStandardItemModel(0,0,this);
    QStringList headerNames;
    headerNames<<"Data"<<"Operator"<<"Filter Value"<<"Time";
    filterModel->setHorizontalHeaderLabels(headerNames);
    ui->tableFilters->setModel(filterModel);
    ui->tableFilters->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableFilters->setSelectionBehavior(QAbstractItemView::SelectRows);

    //set up result model
    resultModel = new QStandardItemModel(0,0,this);
    QStringList headerNamesResults;
    headerNamesResults<<"Scenario ID" << "Date" << "Name" << "Description";
    resultModel->setHorizontalHeaderLabels(headerNamesResults);
    ui->tableResults->setModel(resultModel);
    ui->tableResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableResults->setSelectionBehavior(QAbstractItemView::SelectRows);

    //init text for line edit
    ui->lineEditTime->setText("0");
    ui->lineEditValue->setText("0");
}

void FilterWindow::setUpComboBox() {
    //get datagroups from DB to fill ComboBox
    if( !dB.open() ) {
        std::cerr << "Error: No DB connection!" <<   dB.lastError().text().toStdString() << std::endl;
        return;
    }

    QStringList boxData;
    {
        QString s="SELECT FULLPATH FROM dataGroups;";
        QSqlQuery qry;
        qry.prepare(s);
        if( !qry.exec() ) {
           cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
           return;
        }

        while (qry.next()) {
            s=qry.value(0).toString();
            boxData.append(s);
        }
        boxData=boxData.toSet().toList();
    }

    QStringList presetNames;
    {
        QSqlQuery qry;
        QString s="select PRESET_NAME from presetsName";
        qry.prepare(s);
        if( !qry.exec() ) {
            cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
            return;
        }

        while (qry.next()) {
            s = qry.value(0).toString();
            presetNames.append(s);
        }
    }

    boxData.sort();
    presetNames.sort();
    ui->comboBoxData->addItems(boxData);
    ui->comboBoxPresets->addItems(presetNames);
    dB.close();
}

QStringList FilterWindow::dataGroupIDs_to_ScenarioIDs(QStringList const &dataGroup_IDs) {
    QStringList scenario_IDs;
    for (int i=0; i<dataGroup_IDs.size(); i++) {

        //get System_ID from dataGroup_IDs // FIXME: use JOIN
        QString sys_id;
        {
            QSqlQuery qry1;
            QString str1 = "SELECT SYSTEM_ID FROM dataGroups WHERE ID =:ID;";
            qry1.prepare(str1);
            qry1.bindValue(":ID", dataGroup_IDs[i]);
            if( !qry1.exec()) {
               cerr << "Error occured during execution of Query: "<< qry1.lastError().text().toStdString() << endl;
               return scenario_IDs;
            }
            qry1.next();
            sys_id = qry1.value(0).toString();
        }

        //get Scenario_ID from System_ID
        {
            QSqlQuery qry2;
            QString str2 = "select SCENARIO_ID FROM systems WHERE ID=:ID;";
            qry2.prepare(str2);
            qry2.bindValue(":ID", sys_id);
            if( !qry2.exec() ) {
               cerr << "Error occured during execution of Query: "<< qry2.lastError().text().toStdString() << endl;
               return scenario_IDs;
            }
            qry2.next();
            scenario_IDs.append(qry2.value(0).toString());
        }
    }
    return scenario_IDs;
}

/*
 * FIXME: what is this pointer chaos??
 */
void FilterWindow::_showResultsTable(QStringList Scenarios) {
    Scenarios.sort();
    resultModel->removeRows(0, resultModel->rowCount());

    resultScenarioIDs = Scenarios; // save results for when we leave the dialog
    _getScenarioDetails(Scenarios, resultScenarioNames, resultScenarioDescs, resultScenarioDates);

    //qDebug() << *resultScenarioNames;
    for (int i=0; i<Scenarios.size(); i++) {
        QList<QStandardItem *> newRow;
        QStandardItem *item = new QStandardItem(Scenarios[i]);
        QStandardItem *item2= new QStandardItem(resultScenarioDates[i]);
        QStandardItem *item3= new QStandardItem(resultScenarioNames[i]);
        QStandardItem *item4= new QStandardItem(resultScenarioDescs[i]);
        newRow << item << item2 << item3 << item4;
        resultModel->appendRow(newRow);
    }
}

void FilterWindow::_getScenarioDetails(const QStringList & ID, QStringList & ScenarioNames, QStringList & ScenarioDescs, QStringList & ScenarioDates) const {
    ScenarioDates.clear(); ScenarioDescs.clear(); ScenarioNames.clear();
    if (ID.size()==0) return;

    QString strQry="SELECT TIME_START, DESCRIPTION, FILENAME FROM scenarios WHERE ID=:id;";
    for (int i=0; i<ID.size(); i++) {
        QSqlQuery qry;
        qry.prepare(strQry);
        qry.bindValue(":id", ID[i]);

        if( !qry.exec() ) {
           cerr << "Error occured during execution of Query: "<< qry.lastError().text().toStdString() << endl;
           ScenarioDescs.append(QString("??"));
           ScenarioDates.append(QString("??"));
           ScenarioNames.append(QString("??"));
        } else {
            if (qry.next()) {
                ScenarioDates.append(qry.value(0).toString());
                ScenarioDescs.append(qry.value(1).toString());
                ScenarioNames.append(qry.value(2).toString());
            } else {
                ScenarioDescs.append(QString("??"));
                ScenarioDates.append(QString("??"));
                ScenarioNames.append(QString("??"));
            }
        }
    }
}

QStringList FilterWindow::_filterTime(QStringList const &data_datagroup_IDs, const QStringList &data_datagroup_IDs_nodouble, QStringList const &data_IDs,
                                      QStringList const &Times, double const &fTime) {
    int startidx=0;
    bool firstElement=true;    

    if (fTime <= 0) return data_datagroup_IDs_nodouble;
    const double time_sec = fTime/1000;


    QStringList dataGroup_IDs_compliant;

    // for each datagroup: check whether data in group fulfills time criterion
    for (int currentGroup=0; currentGroup<data_datagroup_IDs_nodouble.size(); currentGroup++) {
        // go through ALL data rows ...
        firstElement = true;
        for (int dataidx = 1; dataidx < data_datagroup_IDs.size(); dataidx++) {
            if(data_datagroup_IDs[dataidx] == data_datagroup_IDs_nodouble[currentGroup]) {
                // this data row belongs to the group that we are currently checking...
                if (firstElement) {
                    // first data row that fulfills criterion => save row number
                    firstElement = false;
                    startidx = dataidx;
                } else {
                    /*
                     * assumption: data was inserted one group after the other, and IDs are auto-increment
                     * Since we already filtered data w.r.t. the value, this means if we have a gap in the
                     * data ID, then there was a value in between that did not fulfill the value criterion.
                     * In other words, data IDs have to be ascending w/o gaps for longer than the time
                     * criterion.
                     */
                    // if IDs are still ascending w/o gaps...
                    if (data_IDs[dataidx-1].toInt() == data_IDs[dataidx].toInt()-1) {
                        // check if that has been going on for long enough...
                        if (Times[dataidx-1].toDouble() - Times[startidx].toDouble() > time_sec) {
                            dataGroup_IDs_compliant.append(data_datagroup_IDs_nodouble[currentGroup]);
                            break; // finish with this datagroup, it is compliant
                        }
                    } else {
                        // there is a gap! -> restart time counter
                        startidx=dataidx;
                    }
                }
            }
        }
    }
    return dataGroup_IDs_compliant;
}
