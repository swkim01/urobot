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

#include <syslog.h>
#include "turtlebot.h"
#define INPUT_PLUGIN_PREFIX " i: "
#define IPRINT(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", INPUT_PLUGIN_PREFIX); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

/* parameters for input plugin */
typedef struct _input_parameter input_parameter;
struct _input_parameter {
    int id;
    char *parameters;
    int argc;
    char *argv[MAX_PLUGIN_ARGUMENTS];
    struct _globals *global;
};

/* structure to store variables/functions for input plugin */
typedef struct _input input;
struct _input {
    char *plugin;
    void *handle;

    input_parameter param; // this holds the command line arguments

    /* signal fresh frames */
    pthread_mutex_t db;
    pthread_cond_t  db_update;

    /* global JPG frame, this is more or less the "database" */
    unsigned char *buf;
    int size;

    /* v4l2_buffer timestamp */
    struct timeval timestamp;

    int (*init)(input_parameter *, int id);
    int (*stop)(int);
    int (*run)(int);
    int (*cmd)(int plugin, unsigned int control_id, unsigned int group, int value);
};

//int input_init(input_parameter *, int id);
//int input_run(int);
