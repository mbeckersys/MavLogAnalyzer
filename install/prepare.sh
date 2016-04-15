#!/bin/bash

#0)benötigte Pakete installieren
sudo aptitude install qtcreator git python-tk libmysqlclient-dev libqwt-dev

# optional (wenn nicht über RCS server)
sudo aptitude install mysql-server

# Blöd: Qt4 (5 auch?) braucht bringt das PLugin für den SQL driver aus lizenzrechtlichen Gründen nicht mit. 
# Prüfe: wenn /usr/lib/x86_64-linux-gnu/qt4/plugins/sqldrivers/libqsqlmysql.so (DEbian 7) nicht existiert,
# dann muss man es wie folgt selbst bauen:
# 1. mkdir /tmp/mysqldrv && cd /tmp/mysqldrv && wget https://download.qt.io/archive/qt/4.8/4.8.2/qt-everywhere-opensource-src-4.8.2.tar.gz
# 2. cd /tmp/mysqldrv/qt-everywhere-opensource-src-4.8.2/src/plugins/sqldrivers/mysql
# 3. qmake "INCLUDEPATH+=/usr/include/mysql" "LIBS+=-L/usr//lib/x86_64-linux-gnu -lmysqlclient_r" mysql.pro # on debian
# 4. make # creates libqsqlmysql.so
# 5. make install
# (Siehe http://doc.qt.io/qt-4.8/sql-driver.html)

#1)Mavlink aus git auschecken
svn_directory=$(pwd)
cd ../../
working_directory=$(pwd)
git clone git://github.com/mavlink/mavlink.git

#2)Mavlink Header generieren (halbautomatisch: Pfade müssen in GUI angegeben werden; Hilfe wird im Terminal ausgegeben
cd mavlink
echo ""
echo ""
echo "Three settings to change:"
echo "XML:	/mavlink/message_definitions/v1.0/ardupilotmega.xml"
echo "Out:	/gen     //'\$working_directory'/mavlink_gen"
echo "Language: C"
echo "=> Generate"
python2.7 mavgenerate.py 

#3)Datenbank anlegen
mysql -u root -p < makedb.sql
