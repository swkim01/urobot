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

#ifndef KOBUKI_H
#define KOBUKI_H
#define SOURCE_VERSION "1.0"

#define KOBUKI_DEVICE_TYPE 0

#ifdef __cplusplus
extern "C" {
#endif

enum _sensor_bytes {
    TIMESTAMP_HI	= 0,
    TIMESTAMP_LO	= 1,
    BUMPER	= 2,
    WHEELDROP	= 3,
    CLIFF	= 4,
    LEFTENCODER_HI	= 5,
    LEFTENCODER_LO	= 6,
    RIGHTENCODER_HI	= 7,
    RIGHTENCODER_LO	= 8,
    LEFTPWM	= 9,
    RIGHTPWM	= 10,
    BUTTON	= 11,
    CHARGER	= 12,
    BATTERY	= 13,
    OVERCURRENT	= 14,
};

int kobuki_init(device_argument *args, int id);
int kobuki_stop(int id);
int kobuki_run(int id);
int kobuki_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output);

#ifdef __cplusplus
};
#endif

#endif
