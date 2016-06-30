#-------------------------------------------------
#
# Project created by QtCreator 2014-04-18T14:16:19
# (C) 2014-2016 by Martin Becker <becker@rcs.ei.tum.de>
# 
# Builds on Linux with Qt4.8.6 and Qwt package (6.0.x),
# as well as on Windows with MingW 4.8.2-posix-dwarf-rt_v3 
# as well as on Windows with same Qwt version. Newer
# versions of Qt should work, as opposed to newer versions
# of Qwt.
# Qt 5.2.1 on Ubuntu 14: does not work, because libqwt6 still requires Qt4 => both Qt4 and Qt5 get linked in -> segfault
# Qt 4.8 on Ubuntu 14: works with libqwt6
#
#-------------------------------------------------

###########################
#    USER SETTINGS
###########################
# adjust the path to the MavLink headers. You can get them from https://github.com/mavlink/mavlink
MAVLINK_COMMON=$$_PRO_FILE_PWD_/../external/mavlink/gen/common
#message("MavLink headers expected at" $$MAVLINK_COMMON)

# adjust the path to Qwt installation here, if necessary
unix {
	QWT_INSTALLPATH=/usr
	QWT_LIBPATH=$$QWT_INSTALLPATH/lib
	QWT_INCPATH=$$QWT_INSTALLPATH/include/qwt
}
win32 {
        QWT_INSTALLPATH=C:/Qwt-6.1.0
	QWT_LIBPATH=$$QWT_INSTALLPATH/lib
	QWT_INCPATH=$$QWT_INSTALLPATH/include
}
#message("Qwt installation expected at" $$QWT_INSTALLPATH)

############# HANDS AWAY FROM HERE ###############
QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport # because Qt5+ has it in modules now

# save us trouble with installing the correct version of Qt and Qwt
CONFIG+=release_and_debug
CONFIG+=warn_on
CONFIG+=static

###########################
#    GENERAL
###########################
TARGET = MavLogAnalyzer
TEMPLATE = app

###########################
#    CHECK LIBS AND FILES
###########################
!exists($$MAVLINK_COMMON/mavlink.h) {
	error("MavLink files not found. Please configure project file correctly.")
}
!exists($$QWT_INCPATH/qwt_plot_seriesitem.h) {
	error("Qwt files not found. Please configure project file correctly.") 	
}

###########################
#    INCLUDE/LIB PATHS
###########################
# this points to generated MavLink headers
INCLUDEPATH += $$MAVLINK_COMMON $$QWT_INCPATH
LIBS+= -L $$QWT_LIBPATH

###########################
#    CPPFLAGS/LFLAGS
###########################
QMAKE_CXXFLAGS += -Wall -fpermissive
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_DEBUG += -O0
#QMAKE_CXXFLAGS_DEBUG += -pg -p
#QMAKE_LFLAGS_DEBUG += -pg
#QMAKE_CXXFLAGS += -Werror
RESOURCES=app.qrc
LIBS+= -lqwt # -lcsv_parser now "inline"

SOURCES += main.cpp\
        mainwindow.cpp \
    cmdlineargs.cpp \
    mavlinkparser.cpp \    
    mavsystem.cpp \
    SystemTableViewModel.cpp \
    datagroup.cpp \
    stringfun.cpp \
    data.cpp \
    datatreeviewmodel.cpp \
    treeitem.cpp \
    mavplot.cpp \
    filefun.cpp \
    dialogdatadetails.cpp \
    time_fun.cpp \
    mavlinkscenario.cpp \
    dialogprogressbar.cpp \
    onboardlogparser_apm.cpp \
    onboarddata.cpp \
    logger.cpp \
    dialogstats.cpp \
    dialogscenarioprops.cpp \
    dialogdbsettings.cpp \
    filterwindow.cpp \
    dbconnector.cpp \
    dialogselectscenario.cpp \
    mavplotdataitemmodel.cpp \
    dialogdatatable.cpp \
    logtablemodel.cpp \
    onboardlogparser_px4.cpp \
    onboardlogparser.cpp

# add CSV parser
SOURCES += csv_parser/csv_parser.cpp

HEADERS  += mainwindow.h \
    cmdlineargs.h \
    mavlinkparser.h \
    mavsystem.h \
    SystemTableviewModel.h \
    data_timeseries.h \
    data.h \
    datagroup.h \
    stringfun.h \
    datatreeviewmodel.h \
    treeitem.h \
    mavplot.h \
    Zoomer.h \
    Panner.h \
    data_event.h \
    filefun.h \
    dialogdatadetails.h \
    data_untimed.h \
    config.h \
    time_fun.h \
    mavlinkscenario.h \
    dialogprogressbar.h \
    qwt_compat.h \
    onboardlogparser_apm.h \
    onboarddata.h \
    mavsystem_macros.h \
    logger.h \
    dialogstats.h \
    dialogscenarioprops.h \
    dialogdbsettings.h \
    filterwindow.h \
    dbconnector.h \
    debugtype.h \
    dialogselectscenario.h \
    mavplotdataitemmodel.h \
    dialogdatatable.h \
    logtablemodel.h \
    logmsg.h \
    onboardlogparser.h \
    onboardlogparser_px4.h

FORMS    += mainwindow.ui \
	filterwindow.ui

OTHER_FILES += \
    darkorange.stylesheet
