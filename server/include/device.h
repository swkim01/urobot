/*
 * device.h : uRobot Device header.
 * Originally developped for the uRobot project.
 * Copyright (C) 2011-2015 Seong-Woo Kim <swkim01@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DEVICE_H
#define DEVICE_H
#define SOURCE_VERSION "1.0"

#include "linux/types.h"          /* for videodev2.h */
#include "linux/videodev2.h"      /* for videodev2.h */

#include "urobot.h"
#include "services.h"

#define MAX_DEVICE_ARGUMENTS 32
#define MAX_SERVICES 10

#include <syslog.h>
#ifdef DEBUG
#define DBG(...) fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...) 
#endif

#define DLOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, " device: %s", _bf); syslog(LOG_INFO, "%s", _bf); }

/* an enum to identify the commands destination*/
typedef enum {
    Dest_Input = 0,
    Dest_Output = 1,
    Dest_Program = 2,
} command_dest;
/* commands which can be send to the input plugin */
enum _cmd_group {
    IN_CMD_GENERIC = 0, // if you use non V4L2 input plugin you not need to deal the groups.
    IN_CMD_V4L2 = 1,
    IN_CMD_RESOLUTION = 2,
    IN_CMD_JPEG_QUALITY = 3,
};
typedef enum _cmd_group cmd_group;

typedef struct _v4l2ctrl v4l2ctrl;
struct _v4l2ctrl {
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

typedef union _control control;
union _control {
    struct _v4l2ctrl v4l2ctrl;
};

typedef struct device_argument_t device_argument;
typedef struct device_t device;
struct device_argument_t {
    int id;
    char *arguments;
    int argc;
    char *argv[MAX_DEVICE_ARGUMENTS];
    struct device_t *dev;
    struct urobot_t *ut;
};

/* structure to store variables/functions for device */
struct device_t {
    int type;
    char *name;
    void *handle;
    char *model_name;
    char *udn;
    struct buffer_t *presentation;
    device_argument args;

    // input plugin parameters
    control *parameters;
    int parametercount;

    // services
    struct service_t *service[MAX_SERVICES];
    int servicecount;

    int (*init)(device_argument *args, int id);
    int (*stop)(int id);
    int (*run)(int id);
    //int (*cmd)(struct service_t *service, unsigned int control_id, unsigned int args, ...);
    int (*cmd)(struct service_t *service, unsigned int id, action_arg_t *in, action_arg_t *out);

    char *priv_data;
};

/* structure to store common data between devices */
typedef struct device_common_t common;
struct device_common_t {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    unsigned char * buf;
    int size;
    /* v4l2_buffer timestamp */
    struct timeval timestamp;
};

#ifdef __cplusplus
extern "C" {
#endif

struct device_t * urobot_device_add (struct urobot_t *ut, int type, char *name,
	int (*init)(device_argument *, int),
	int (*stop)(int),
	int (*run)(int),
	int (*cmd)(struct service_t *, unsigned int, action_arg_t *, action_arg_t *));
int urobot_device_delete (struct urobot_t *ut, struct device_t *dev);
void init_devices(struct urobot_t *ut, int argc, char *argv[]);
void finish_devices(struct urobot_t *ut, int argc, char *argv[]);

#ifdef __cplusplus
};
#endif

#endif
