/*******************************************************************************
# Turtlebot-Roomba operating input-plugin for uRobot                           #
#                                                                              #
# This package work with the iRobot Roomba based robots                        #
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

#ifndef ROOMBA_H
#define ROOMBA_H

#define ROOMBA_DEVICE_TYPE 0

#ifdef __cplusplus
extern "C" {
#endif

enum _mode {
    MODE_UNKNOWN = 0,
    MODE_PASSIVE = 1,
    MODE_SAFE	 = 2,
    MODE_FULL	 = 3,
};
typedef enum _mode mode;

enum _opcodes {
    START	= 128,
    BAUD	= 129,
    CONTROL	= 130,
    SAFE	= 131,
    FULL	= 132,
    POWER	= 133,
    SPOT	= 134,
    CLEAN	= 135,
    MAX		= 136,
    DRIVE	= 137,
    MOTORS	= 138,
    LEDS	= 139,
    SONG	= 140,
    PLAY	= 141,
    SENSORS	= 142,
    DOCS	= 143,
    PWMMOTORS	= 144,
    DRIVEWHEELS	= 145,
    DRIVEPWM	= 146,
    STREAM	= 148,
    QUERYLIST	= 149,
    STOPSTARTSTREAM	= 150,
    SCHEDULINGLEDS	= 162,
    DIGITLEDSRAW	= 163,
    DIGITLEDSASCII	= 164,
    BUTTONSCMD	= 165,
    SCHEDULE	= 167,
    SETDAYTIME	= 168,
};
typedef enum _opcodes opcodes;

enum _sensor_bytes {
    BUMPSWHEELDROPS	= 0,
    WALL	= 1,
    CLIFFLEFT	= 2,
    CLIFFFRONTLEFT	= 3,
    CLIFFFRONTRIGHT	= 4,
    CLIFFRIGHT	= 5,
    VIRTUALWALL	= 6,
    MOTOROVERCURRENTS	= 7,
    DIRTLEFT	= 8,
    DIRTRIGHT	= 9,
    REMOTEOPCODE	= 10,
    BUTTONS	= 11,
    DISTANCE_HI	= 12,
    DISTANCE_LO	= 13,
    ANGLE_HI	= 14,
    ANGLE_LO	= 15,
    CHARGINGSTATE	= 16,
    VOLTAGE_HI	= 17,
    VOLTAGE_LO	= 18,
    CURRENT_HI	= 19,
    CURRENT_LO	= 20,
    TEMPERATURE	= 21,
    CHARGE_HI	= 22,
    CHARGE_LO	= 23,
    CAPACITY_HI	= 24,
    CAPACITY_LO	= 25,
};
typedef enum _sensor_bytes sensor_bytes;

#define WHEELDROP_MASK		0x1C
#define BUMP_MASK		0x03
#define BUMPRIGHT_MASK		0x01
#define BUMPLEFT_MASK		0x02
#define WHEELDROPRIGHT_MASK	0x04
#define WHEELDROPLEFT_MASK	0x08
#define WHEELDROPCENT_MASK	0x10

#define MOTORDRIVELEFT_MASK	0x10
#define MOTORDRIVERIGHT_MASK	0x08
#define MOTORMAINBRUSH_MASK	0x04
#define MOTORVACUUM_MASK	0x02
#define MOTORSIDEBRUSH_MASK	0x01

#define POWERBUTTON_MASK	0x08
#define STOPBUTTON_MASK		0x04
#define CLEANBUTTON_MASK	0x02
#define MAXBUTTON_MASK		0x01

enum _sensors {
    SENSORS_ALL		= 0,
    SENSORS_PHYSICAL	= 1,
    SENSORS_INTERNAL	= 2,
    SENSORS_POWER	= 3,
    SENSORS_WALL	= 4,
    SENSORS_MODE	= 5,
    SENSORS_ALLMODE	= 6,
    SENSORS_WHOLE	= 100,
    SENSORS_ETC		= 101,
    SENSORS_LIGHTBUMPER	= 106,
    SENSORS_MOTORCURRENT= 107,
};
typedef enum _sensors sensors;

enum _remote {
    REMOTE_NONE		= 0xff,
    REMOTE_POWER	= 0x8a,
    REMOTE_PAUSE	= 0x89,
    REMOTE_CLEAN	= 0x88,
    REMOTE_MAX		= 0x85,
    REMOTE_SPOT		= 0x84,
    REMOTE_SPINLEFT	= 0x83,
    REMOTE_FORWARD	= 0x82,
    REMOTE_SPINRIGHT	= 0x81,
};
typedef enum _remote remote;

int roomba_init(device_argument *args, int id);
int roomba_stop(int id);
int roomba_run(int id);
int roomba_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output);

#ifdef __cplusplus
};
#endif


#endif
