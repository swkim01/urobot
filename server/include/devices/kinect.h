/*******************************************************************************
# Turtlebot-Kinect streaming input-plugin for uRobot                           #
#                                                                              #
# This package work with the MS Kinect based camera with the mjpeg feature     #
#                                                                              #
# Copyright (C) 2012 Seong-Woo Kim                                             #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#ifndef KINECT_H
#define KINECT_H

#define KINECT_DEVICE_TYPE 1

#ifdef __cplusplus
extern "C" {
#endif

int kinect_init(device_argument *args, int id);
int kinect_stop(int id);
int kinect_run(int id);
int kinect_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output);

#ifdef __cplusplus
}
#endif

#endif
