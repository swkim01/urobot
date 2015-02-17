#include <iostream>
#include <kobuki_driver/kobuki.hpp>
#include <kobuki_driver/packets/core_sensors.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "urobot.h"
#include "target.h"
#include "device.h"
#include "services.h"
#include "mcs.h"
#include "sms.h"

#include "utils.h"
#include "devices/kobuki.h"

#define KOBUKI_NAME "Kobuki Actuator"

#define USBSERIAL	"/dev/ttyUSB0"

/* Car Status Check */
int go = 0;
int back = 0;
int dm_right = 0;
int dm_left = 0;
int breakl = 0;

int kobuki_fd;

static void help(void);

/* globals */
static struct urobot_t *ut;
static struct device_t *dev;

kobuki::Kobuki kobuki_robot;
kobuki::Parameters parameters;

void drive(double velocity, double angular) {
    kobuki_robot.setBaseControl(velocity, angular);
}

int send_byte(unsigned char send_data)
{
	int value;
	value = write(kobuki_fd, &send_data, 1);
	return value;
}

int send_bytes(unsigned char *send_data, int size)
{
	int value;
	value = write(kobuki_fd, send_data, size);
	return value;
}

void stop() {
	drive(0, 0);
}

void go_forward() {
	drive(0.3, 0);
}

void go_backward() {
	drive(-0.3, 0);
}

void turn_left() {
	drive(0.3, 3.3);
}

void turn_right() {
	drive(0.3, -3.3);
}

extern "C" int set_motion(int cmd, int value)
{
	int result = 0;

	switch(cmd){
	case FORWARD_MOTION:
		if (go == 0) {
			usleep(10000);
			go = 1;
			back = 0;
			go_forward();
			printf("GoForward motion was set\n");
		}
		else {
			go = 0;
			back = 0;
			stop();
			printf("GoForward motion was not set\n");
		}
		result = go;
		break;
	case RIGHTTURN_MOTION:
		if(dm_right == 0){
			dm_right = 1;
			dm_left = 0;
			turn_right();
			printf("TurnRight motion was set\n");
		}
		else{
			dm_right = 0;
			stop();
			printf("TurnRight motion was not set\n");
		}
		result = dm_right;
		break;
	case BACKWARD_MOTION:
		if(back == 0){
			usleep(10000);
			back = 1;
			go = 0;
			go_backward();
			printf("GoBackward motion was set\n");
		}
		else{
			back = 0;
			go = 0;
			stop();
			printf("GoBackward motion was not set\n");
		}
		result = back;
		break;
	case LEFTTURN_MOTION:
		if(dm_left == 0){
			dm_right = 0;
			dm_left = 1;
			turn_left();
			printf("TurnLeft motion was set\n");
		}
		else{
			dm_left = 0;
			stop();
			printf("TurnLeft motion was not set\n");
		}
		result = dm_left;
		break;
	case STOP_MOTION:
		back = 0;
		go = 0;
		dm_right = 0;
		dm_left = 0;
		stop();
		break;
	}
	return result;
}

extern "C" void set_basecontrol(float velocity, float angular)
{
	drive(velocity, angular);
}

void read_sensors() {
}

int get_sensor_data(int type, unsigned short *send_data)
{
	int result = 0;

	usleep(5000);

	kobuki::CoreSensors::Data data = kobuki_robot.getCoreSensorData();
	send_data[0] = data.battery;
	if (result != 26) {
		printf("Success to read sensor data!\n");
		return 1;
	} else {
		printf("Success to read sensor data!\n");
		return 0;
	}
}

static int sensor_thread_run = 0;
static char values[7][20];
static char *pvalues[7] = {values[0], values[1], values[2], values[3], values[4], values[5], values[6]};
void *sensor_thread(void *arg)
{
	int sleeptime = 2;
	unsigned char send_data[26];
	char value[20];

	while (sensor_thread_run) {
		sleep((unsigned int)sleeptime);

		get_sensor_data(DISTANCE_SENSOR, (unsigned short *)send_data);
		/*
		sprintf(value, "%d", send_data[1]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_FRONT_DISTANCE, value);
		sprintf(value, "%d", send_data[2]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_BACK_DISTANCE, value);
		sprintf(value, "%d", send_data[3]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_LEFT_DISTANCE, value);
		sprintf(value, "%d", send_data[4]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_RIGHT_DISTANCE, value);
		*/
		sprintf(values[0], "%d", send_data[BUMPER]);
		sprintf(values[1], "%d", send_data[WHEELDROP]);
		sprintf(values[2], "%d", send_data[CLIFF]);

		/*
		sprintf(value, "%d", send_data[1]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_ACCELERATOR_X, value);
		sprintf(value, "%d", send_data[2]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_ACCELERATOR_Y, value);
		sprintf(value, "%d", send_data[3]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_ACCELERATOR_Z, value);
		*/
		sprintf(values[3], "%d", (short)((send_data[LEFTENCODER_HI] << 8) | (send_data[LEFTENCODER_LO] & 0xff)));
		sprintf(values[4], "%d", (short)((send_data[RIGHTENCODER_HI] << 8) | (send_data[RIGHTENCODER_LO] & 0xff)));
		sprintf(values[5], "%d", send_data[CHARGER] & 0xff);
		sprintf(values[6], "%d", send_data[BATTERY] & 0xff);
		//printf("values=%s\n", *values);
		//upnp_set_variables(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_FRONT_DISTANCE, pvalues, 7);
		
	}

	return NULL;
}

extern "C" int kobuki_init(device_argument *args, int id)
{
    int i;
    char *devname = NULL;

    args->argv[0] = KOBUKI_NAME;
    ut = args->ut;
    dev = args->dev;

    /* show all parameters for DBG purposes */
    for(i = 0; i < args->argc; i++) {
        DBG("argv[%d]=%s\n", i, args->argv[i]);
    }

    /* parse the parameters */
    reset_getopt();
    while(1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"h", no_argument, 0, 0},
            {"help", no_argument, 0, 0},
            {"d", required_argument, 0, 0},
            {"device", required_argument, 0, 0},
            {"n", no_argument, 0, 0},
            {0, 0, 0, 0}
        };

        /* parsing all parameters according to the list above is sufficent */
        c = getopt_long_only(args->argc, args->argv, "", long_options, &option_index);

        /* no more options to parse */
        if(c == -1) break;

        /* unrecognized option */
        if(c == '?') {
            help();
            return 1;
        }

        /* dispatch the given options */
        switch(option_index) {
            /* h, help */
        case 0:
        case 1:
            DBG("case 0,1\n");
            help();
            return 1;
            break;

            /* d, device */
        case 2:
        case 3:
            DBG("case 2,3\n");
            devname = strdup(optarg);
            break;

        default:
            DBG("default case\n");
            help();
            return 1;
        }
    }
    DBG("input id: %d\n", id);

    // change the default device port from /dev/kobuki to /dev/ttyUSB0
    parameters.device_port = (devname ? devname : USBSERIAL);
    // Other parameters are typically happy enough as defaults
    // namespaces all sigslot connection names under this value, only important if you want to
    parameters.sigslots_namespace = "/kobuki";
    // Most people will prefer to do their own velocity smoothing/acceleration limiting.
    // If you wish to utilise kobuki's minimal acceleration limiter, set to true
    parameters.enable_acceleration_limiter = false;
    kobuki_robot.init(parameters);
    kobuki_robot.enable();

    return 0;
}

/*** private functions for this plugin below ***/
/******************************************************************************
Description.: print a help message to stderr
Input Value.: -
Return Value: -
******************************************************************************/
static void help(void)
{
    fprintf(stderr, " ---------------------------------------------------------------\n" \
    " Help for virtual car device..: "KOBUKI_NAME"\n" \
    " ---------------------------------------------------------------\n" \
    " The following parameters can be passed to this plugin:\n\n" \
    " [-d | --device ].......: car device to open\n" \
    " ---------------------------------------------------------------\n\n");
}

extern "C" int kobuki_stop(int id)
{
    kobuki_robot.setBaseControl(0,0); // linear_velocity, angular_velocity in (m/s), (rad/s)
    kobuki_robot.disable();

    return 0;
}

extern "C" int kobuki_run(int id)
{
    return 0;
}

extern "C" int kobuki_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
{
    int ret = -1;
    int i = 0, command;
    float velocity, angular;
    device *dev= (device *) service->dev;
    //v4l2priv *priv = (v4l2priv *) dev->priv_data;

    if (strstr(service->type, "MotionController")) {
        switch(command_id) {
        case FORWARD_MOTION:
            command = input[0].i;
            set_motion(FORWARD_MOTION, command);
            break;
        case BACKWARD_MOTION:
            command = input[0].i;
            set_motion(BACKWARD_MOTION, command);
            break;
        case STOP_MOTION:
            command = input[0].i;
            set_motion(STOP_MOTION, command);
            break;
        case LEFTTURN_MOTION:
            command = input[0].i;
            set_motion(LEFTTURN_MOTION, command);
            break;
        case RIGHTTURN_MOTION:
            command = input[0].i;
            set_motion(RIGHTTURN_MOTION, command);
            break;
        case MOVE_MOTION:
            velocity = input[0].f;
            angular = input[1].f;
            set_basecontrol(velocity, angular);
            break;
        }
    }
    else if (strstr(service->type, "SensorMonitoring")) {
        switch(command_id) {
        }
    }

    //DBG("Requested cmd (id: %d) for the %d plugin. Group: %d value: %d\n", control_id, plugin_number, group, value);

    return 0;
}
