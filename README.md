URobot - Introduction
=============================

URobot is a UPnP (TM) Robot Control System. It implements the server
component that provides UPnP robot devices among available robot platforms,
and the controller app that controls the server on an android platform.

URobot server is written in C for the URobot project (see https://github.com/swkim01/urobot/tree/master/server/).
It is designed to provide access to device/service profiles and web contents to be controlled by any other URobot controller devices.
It should compile and run on any modern POSIX compatible system such as Linux.

URobot controller is written in Java for the URobot project (see https://github.com/swkim01/urobot/tree/master/controller/).
It is designed to control robot motion by using button or joystick, monitor any sensors, and stream any camera.
It should compile on any modern platform with installed Android SDK and run on any android-compatible devices.

URobot is free software - it is licensed under the terms of the GNU
General Public License (GPL).

Copyright and License
=====================

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 51
Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Note that URobot links with libupnp, which is licensed under the
terms of a modified BSD license (i.e. the original BSD license without the
advertising clause). This license is compatible with the GNU GPL.

Homepage
========

Web site and file area for URobot is hosted on URobot server :

   http://github.com/swkim01/urobot/

The latest version of URobot should always be available on this site.

Feedback
========

Please send bug reports, suggestions, ideas, comments or patches to :

   swkim01@gmail.com

Known bugs and limitations
==========================

Thanks
======

Many thanks to Benjamin Jores for its original work on uShare.
Many thanks to Tom Stoeveken for its original work on MJPG-streamer.
Many thanks to developers for its original work on cling library.

References
==========

Note that this list of references is not complete.

 * UPnP(TM) Standards (http://www.upnp.org/standardizeddcps/default.asp)

Trademarks
==========

UPnP(TM) is a trademark of the UPnP(TM) Implementers Corporation.

-
