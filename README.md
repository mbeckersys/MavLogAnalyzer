# MavLogAnalyzer
GUI to parse, display, filter, export and store flight logs of MavLink, APM:Pilot, Pixhawk etc.

![Alt text](/screenshots/datacursor_log.jpg?raw=true "Screenshot")

## Features
 - Parse MavLink Logfiles as they are created by, e.g., QGroundControl
 - Parse onboard logs of APM:Copter and PX4
 - merge flights (e.g., entire day of flight tests)
 - Graph plot with pan/zoom, marker, annotations, color selection, scaling, ...
 - can compute sythetic data from the raw data, e.g., cumulated power from current and voltage series
 - flight book summary: number of takeoffs, flight time, first and last flight, ...
 - export to CSV and PDF
 - store and load to/from MySQL database
 - ...

## Prerequisites
 - Linux or Windows (Mac)?
 - libqwt6
 - QT4.8+ or later (QT5 also works)
 - MavLink code generator (https://github.com/mavlink/mavlink/)
 - SQL bindings for Qt, if you want to store/load data in/from a database

### Debian 7
With the distro packages, you have to use Qt4 and Qwt 6
apt-get installs libqwt-dev qt4-dev-tools qt4-qmake qtcreator 

### Debian 8
Since Debian 8 does not yet include libqwt-6.1 (still 6.0), Qt4 has to be used like
in Debian 7.

Alternatively, install libqwt-6.1 manually from sources and use Debians's packaged Qt5.

### Tested combinations of Qt, qwt and OS
In general: 
 * Qt5 and Qwt <6.1 does not work
 * Qt4 and Qwt >= 6.1 does not work 

Confirmed combinations:
 * Debian Wheezy (7) with packages Qt 4.8.2 and Qwt 6.0.0 or newer
 * Debian Jessie (8) with packages Qt 4.8.6 and Qwt 6.0.0 or newer
 * Windows 7 with Qt 5.3.1 and MingW 4.8.x and Qwt 6.1.0
 * Ubuntu 14.04 (same as Wheezy): Qt 4.8.2 and Qwt 6.0.0

Other:
 * Ubuntu 10.04 might work: Qt 4.6.2 and Qwt 4.2 (very old!!)

Mind the version of Qt and Qwt. Often there are two versions, and qtcreator depends on the newest. However, you can install more than one version of Qt (e.g., in case Qwt is not compatible to Qt), and chose the compatible Qt version in the project settings. In Ubuntu 14, by default Qt5 is used. That does not work! You must select Qt4.8, otherwise both Qt4 and Qt5 get linked in, since libqwt-6.0 is used on Ubuntu 14.


## Build Instructions
 0. Make sure prerequisites are fulfilled (see above)
 1. Clone this repository (git clone ...)
 2. Change into directory 'external' and follow README instructions
 3. Change into directory 'src' and start qtcreator with the project file 'MavLogAnalyzer.pro'
 4. Configure the project if requested by qtcreator
 5. Build and run
