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

#ifndef OPENNI_KINECT_H
#define OPENNI_KINECT_H

#include <XnOpenNI.h>
#include <XnCppWrapper.h>
//using namespace xn;
#include "urobot.h"
#include "device.h"
#include "devices/cambuf.h"

enum _streaming_state {
    STREAMING_OFF = 0,
    STREAMING_ON = 1,
    STREAMING_PAUSED = 2,
};
typedef enum _streaming_state streaming_state;

struct vdIn {
    int width;
    int height;
    int fps;
    XnPixelFormat formatIn;
    int formatOut;
    int framesizeIn;
    xn::ImageMetaData* pImageMD;
    //const XnYUV422DoublePixel* pImageMap;
    const XnUInt8* pImageMap;
};

/* context of each camera thread */
typedef struct {
    int id;
    struct urobot_t *ut;
    pthread_t threadID;
    pthread_mutex_t controls_mutex;
    struct vdIn *videoIn;
    cameraBuffer *cambuf;
} context;

extern context cams[MAX_DEVICE_PLUGINS];

#endif
