/*******************************************************************************
#                                                                              #
#      uRobot allows to stream JPG frames from an input-plugin                 #
#      to several output plugins                                               #
#                                                                              #
#      Copyright (C) 2012 Seong-Woo Kim                                        #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
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

#ifndef HYBUS_H
#define HYBUS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <termios.h>
#include <strings.h>
#include <signal.h>

#define VIDEOBUF_COUNT  3
#define STILLBUF_COUNT  2
#define WIDTH			320
#define HEIGHT			240

#define CAM_STATUS_INIT     0
#define CAM_STATUS_READY    1
#define CAM_STATUS_BUSY     2

#define DEVICE_ACC          "/dev/MMA_ADC"
#define DEVICE_ADC          "/dev/FOUR_ADC"
#define SERIAL2             "/dev/ttyS2"

#define CMD_PORT			9734
#define CAM_PORT			9735
#define SEN_PORT			9736

#define RGB_SIZE (320*240*3)
#define BUFF_SIZE ((320*240) + (320*240/2) + (320*240/2))
#define CLIP(x) (((x) < 0) ? 0 : (((x) > 255) ? 255 : (x)))

#define START		0xAA
#define END			0xFF

#define ADCID		0x00AD
#define ACCID		0x00AC
#define CAMID		0x0C

#define MO 0xAF
#define SE 0xB9

/*---------------------- ACC -----------------------*/ 
#define MMA_SLEEP_MODE_ON       0x1001
#define MMA_SLEEP_MODE_OFF      0x1002
#define MMA_VIN_OFF             0x1003
#define MMA_VIN_ON              0x1004
#define MMA_SENS_15G            0x1005  // 800mVg
#define MMA_SENS_20G            0x1006  // 600mVg
#define MMA_SENS_40G            0x1007  // 300mVg
#define MMA_SENS_60G            0x1008  // 200mVg

/*-------------- Car Key Function ---------------*/
#define NN	0xAA
#define NE	0xB2
#define EE	0xAC
#define SS	0xAB
#define WW	0xAD
#define NW	0xB8
#define F1	0xA0
#define F2	0xA1
#define F3	0xA2
#define F4	0xA3
#define F5	0xA4
#define F6	0xA5

/*------------- Robot Key Function -------------*/

#define RNN 1
#define RNE 2
#define REE 3
#define RSS 4
#define RWW 5
#define RNW 6
#define RF1 7
#define RF2 8
#define RF3 9
#define RF4 10
#define RF5 11
#define RF6 12

/*------------------ Car CMD Code ---------------*/

/* Light CMD & Option */
#define ON			0x00
#define OFF			0xFF
#define LEFT		1
#define RIGHT		2
#define EMER		3
#define LIGHT       0xF0 // Head Light
#define BREAK       0xF1 // Break Light
#define FOG         0xF2 // Fog Light
#define WINKER      0xF3 // Winker Light

/* L Motor Control CMD */
#define SETSPEED	0xA0 //L Motor Set Speed (1~3900)
#define SETDIS		0xA7 //L Motor Set Distance	(0~65535)
#define	FORWARD		0xC8 // Forward !!
#define BACKWART	0xC9 // Backward !!
#define STOP		0xCA // STOP

/* Serve Motor Control CMD */
#define STEERA		0xF4 //Steering angle
#define STTERS		0xF5 //Steering speed

#define CAMA		0xF6 //Camera angle
#define CAMS		0xF7 //Camera speed

/*------------------------------------------------*/
#define MAX_INPUT_PLUGINS 10
#define MAX_OUTPUT_PLUGINS 10
#define MAX_PLUGIN_ARGUMENTS 32

#ifdef DEBUG_BUILD
#define DBG(...) fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

#define LOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

#include "input.h"
#include "output.h"

/* global variables that are accessed by all plugins */
typedef struct _globals globals;

/* an enum to identify the commands destination*/
typedef enum {
    Dest_Input = 0,
    Dest_Output = 1,
    Dest_Program = 2,
} command_dest;

/* commands which can be send to the input plugin */
typedef enum _cmd_group cmd_group;
enum _cmd_group {
    IN_CMD_GENERIC = 0, // if you use non V4L2 input plugin you not need to deal the groups.
    IN_CMD_V4L2 = 1,
    IN_CMD_RESOLUTION = 2,
    IN_CMD_JPEG_QUALITY = 3,
};

#if 0
typedef struct _control control;
struct _control {
    struct v4l2_queryctrl ctrl;
    int value;
    struct v4l2_querymenu *menuitems;
    /*  In the case the control a V4L2 ctrl this variable will specify
        that the control is a V4L2_CTRL_CLASS_USER control or not.
        For non V4L2 control it is not acceptable, leave it 0.
    */
    int class_id;
    int group;
};
#endif

struct _globals {
    int stop;

    /* input plugin */
    input in[MAX_INPUT_PLUGINS];
    int incnt;

    /* output plugin */
    output out[MAX_OUTPUT_PLUGINS];
    int outcnt;

    /* pointer to control functions */
    //int (*control)(int command, char *details);
};

#endif
