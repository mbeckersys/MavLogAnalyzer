/********************************************************************************
** Form generated from reading UI file 'filterwindow.ui'
**
** Created: Wed Apr 13 17:20:37 2016
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILTERWINDOW_H
#define UI_FILTERWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTableView>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FilterWindow
{
public:
    QWidget *centralWidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *comboBoxPresets;
    QPushButton *buttonPreset;
    QRadioButton *radioOr;
    QRadioButton *radioAnd;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_3;
    QSplitter *splitter;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_3;
    QComboBox *comboBoxData;
    QLabel *label_4;
    QComboBox *comboBoxComp;
    QLabel *label_5;
    QLineEdit *lineEditValue;
    QLabel *label_6;
    QLineEdit *lineEditTime;
    QPushButton *buttonPlus;
    QPushButton *buttonMinus;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_8;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *buttonSavePreset;
    QPushButton *buttonApplyFilters;
    QTableView *tableFilters;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QTableView *tableResults;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *buttonCancel;
    QPushButton *buttonOK;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QDialog *FilterWindow)
    {
        if (FilterWindow->objectName().isEmpty())
            FilterWindow->setObjectName(QString::fromUtf8("FilterWindow"));
        FilterWindow->resize(912, 704);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/logo_rcs.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(QString::fromUtf8(":/images/logo_rcs.png"), QSize(), QIcon::Normal, QIcon::On);
        FilterWindow->setWindowIcon(icon);
        centralWidget = new QWidget(FilterWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setGeometry(QRect(0, 0, 921, 671));
        horizontalLayoutWidget = new QWidget(centralWidget);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(20, 20, 881, 29));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setMinimumSize(QSize(1, 27));
        label->setMaximumSize(QSize(92, 27));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);

        horizontalLayout->addWidget(label);

        comboBoxPresets = new QComboBox(horizontalLayoutWidget);
        comboBoxPresets->setObjectName(QString::fromUtf8("comboBoxPresets"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(10);
        sizePolicy1.setHeightForWidth(comboBoxPresets->sizePolicy().hasHeightForWidth());
        comboBoxPresets->setSizePolicy(sizePolicy1);
        comboBoxPresets->setMinimumSize(QSize(250, 0));

        horizontalLayout->addWidget(comboBoxPresets, 0, Qt::AlignLeft);

        buttonPreset = new QPushButton(horizontalLayoutWidget);
        buttonPreset->setObjectName(QString::fromUtf8("buttonPreset"));

        horizontalLayout->addWidget(buttonPreset);

        radioOr = new QRadioButton(horizontalLayoutWidget);
        radioOr->setObjectName(QString::fromUtf8("radioOr"));

        horizontalLayout->addWidget(radioOr);

        radioAnd = new QRadioButton(horizontalLayoutWidget);
        radioAnd->setObjectName(QString::fromUtf8("radioAnd"));

        horizontalLayout->addWidget(radioAnd);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setGeometry(QRect(20, 50, 881, 581));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy2);
        splitter->setOrientation(Qt::Vertical);
        verticalLayoutWidget = new QWidget(splitter);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_3 = new QLabel(verticalLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font);

        horizontalLayout_6->addWidget(label_3);

        comboBoxData = new QComboBox(verticalLayoutWidget);
        comboBoxData->setObjectName(QString::fromUtf8("comboBoxData"));
        comboBoxData->setMinimumSize(QSize(200, 0));

        horizontalLayout_6->addWidget(comboBoxData);

        label_4 = new QLabel(verticalLayoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font);

        horizontalLayout_6->addWidget(label_4);

        comboBoxComp = new QComboBox(verticalLayoutWidget);
        comboBoxComp->setObjectName(QString::fromUtf8("comboBoxComp"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(comboBoxComp->sizePolicy().hasHeightForWidth());
        comboBoxComp->setSizePolicy(sizePolicy3);

        horizontalLayout_6->addWidget(comboBoxComp);

        label_5 = new QLabel(verticalLayoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setFont(font);

        horizontalLayout_6->addWidget(label_5);

        lineEditValue = new QLineEdit(verticalLayoutWidget);
        lineEditValue->setObjectName(QString::fromUtf8("lineEditValue"));

        horizontalLayout_6->addWidget(lineEditValue);

        label_6 = new QLabel(verticalLayoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setFont(font);

        horizontalLayout_6->addWidget(label_6);

        lineEditTime = new QLineEdit(verticalLayoutWidget);
        lineEditTime->setObjectName(QString::fromUtf8("lineEditTime"));

        horizontalLayout_6->addWidget(lineEditTime);

        buttonPlus = new QPushButton(verticalLayoutWidget);
        buttonPlus->setObjectName(QString::fromUtf8("buttonPlus"));
        buttonPlus->setMinimumSize(QSize(50, 0));

        horizontalLayout_6->addWidget(buttonPlus);

        buttonMinus = new QPushButton(verticalLayoutWidget);
        buttonMinus->setObjectName(QString::fromUtf8("buttonMinus"));
        buttonMinus->setMinimumSize(QSize(50, 0));

        horizontalLayout_6->addWidget(buttonMinus);


        verticalLayout->addLayout(horizontalLayout_6);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_8 = new QLabel(verticalLayoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setFont(font);

        horizontalLayout_3->addWidget(label_8);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        buttonSavePreset = new QPushButton(verticalLayoutWidget);
        buttonSavePreset->setObjectName(QString::fromUtf8("buttonSavePreset"));

        horizontalLayout_3->addWidget(buttonSavePreset);

        buttonApplyFilters = new QPushButton(verticalLayoutWidget);
        buttonApplyFilters->setObjectName(QString::fromUtf8("buttonApplyFilters"));
        buttonApplyFilters->setMinimumSize(QSize(105, 0));

        horizontalLayout_3->addWidget(buttonApplyFilters);


        verticalLayout->addLayout(horizontalLayout_3);

        tableFilters = new QTableView(verticalLayoutWidget);
        tableFilters->setObjectName(QString::fromUtf8("tableFilters"));

        verticalLayout->addWidget(tableFilters);

        splitter->addWidget(verticalLayoutWidget);
        verticalLayoutWidget_2 = new QWidget(splitter);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(verticalLayoutWidget_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        verticalLayout_2->addWidget(label_2);

        tableResults = new QTableView(verticalLayoutWidget_2);
        tableResults->setObjectName(QString::fromUtf8("tableResults"));

        verticalLayout_2->addWidget(tableResults);

        splitter->addWidget(verticalLayoutWidget_2);
        horizontalLayoutWidget_2 = new QWidget(centralWidget);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(19, 630, 881, 41));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        buttonCancel = new QPushButton(horizontalLayoutWidget_2);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));

        horizontalLayout_2->addWidget(buttonCancel);

        buttonOK = new QPushButton(horizontalLayoutWidget_2);
        buttonOK->setObjectName(QString::fromUtf8("buttonOK"));

        horizontalLayout_2->addWidget(buttonOK);

        splitter->raise();
        horizontalLayoutWidget->raise();
        horizontalLayoutWidget_2->raise();
        mainToolBar = new QToolBar(FilterWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setGeometry(QRect(0, 0, 4, 13));
        statusBar = new QStatusBar(FilterWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        statusBar->setGeometry(QRect(0, 0, 3, 22));

        retranslateUi(FilterWindow);

        QMetaObject::connectSlotsByName(FilterWindow);
    } // setupUi

    void retranslateUi(QDialog *FilterWindow)
    {
        FilterWindow->setWindowTitle(QApplication::translate("FilterWindow", "MavLogAnayzer (C)2014 by Martin Becker", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("FilterWindow", "Presets:", 0, QApplication::UnicodeUTF8));
        buttonPreset->setText(QApplication::translate("FilterWindow", "Add Preset to Filter-List", 0, QApplication::UnicodeUTF8));
        radioOr->setText(QApplication::translate("FilterWindow", "OR", 0, QApplication::UnicodeUTF8));
        radioAnd->setText(QApplication::translate("FilterWindow", "AND", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("FilterWindow", "Data:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("FilterWindow", "Operator:", 0, QApplication::UnicodeUTF8));
        comboBoxComp->clear();
        comboBoxComp->insertItems(0, QStringList()
         << QApplication::translate("FilterWindow", ">", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("FilterWindow", ">=", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("FilterWindow", "<", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("FilterWindow", "<=", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("FilterWindow", "=", 0, QApplication::UnicodeUTF8)
        );
        label_5->setText(QApplication::translate("FilterWindow", "Filter-Value:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("FilterWindow", "Time (ms):", 0, QApplication::UnicodeUTF8));
        buttonPlus->setText(QApplication::translate("FilterWindow", "+", 0, QApplication::UnicodeUTF8));
        buttonMinus->setText(QApplication::translate("FilterWindow", "-", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("FilterWindow", "Selected Filters:", 0, QApplication::UnicodeUTF8));
        buttonSavePreset->setText(QApplication::translate("FilterWindow", "Save as Preset", 0, QApplication::UnicodeUTF8));
        buttonApplyFilters->setText(QApplication::translate("FilterWindow", "Search", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("FilterWindow", "Results:", 0, QApplication::UnicodeUTF8));
        buttonCancel->setText(QApplication::translate("FilterWindow", "Cancel", 0, QApplication::UnicodeUTF8));
        buttonOK->setText(QApplication::translate("FilterWindow", "OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FilterWindow: public Ui_FilterWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILTERWINDOW_H
