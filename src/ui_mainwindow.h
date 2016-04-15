/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Apr 13 17:20:37 2016
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTableView>
#include <QtGui/QTextBrowser>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QSplitter *splitterTopBottom;
    QSplitter *splitter_top;
    QGroupBox *grScenario;
    QVBoxLayout *vlScenario;
    QHBoxLayout *vlScenarioHU;
    QVBoxLayout *vlSystems;
    QLabel *label;
    QTableView *tableSystems;
    QVBoxLayout *vlSummary;
    QLabel *label_2;
    QTextBrowser *txtDetails;
    QHBoxLayout *vlScenarioButtons;
    QPushButton *buttonClearScenario;
    QPushButton *buttonScenarioProps;
    QPushButton *buttonSaveDB;
    QWidget *layoutWidget1;
    QHBoxLayout *hTop;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_10;
    QTableView *tableDB;
    QHBoxLayout *horizontalDBCtrl;
    QCheckBox *chkLazy;
    QHBoxLayout *horizontalLayoutButtonsDB;
    QPushButton *buttonSearchDB;
    QPushButton *buttonSetupDB;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_4;
    QListWidget *listFiles;
    QHBoxLayout *horizontalLayout;
    QPushButton *buttonAddFileWithDelay;
    QPushButton *buttonAddFile;
    QWidget *layoutWidget4;
    QHBoxLayout *hBottom;
    QSplitter *splitter_bottom;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *vlData;
    QLabel *label_3;
    QTreeView *treeData;
    QPushButton *buttonAddData;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *vlPlot;
    QScrollBar *scrollHPlot;
    QFrame *line;
    QVBoxLayout *vlActions;
    QLabel *label_6;
    QPushButton *buttonLock;
    QPushButton *buttonXzoom;
    QPushButton *buttonYzoom;
    QPushButton *buttonAutoFit;
    QPushButton *buttonClear;
    QFrame *line_5;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *buttonMarkerA;
    QPushButton *buttonMarkerB;
    QLabel *label_11;
    QComboBox *cboDataSel;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *buttonDataPrev;
    QPushButton *buttonDataPut;
    QPushButton *buttonDataNext;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *buttonDataMin;
    QPushButton *buttonDataMax;
    QPushButton *buttonDataTable;
    QLabel *lblMarkerState;
    QFrame *line_3;
    QLabel *label_8;
    QPushButton *buttonCalcStats;
    QFrame *line_2;
    QSpacerItem *verticalSpacer_2;
    QPushButton *buttonPrint;
    QPushButton *buttonExportPdf;
    QPushButton *buttonExportCsv;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(912, 872);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/logo_rcs.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(QString::fromUtf8(":/images/logo_rcs.png"), QSize(), QIcon::Normal, QIcon::On);
        MainWindow->setWindowIcon(icon);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(5, 5, 5, 5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        splitterTopBottom = new QSplitter(centralWidget);
        splitterTopBottom->setObjectName(QString::fromUtf8("splitterTopBottom"));
        splitterTopBottom->setAutoFillBackground(false);
        splitterTopBottom->setFrameShape(QFrame::NoFrame);
        splitterTopBottom->setLineWidth(0);
        splitterTopBottom->setOrientation(Qt::Vertical);
        splitterTopBottom->setOpaqueResize(false);
        splitterTopBottom->setHandleWidth(2);
        splitterTopBottom->setChildrenCollapsible(false);
        splitter_top = new QSplitter(splitterTopBottom);
        splitter_top->setObjectName(QString::fromUtf8("splitter_top"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitter_top->sizePolicy().hasHeightForWidth());
        splitter_top->setSizePolicy(sizePolicy);
        splitter_top->setMinimumSize(QSize(5, 0));
        splitter_top->setLineWidth(0);
        splitter_top->setOrientation(Qt::Horizontal);
        splitter_top->setOpaqueResize(false);
        splitter_top->setHandleWidth(5);
        splitter_top->setChildrenCollapsible(false);
        grScenario = new QGroupBox(splitter_top);
        grScenario->setObjectName(QString::fromUtf8("grScenario"));
        QFont font;
        font.setBold(false);
        font.setWeight(50);
        grScenario->setFont(font);
        vlScenario = new QVBoxLayout(grScenario);
        vlScenario->setSpacing(6);
        vlScenario->setContentsMargins(11, 11, 11, 11);
        vlScenario->setObjectName(QString::fromUtf8("vlScenario"));
        vlScenarioHU = new QHBoxLayout();
        vlScenarioHU->setSpacing(6);
        vlScenarioHU->setObjectName(QString::fromUtf8("vlScenarioHU"));
        vlSystems = new QVBoxLayout();
        vlSystems->setSpacing(6);
        vlSystems->setObjectName(QString::fromUtf8("vlSystems"));
        label = new QLabel(grScenario);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);

        vlSystems->addWidget(label);

        tableSystems = new QTableView(grScenario);
        tableSystems->setObjectName(QString::fromUtf8("tableSystems"));
        tableSystems->setSelectionMode(QAbstractItemView::SingleSelection);
        tableSystems->setSelectionBehavior(QAbstractItemView::SelectRows);

        vlSystems->addWidget(tableSystems);


        vlScenarioHU->addLayout(vlSystems);

        vlSummary = new QVBoxLayout();
        vlSummary->setSpacing(6);
        vlSummary->setObjectName(QString::fromUtf8("vlSummary"));
        label_2 = new QLabel(grScenario);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        vlSummary->addWidget(label_2);

        txtDetails = new QTextBrowser(grScenario);
        txtDetails->setObjectName(QString::fromUtf8("txtDetails"));
        txtDetails->setEnabled(true);
        txtDetails->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        vlSummary->addWidget(txtDetails);


        vlScenarioHU->addLayout(vlSummary);


        vlScenario->addLayout(vlScenarioHU);

        vlScenarioButtons = new QHBoxLayout();
        vlScenarioButtons->setSpacing(6);
        vlScenarioButtons->setObjectName(QString::fromUtf8("vlScenarioButtons"));
        buttonClearScenario = new QPushButton(grScenario);
        buttonClearScenario->setObjectName(QString::fromUtf8("buttonClearScenario"));

        vlScenarioButtons->addWidget(buttonClearScenario);

        buttonScenarioProps = new QPushButton(grScenario);
        buttonScenarioProps->setObjectName(QString::fromUtf8("buttonScenarioProps"));

        vlScenarioButtons->addWidget(buttonScenarioProps);

        buttonSaveDB = new QPushButton(grScenario);
        buttonSaveDB->setObjectName(QString::fromUtf8("buttonSaveDB"));

        vlScenarioButtons->addWidget(buttonSaveDB);


        vlScenario->addLayout(vlScenarioButtons);

        splitter_top->addWidget(grScenario);
        layoutWidget1 = new QWidget(splitter_top);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        hTop = new QHBoxLayout(layoutWidget1);
        hTop->setSpacing(6);
        hTop->setContentsMargins(11, 11, 11, 11);
        hTop->setObjectName(QString::fromUtf8("hTop"));
        hTop->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(layoutWidget1);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(-1, 9, -1, -1);
        label_10 = new QLabel(tab);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setFont(font);

        verticalLayout_2->addWidget(label_10);

        tableDB = new QTableView(tab);
        tableDB->setObjectName(QString::fromUtf8("tableDB"));

        verticalLayout_2->addWidget(tableDB);

        horizontalDBCtrl = new QHBoxLayout();
        horizontalDBCtrl->setSpacing(6);
        horizontalDBCtrl->setObjectName(QString::fromUtf8("horizontalDBCtrl"));
        horizontalDBCtrl->setContentsMargins(-1, 0, -1, -1);
        chkLazy = new QCheckBox(tab);
        chkLazy->setObjectName(QString::fromUtf8("chkLazy"));
        chkLazy->setChecked(true);

        horizontalDBCtrl->addWidget(chkLazy);


        verticalLayout_2->addLayout(horizontalDBCtrl);

        horizontalLayoutButtonsDB = new QHBoxLayout();
        horizontalLayoutButtonsDB->setSpacing(6);
        horizontalLayoutButtonsDB->setObjectName(QString::fromUtf8("horizontalLayoutButtonsDB"));
        buttonSearchDB = new QPushButton(tab);
        buttonSearchDB->setObjectName(QString::fromUtf8("buttonSearchDB"));

        horizontalLayoutButtonsDB->addWidget(buttonSearchDB);

        buttonSetupDB = new QPushButton(tab);
        buttonSetupDB->setObjectName(QString::fromUtf8("buttonSetupDB"));

        horizontalLayoutButtonsDB->addWidget(buttonSetupDB);


        verticalLayout_2->addLayout(horizontalLayoutButtonsDB);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_3 = new QVBoxLayout(tab_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_4 = new QLabel(tab_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font);

        verticalLayout_3->addWidget(label_4);

        listFiles = new QListWidget(tab_2);
        listFiles->setObjectName(QString::fromUtf8("listFiles"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(listFiles->sizePolicy().hasHeightForWidth());
        listFiles->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(listFiles);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 0, -1, -1);
        buttonAddFileWithDelay = new QPushButton(tab_2);
        buttonAddFileWithDelay->setObjectName(QString::fromUtf8("buttonAddFileWithDelay"));

        horizontalLayout->addWidget(buttonAddFileWithDelay);

        buttonAddFile = new QPushButton(tab_2);
        buttonAddFile->setObjectName(QString::fromUtf8("buttonAddFile"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(buttonAddFile->sizePolicy().hasHeightForWidth());
        buttonAddFile->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(buttonAddFile);


        verticalLayout_3->addLayout(horizontalLayout);

        tabWidget->addTab(tab_2, QString());

        hTop->addWidget(tabWidget);

        splitter_top->addWidget(layoutWidget1);
        splitterTopBottom->addWidget(splitter_top);
        layoutWidget4 = new QWidget(splitterTopBottom);
        layoutWidget4->setObjectName(QString::fromUtf8("layoutWidget4"));
        hBottom = new QHBoxLayout(layoutWidget4);
        hBottom->setSpacing(6);
        hBottom->setContentsMargins(11, 11, 11, 11);
        hBottom->setObjectName(QString::fromUtf8("hBottom"));
        hBottom->setSizeConstraint(QLayout::SetMinimumSize);
        hBottom->setContentsMargins(0, 0, 0, 0);
        splitter_bottom = new QSplitter(layoutWidget4);
        splitter_bottom->setObjectName(QString::fromUtf8("splitter_bottom"));
        sizePolicy.setHeightForWidth(splitter_bottom->sizePolicy().hasHeightForWidth());
        splitter_bottom->setSizePolicy(sizePolicy);
        splitter_bottom->setOrientation(Qt::Horizontal);
        verticalLayoutWidget = new QWidget(splitter_bottom);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        vlData = new QVBoxLayout(verticalLayoutWidget);
        vlData->setSpacing(6);
        vlData->setContentsMargins(11, 11, 11, 11);
        vlData->setObjectName(QString::fromUtf8("vlData"));
        vlData->setContentsMargins(0, 0, 0, 0);
        label_3 = new QLabel(verticalLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        label_3->setFont(font1);

        vlData->addWidget(label_3);

        treeData = new QTreeView(verticalLayoutWidget);
        treeData->setObjectName(QString::fromUtf8("treeData"));
        sizePolicy1.setHeightForWidth(treeData->sizePolicy().hasHeightForWidth());
        treeData->setSizePolicy(sizePolicy1);
        treeData->setAlternatingRowColors(false);

        vlData->addWidget(treeData);

        buttonAddData = new QPushButton(verticalLayoutWidget);
        buttonAddData->setObjectName(QString::fromUtf8("buttonAddData"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(buttonAddData->sizePolicy().hasHeightForWidth());
        buttonAddData->setSizePolicy(sizePolicy3);

        vlData->addWidget(buttonAddData);

        splitter_bottom->addWidget(verticalLayoutWidget);
        verticalLayoutWidget_2 = new QWidget(splitter_bottom);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        vlPlot = new QVBoxLayout(verticalLayoutWidget_2);
        vlPlot->setSpacing(6);
        vlPlot->setContentsMargins(11, 11, 11, 11);
        vlPlot->setObjectName(QString::fromUtf8("vlPlot"));
        vlPlot->setContentsMargins(0, 0, 0, 0);
        scrollHPlot = new QScrollBar(verticalLayoutWidget_2);
        scrollHPlot->setObjectName(QString::fromUtf8("scrollHPlot"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(scrollHPlot->sizePolicy().hasHeightForWidth());
        scrollHPlot->setSizePolicy(sizePolicy4);
        scrollHPlot->setMinimumSize(QSize(0, 15));
        scrollHPlot->setOrientation(Qt::Horizontal);

        vlPlot->addWidget(scrollHPlot);

        splitter_bottom->addWidget(verticalLayoutWidget_2);

        hBottom->addWidget(splitter_bottom);

        line = new QFrame(layoutWidget4);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        hBottom->addWidget(line);

        vlActions = new QVBoxLayout();
        vlActions->setSpacing(6);
        vlActions->setObjectName(QString::fromUtf8("vlActions"));
        vlActions->setSizeConstraint(QLayout::SetMinimumSize);
        vlActions->setContentsMargins(10, -1, -1, -1);
        label_6 = new QLabel(layoutWidget4);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy5);
        label_6->setFont(font1);
        label_6->setStyleSheet(QString::fromUtf8("Heading"));

        vlActions->addWidget(label_6);

        buttonLock = new QPushButton(layoutWidget4);
        buttonLock->setObjectName(QString::fromUtf8("buttonLock"));
        sizePolicy2.setHeightForWidth(buttonLock->sizePolicy().hasHeightForWidth());
        buttonLock->setSizePolicy(sizePolicy2);
        buttonLock->setCheckable(true);

        vlActions->addWidget(buttonLock);

        buttonXzoom = new QPushButton(layoutWidget4);
        buttonXzoom->setObjectName(QString::fromUtf8("buttonXzoom"));
        sizePolicy2.setHeightForWidth(buttonXzoom->sizePolicy().hasHeightForWidth());
        buttonXzoom->setSizePolicy(sizePolicy2);
        buttonXzoom->setCheckable(true);

        vlActions->addWidget(buttonXzoom);

        buttonYzoom = new QPushButton(layoutWidget4);
        buttonYzoom->setObjectName(QString::fromUtf8("buttonYzoom"));
        sizePolicy2.setHeightForWidth(buttonYzoom->sizePolicy().hasHeightForWidth());
        buttonYzoom->setSizePolicy(sizePolicy2);
        buttonYzoom->setCheckable(true);

        vlActions->addWidget(buttonYzoom);

        buttonAutoFit = new QPushButton(layoutWidget4);
        buttonAutoFit->setObjectName(QString::fromUtf8("buttonAutoFit"));
        sizePolicy2.setHeightForWidth(buttonAutoFit->sizePolicy().hasHeightForWidth());
        buttonAutoFit->setSizePolicy(sizePolicy2);

        vlActions->addWidget(buttonAutoFit);

        buttonClear = new QPushButton(layoutWidget4);
        buttonClear->setObjectName(QString::fromUtf8("buttonClear"));
        sizePolicy2.setHeightForWidth(buttonClear->sizePolicy().hasHeightForWidth());
        buttonClear->setSizePolicy(sizePolicy2);

        vlActions->addWidget(buttonClear);

        line_5 = new QFrame(layoutWidget4);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        vlActions->addWidget(line_5);

        label_9 = new QLabel(layoutWidget4);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        vlActions->addWidget(label_9);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, -1, -1, 6);
        buttonMarkerA = new QPushButton(layoutWidget4);
        buttonMarkerA->setObjectName(QString::fromUtf8("buttonMarkerA"));
        buttonMarkerA->setEnabled(false);
        QSizePolicy sizePolicy6(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(1);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(buttonMarkerA->sizePolicy().hasHeightForWidth());
        buttonMarkerA->setSizePolicy(sizePolicy6);
        buttonMarkerA->setMinimumSize(QSize(0, 0));
        buttonMarkerA->setMaximumSize(QSize(40, 16777215));
        buttonMarkerA->setCheckable(true);

        horizontalLayout_2->addWidget(buttonMarkerA);

        buttonMarkerB = new QPushButton(layoutWidget4);
        buttonMarkerB->setObjectName(QString::fromUtf8("buttonMarkerB"));
        buttonMarkerB->setEnabled(false);
        sizePolicy6.setHeightForWidth(buttonMarkerB->sizePolicy().hasHeightForWidth());
        buttonMarkerB->setSizePolicy(sizePolicy6);
        buttonMarkerB->setMaximumSize(QSize(40, 16777215));
        buttonMarkerB->setCheckable(true);

        horizontalLayout_2->addWidget(buttonMarkerB);


        vlActions->addLayout(horizontalLayout_2);

        label_11 = new QLabel(layoutWidget4);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        vlActions->addWidget(label_11);

        cboDataSel = new QComboBox(layoutWidget4);
        cboDataSel->setObjectName(QString::fromUtf8("cboDataSel"));
        sizePolicy2.setHeightForWidth(cboDataSel->sizePolicy().hasHeightForWidth());
        cboDataSel->setSizePolicy(sizePolicy2);

        vlActions->addWidget(cboDataSel);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, -1, -1, 6);
        buttonDataPrev = new QPushButton(layoutWidget4);
        buttonDataPrev->setObjectName(QString::fromUtf8("buttonDataPrev"));
        buttonDataPrev->setEnabled(false);
        QSizePolicy sizePolicy7(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(buttonDataPrev->sizePolicy().hasHeightForWidth());
        buttonDataPrev->setSizePolicy(sizePolicy7);
        buttonDataPrev->setMinimumSize(QSize(0, 20));

        horizontalLayout_3->addWidget(buttonDataPrev);

        buttonDataPut = new QPushButton(layoutWidget4);
        buttonDataPut->setObjectName(QString::fromUtf8("buttonDataPut"));
        buttonDataPut->setEnabled(true);
        buttonDataPut->setMinimumSize(QSize(0, 20));
        buttonDataPut->setCheckable(true);

        horizontalLayout_3->addWidget(buttonDataPut);

        buttonDataNext = new QPushButton(layoutWidget4);
        buttonDataNext->setObjectName(QString::fromUtf8("buttonDataNext"));
        buttonDataNext->setEnabled(false);
        sizePolicy7.setHeightForWidth(buttonDataNext->sizePolicy().hasHeightForWidth());
        buttonDataNext->setSizePolicy(sizePolicy7);
        buttonDataNext->setMinimumSize(QSize(0, 20));

        horizontalLayout_3->addWidget(buttonDataNext);


        vlActions->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(-1, -1, -1, 6);
        buttonDataMin = new QPushButton(layoutWidget4);
        buttonDataMin->setObjectName(QString::fromUtf8("buttonDataMin"));
        buttonDataMin->setEnabled(false);
        sizePolicy7.setHeightForWidth(buttonDataMin->sizePolicy().hasHeightForWidth());
        buttonDataMin->setSizePolicy(sizePolicy7);

        horizontalLayout_4->addWidget(buttonDataMin);

        buttonDataMax = new QPushButton(layoutWidget4);
        buttonDataMax->setObjectName(QString::fromUtf8("buttonDataMax"));
        buttonDataMax->setEnabled(false);
        sizePolicy7.setHeightForWidth(buttonDataMax->sizePolicy().hasHeightForWidth());
        buttonDataMax->setSizePolicy(sizePolicy7);

        horizontalLayout_4->addWidget(buttonDataMax);


        vlActions->addLayout(horizontalLayout_4);

        buttonDataTable = new QPushButton(layoutWidget4);
        buttonDataTable->setObjectName(QString::fromUtf8("buttonDataTable"));
        sizePolicy7.setHeightForWidth(buttonDataTable->sizePolicy().hasHeightForWidth());
        buttonDataTable->setSizePolicy(sizePolicy7);

        vlActions->addWidget(buttonDataTable);

        lblMarkerState = new QLabel(layoutWidget4);
        lblMarkerState->setObjectName(QString::fromUtf8("lblMarkerState"));
        lblMarkerState->setEnabled(false);
        lblMarkerState->setLayoutDirection(Qt::LeftToRight);
        lblMarkerState->setAlignment(Qt::AlignCenter);

        vlActions->addWidget(lblMarkerState);

        line_3 = new QFrame(layoutWidget4);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        vlActions->addWidget(line_3);

        label_8 = new QLabel(layoutWidget4);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        vlActions->addWidget(label_8);

        buttonCalcStats = new QPushButton(layoutWidget4);
        buttonCalcStats->setObjectName(QString::fromUtf8("buttonCalcStats"));
        sizePolicy7.setHeightForWidth(buttonCalcStats->sizePolicy().hasHeightForWidth());
        buttonCalcStats->setSizePolicy(sizePolicy7);

        vlActions->addWidget(buttonCalcStats);

        line_2 = new QFrame(layoutWidget4);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        vlActions->addWidget(line_2);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vlActions->addItem(verticalSpacer_2);

        buttonPrint = new QPushButton(layoutWidget4);
        buttonPrint->setObjectName(QString::fromUtf8("buttonPrint"));
        sizePolicy7.setHeightForWidth(buttonPrint->sizePolicy().hasHeightForWidth());
        buttonPrint->setSizePolicy(sizePolicy7);

        vlActions->addWidget(buttonPrint);

        buttonExportPdf = new QPushButton(layoutWidget4);
        buttonExportPdf->setObjectName(QString::fromUtf8("buttonExportPdf"));
        sizePolicy7.setHeightForWidth(buttonExportPdf->sizePolicy().hasHeightForWidth());
        buttonExportPdf->setSizePolicy(sizePolicy7);

        vlActions->addWidget(buttonExportPdf);

        buttonExportCsv = new QPushButton(layoutWidget4);
        buttonExportCsv->setObjectName(QString::fromUtf8("buttonExportCsv"));
        sizePolicy7.setHeightForWidth(buttonExportCsv->sizePolicy().hasHeightForWidth());
        buttonExportCsv->setSizePolicy(sizePolicy7);

        vlActions->addWidget(buttonExportCsv);


        hBottom->addLayout(vlActions);

        hBottom->setStretch(0, 1);
        splitterTopBottom->addWidget(layoutWidget4);

        verticalLayout->addWidget(splitterTopBottom);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        statusBar->setEnabled(true);
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MavLogAnalyzer (C)2013-2016 by Martin Becker", 0, QApplication::UnicodeUTF8));
        grScenario->setTitle(QApplication::translate("MainWindow", "Current Scenario", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Systems:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "System Summary (select one at the left)", 0, QApplication::UnicodeUTF8));
        buttonClearScenario->setText(QApplication::translate("MainWindow", "Clear Scenario", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonScenarioProps->setToolTip(QApplication::translate("MainWindow", "Edit properties of current scenario", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonScenarioProps->setText(QApplication::translate("MainWindow", "Scenario Properties ...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonSaveDB->setToolTip(QApplication::translate("MainWindow", "Save current scenario to database", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonSaveDB->setText(QApplication::translate("MainWindow", "Save to DB", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("MainWindow", "Database Scenarios:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkLazy->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        chkLazy->setText(QApplication::translate("MainWindow", "Lazy Loading", 0, QApplication::UnicodeUTF8));
        buttonSearchDB->setText(QApplication::translate("MainWindow", "Search DB ...", 0, QApplication::UnicodeUTF8));
        buttonSetupDB->setText(QApplication::translate("MainWindow", "DB Settings ...", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Load from Database", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "Loaded Files:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonAddFileWithDelay->setToolTip(QApplication::translate("MainWindow", "Parse a file and add user-defined time offset", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonAddFileWithDelay->setText(QApplication::translate("MainWindow", "Add File with Delay ...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonAddFile->setToolTip(QApplication::translate("MainWindow", "Parse a file", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonAddFile->setText(QApplication::translate("MainWindow", "Add File ...", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "Load from File", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "Data of selected system:", 0, QApplication::UnicodeUTF8));
        buttonAddData->setText(QApplication::translate("MainWindow", "Add to Plot", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Actions", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonLock->setToolTip(QApplication::translate("MainWindow", "Lock zoom (pan is still allowed)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonLock->setText(QApplication::translate("MainWindow", "Lock Zoom", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonXzoom->setToolTip(QApplication::translate("MainWindow", "Lock vertical zoom, allow horizontal zoom", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonXzoom->setText(QApplication::translate("MainWindow", "X-Zoom", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonYzoom->setToolTip(QApplication::translate("MainWindow", "Lock horizontal zoom, allow vertical zoom", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonYzoom->setText(QApplication::translate("MainWindow", "Y-Zoom", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonAutoFit->setToolTip(QApplication::translate("MainWindow", "Set X and Y zoom such that all data is visible", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonAutoFit->setText(QApplication::translate("MainWindow", "AutoFit", 0, QApplication::UnicodeUTF8));
        buttonClear->setText(QApplication::translate("MainWindow", "Clear Plot", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("MainWindow", "Markers:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonMarkerA->setToolTip(QApplication::translate("MainWindow", "Set marker A", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonMarkerA->setText(QApplication::translate("MainWindow", "A", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonMarkerB->setToolTip(QApplication::translate("MainWindow", "Set marker B", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonMarkerB->setText(QApplication::translate("MainWindow", "B", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("MainWindow", "Data Cursor:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cboDataSel->setToolTip(QApplication::translate("MainWindow", "Select data", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        buttonDataPrev->setToolTip(QApplication::translate("MainWindow", "Set cursor to previous point of selected data", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonDataPrev->setText(QApplication::translate("MainWindow", "<", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonDataPut->setToolTip(QApplication::translate("MainWindow", "Set cursor in plot", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonDataPut->setText(QApplication::translate("MainWindow", "Put", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonDataNext->setToolTip(QApplication::translate("MainWindow", "Set cursor to next point of selected data", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonDataNext->setText(QApplication::translate("MainWindow", ">", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonDataMin->setToolTip(QApplication::translate("MainWindow", "Set cursor to MAX of selected data", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonDataMin->setText(QApplication::translate("MainWindow", "Min", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonDataMax->setToolTip(QApplication::translate("MainWindow", "Set cursor to MAX of selected data", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonDataMax->setText(QApplication::translate("MainWindow", "Max", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonDataTable->setToolTip(QApplication::translate("MainWindow", "Show selected data as table", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonDataTable->setText(QApplication::translate("MainWindow", "Data Table ...", 0, QApplication::UnicodeUTF8));
        lblMarkerState->setText(QApplication::translate("MainWindow", "Lock zoom to enable", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("MainWindow", "Analysis:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonCalcStats->setToolTip(QApplication::translate("MainWindow", "Show statistics for plot window", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonCalcStats->setText(QApplication::translate("MainWindow", "Calc Statistics", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonPrint->setToolTip(QApplication::translate("MainWindow", "Print plot window", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonPrint->setText(QApplication::translate("MainWindow", "Print", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonExportPdf->setToolTip(QApplication::translate("MainWindow", "Export plot window as PDF", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonExportPdf->setText(QApplication::translate("MainWindow", "Export PDF", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        buttonExportCsv->setToolTip(QApplication::translate("MainWindow", "Export all data in plot to CSV files", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        buttonExportCsv->setText(QApplication::translate("MainWindow", "Export to CSV", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
