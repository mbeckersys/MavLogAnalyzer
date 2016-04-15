-- ### CREATE DATABASE

-- DROP DATABASE IF EXISTS mavlog_dat;
CREATE DATABASE IF NOT EXISTS mavlog_database;

-- ### MAKE TABLES

-- select our db
use mavlog_database;

-- table 'scenarios'
create table if not exists scenarios (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT, 
TIME_START DATETIME,
DESCRIPTION longtext,
FILENAME text(1024)
);

-- table 'systems'
create table if not exists systems (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT, 
SCENARIO_ID INTEGER UNSIGNED,
SYSTEM_ID INTEGER UNSIGNED,
MAVTYPE INTEGER UNSIGNED,
MAVTYPE_STRING CHAR(254),
APTYPE SMALLINT UNSIGNED,
APTYPE_STRING CHAR(254),
ARMED TINYINT,
TIME DOUBLE,
TIME_VALID TINYINT,
TIME_MIN DOUBLE,
TIME_MAX DOUBLE,
TIME_OFFSET_USEC BIGINT,
TIME_OFFSET_GUESS_USEC BIGINT
);

-- table 'dataGroups'
create table if not exists dataGroups (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT,
SYSTEM_ID INTEGER UNSIGNED,
VALID TINYINT,
NAME CHAR(254),
FULLPATH text(1024),
CLASSIFIER INT,
TIME_EPOCH_DATASTART BIGINT UNSIGNED,
UNITS CHAR(254),
TYPE CHAR(254)
);

-- table 'data'
create table if not exists data (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT,
DATAGROUP_ID INTEGER UNSIGNED,
TIME DOUBLE,
VALUE DOUBLE
);

-- table 'events'
create table if not exists events (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT,
EVENT text(1024)
);

-- table 'presetsName'
create table if not exists presetsName (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT,
PRESET_NAME CHAR(64)
);

-- table 'presetsData'
create table if not exists presetsData (
ID Integer UNSIGNED PRIMARY KEY AUTO_INCREMENT,
PRESET_ID INTEGER UNSIGNED,
filterValues text(1024),
filterOperator Char(2),
filterData Char(255),
filterTime Char(16)
);

-- ### CREATE USER
-- also allows remote login, if you don't want this, then make it 'mavlog_user'@'localhost'
GRANT SELECT,INSERT,UPDATE,DELETE ON mavlog_database.* TO 'mavlog_user_rwd'@'%' identified by 'mavlog_ihUXajJLYu0q';
GRANT SELECT,INSERT ON mavlog_database.* TO 'mavlog_user_rw'@'%' identified by 'mavlog_hwceXJvD';
GRANT SELECT ON mavlog_database.* TO 'mavlog_user_ro'@'%' identified by 'mavlog_password';
