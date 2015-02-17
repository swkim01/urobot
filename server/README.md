URobot Server - Introduction
=============================

URobot Server is a UPnP (TM) Robot Control Server. It implements the server
component that provides UPnP robot devices on available robot platforms.

URobot server is written in C for the URobot project.
It is designed to provide access to device/service profiles and web contents to be controlled by any other URobot controller devices.
It should compile and run on any modern POSIX compatible system such as Linux.

Requirements
============

The following programs are required to build URobot Server:

 * GNU C Compiler (gcc), 2.95 or later.

   The GNU C Compiler is part of the GNU Compiler Collection which can be
   downloaded from http://gcc.gnu.org/.

 * pkg-config.

   pkg-config is a helper tool used when compiling applications and libraries.
   It helps you insert the correct compiler options on the command line.
   (see http://pkg-config.freedesktop.org/wiki/ ).

 * cmake, 2.8.0 or later.

   cmake is a build tool.
````
   sudo apt-get install cmake
````
 * Linux SDK for UPnP Devices (libupnp), 1.4.2 or later

   The libupnp library is used to communicate using the UPnP protocol.
   libupnp can be downloaded from http://pupnp.sourceforge.net/.
````
   sudo apt-get install libupnp-dev
````
 * libctemplate, 1.0 or later

   The libctemplate library is used to provide HTML web contents.
````
   sudo apt-get install libctemplate-dev
````
The following software is required to build and run on Kobuki robot :

 * kobuki-core, 1.0 or later

   The kobuki-core package is used to communicate with kobuki robot.
   The installation process is as following links.

   http://yujinrobo.github.io/kobuki/doxygen/enInstallationLinuxGuide.html

The following software is required to build for Kinect device :

 * SensorKinect driver, 5.1.2.1 or later, and OpenNI, 1.5 or later

   The SensorKinect driver and OpenNI package are used to communicate with
   kinect device. The installation process is as following links.

   https://github.com/avin2/SensorKinect
   https://github.com/OpenNI/OpenNI

Building and Installation
=========================

Compile URobot server by making a build directory, running cmake and then make.
This should produce executable urobot servers in the build/src subdirectory.

Currently, URobot can be installed on four platforms which are
linux with uvc cam, raspberrypi with picam, turtlebot from willowgarage co.,
and kobuki from Yujin Robotics co.. Thus, you can select the name of <target> as linux, raspberrypi, turtlebot, or kobuki.

Example on linux or raspberrypi :
````
$ sudo apt-get install cmake libupnp-dev libctemplate-dev
$ git clone https://github.com/swkim01/urobot.git
$ cd urobot/server
$ mkdir build; cd build
$ cmake ..
$ make
$ sudo make install
````
If you want to build URobot server for kobuki robot and kinect device,
you have to install kobuki-core package, sensorKinect driver, OpenNI library,
and dependent packages.

If you want to install URobot servers on your system, run :
````
$ sudo make install
````
This will copy the executables into their appropriate directories
(/usr/local/bin in this example).

Usage
=====

URobot runs from the console only. It supports the usual --help option
which displays usage and option information.
````
Usage: urobot-<target> [-n name] [-i interface] [-p port] [-f cfg_file] [-w] [-D]
Options:
 -n, --name=NAME        Set UPnP Friendly Name (default is 'URobot')
 -i, --interface=IFACE  Use IFACE Network Interface (default is 'eth0')
 -f, --cfg=FILE         Config file to be used
 -p, --port=PORT        Forces the HTTP server to run on PORT
 -w, --no-web           Disable the control web page (enabled by default)
 -v, --verbose          Set verbose display
 -D, --daemon           Run as a daemon.
 -V, --version          Display the version of URobot and exit
 -h, --help             Display this help
````
URobot gets its configuration from the /etc/urobot.conf file.
You can force configuration options through command line.
````
   urobot-<target> -f urobot.conf
````
You can also perform remote control of URobot UPnP Server through its
web interface. This let you define new content locations at runtime or
update the currently shared one in case the filesystem has changed.
Just go to :

   http://ip_address:port/web/urobot.html

Note on raspberrypi, you should run the program as root user
due to access right to GPIO. That is, use sudo in front of the command.

Feedback
========

Please send bug reports, suggestions, ideas, comments or patches to :

   swkim01@gmail.com

Known bugs and limitations
==========================

URobot keeps some information on files in memory.

Thanks
======

Many thanks to Benjamin Jores for its original work on uShare.
Many thanks to Tom Stoeveken for its original work on MJPG-streamer.

References
==========

Note that this list of references is not complete.

 * UPnP(TM) Standards (http://www.upnp.org/standardizeddcps/default.asp)

Trademarks
==========

UPnP(TM) is a trademark of the UPnP(TM) Implementers Corporation.

-
