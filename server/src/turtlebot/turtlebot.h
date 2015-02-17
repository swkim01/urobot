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

#ifndef TURTLEBOT_H
#define TURTLEBOT_H
#define SOURCE_VERSION "1.0"

/* FIXME take a look to the output_http clients thread marked with fixme if you want to set more then 10 plugins */
#define MAX_INPUT_PLUGINS 10
#define MAX_OUTPUT_PLUGINS 10
#define MAX_PLUGIN_ARGUMENTS 32

#ifdef DEBUG
#define DBG(...) fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

#define LOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

#include "input.h"
#include "output.h"

/* global variables that are accessed by all plugins */
typedef struct _globals globals;

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
