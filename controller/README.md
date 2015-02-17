URobot Controller - Introduction
=============================

URobot Controller is a UPnP (TM) Robot Control App. It implements
the controller app that controls the server on an android platform.

URobot controller is written in Java for the URobot project (see https://github.com/swkim01/urobot/controller/).
It is designed to control robot motion by using button or joystick, monitor any sensors, and stream any camera.
It should compile on any modern platform with installed Android SDK and run on any android-compatible devices.

Requirements
============

The following programs are required to build URobot Controller:

 * Android SDK (software developer kit), r19 or later.

   The Android SDK is part of the Android Tools which can be
   downloaded from http://developer.android.com/.

 * Cling library, 1.0 or later

   The cling library is used to communicate with server using the UPnP protocol.
   cling library can be downloaded from http://4thline.org/projects/cling/.

Building and Installation
=========================

Build and install the package by using android development tools such as
IDE(eclipse or android studio), or command line tools(ant or gradle).

Feedback
========

Please send bug reports, suggestions, ideas, comments or patches to :

   swkim01@gmail.com

Known bugs and limitations
==========================

URobot controller dies abnormally in case of closing window on streaming camera.

Thanks
======

Many thanks to developers for its original work on cling library.

References
==========

Note that this list of references is not complete.

 * UPnP(TM) Standards (http://www.upnp.org/standardizeddcps/default.asp)

Trademarks
==========

UPnP(TM) is a trademark of the UPnP(TM) Implementers Corporation.

-
