#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include "urobot.h"
#include "target.h"
#include "device.h"
#include "services.h"
#include "mcs.h"
#include "css.h"
#include "sms.h"

#include "utils.h"

#define ADCID		0x00AD
#define ACCID		0x00AC
#define CAMID		0x0C

#define VIRTUAL_CAR_NAME "Virtual Car Actuator"

static void help(void);

/* Car Status Check */
static int go = 0;
static int back = 0;
static int left = 0;
static int right = 0;
static int fog = 0;
static int front = 0;
static int emer = 0;
static int dm_right = 0;
static int dm_left = 0;
static int breakl = 0;

/* globals */
static struct urobot_t *ut;
static struct device_t *dev;

int init_sensor(void);

int get_adc_data(unsigned short *send_data)
{
	int i;

	for(i=1; i<5; i++)
		send_data[i] = (unsigned short)(1000*(rand()/(RAND_MAX+1.0)));
	send_data[0] = ADCID;
			
	return 0;
}

int get_acc_data(unsigned short *send_data)
{
	int i;

	for(i=1;i<4;i++)
		send_data[i] = (unsigned short)(1000*(rand()/(RAND_MAX+1.0)));
	send_data[4] = 0x00; //dummy
	send_data[0] = ACCID;

	return 0;
}

int get_sensor_data(int type, unsigned short *send_data)
{
	if (type == LIGHT_SENSOR) {
		send_data[0] = front;
		send_data[1] = fog;
		send_data[2] = breakl;
		send_data[3] = (emer << 2) + (right << 1) + left;
		return 0;
	}
	else if (type == ACCELERATOR_SENSOR)
		return get_acc_data(send_data);

	else if (type == DISTANCE_SENSOR)
		return get_adc_data(send_data);

	else
		return -1;
}
/*
static int sensor_thread_run = 0;
static char values[7][20];
static char *pvalues[7] = {values[0], values[1], values[2], values[3], values[4], values[5], values[6]};
void *sensor_thread(void *arg)
{
	int sleeptime = 2;
	unsigned short send_data[5];
	char value[20];

	while (sensor_thread_run) {
		sleep((unsigned int)sleeptime);

		get_adc_data(send_data);

		sprintf(values[0], "%d", send_data[1]);
		sprintf(values[1], "%d", send_data[2]);
		sprintf(values[2], "%d", send_data[3]);
		sprintf(values[3], "%d", send_data[4]);

		get_acc_data(send_data);
		sprintf(values[4], "%d", send_data[1]);
		sprintf(values[5], "%d", send_data[2]);
		sprintf(values[6], "%d", send_data[3]);
		printf("values=%s\n", *values);
		upnp_set_variables(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_FRONT_DISTANCE, pvalues, 7);
	}

	return NULL;
}
*/
int set_sensor_data(int type, int index, int value)
{
	char cmd_buf;
	int result = 0;

	if (type != LIGHT_SENSOR)
		return -1;

	switch(index){
	case FRONT_LIGHT_SENSOR:
		front = (front == 0) ? 1 : 0;
		printf("Front light sets to be %s\n", front ? "ON":"OFF");
		result = front;
		break;
	case FOG_LIGHT_SENSOR:
		fog = (fog == 0) ? 1 : 0;
		printf("Fog light sets to be %s\n", fog ? "ON":"OFF");
		result = fog;
		break;
	case BREAK_LIGHT_SENSOR:
		breakl = (breakl == 0) ? 1 : 0;
		printf("Break light sets to be %s\n", breakl ? "ON":"OFF");
		result = breakl;
		break;
	case WINKER_LIGHT_SENSOR:
		if (value == 0) {
			emer = (emer == 0) ? 1 : 0;
			printf("Emergency light sets to be %s\n", emer ? "ON":"OFF");
			result = emer;
		}
		else if (value == 1) {
			left = (left == 0) ? 1 : 0;
			printf("Leftturn light sets to be %s\n", emer ? "ON":"OFF");
			result = left;
		}
		else if (value == 2) {
			right = (right == 0) ? 1 : 0;
			printf("Rightturn light sets to be %s\n", emer ? "ON":"OFF");
			result = right;
		}
		break;
	}
	return result;
}

void set_basecontrol(float velocity, float angular)
{
	int result = 0;
	printf("Move motion was set with velocity=%f, angular=%f\n", velocity, angular);
}

int set_motion(int cmd, int value)
{
	char cmd_buf;
	int result = 0;

	switch(cmd){
	case FORWARD_MOTION:
		if (go == 0) {
			usleep(10000);
			go = 1;
			back = 0;
			printf("GoForward motion was set\n");
		}
		else {
			go = 0;
			back = 0;
			printf("GoForward motion was not set\n");
		}
		result = go;
		break;
	case RIGHTTURN_MOTION:
		if(dm_right == 0){
			dm_right = 1;
			dm_left = 0;
			printf("TurnRight motion was set\n");
		}
		else{
			dm_right = 0;
			printf("TurnRight motion was not set\n");
		}
		result = dm_right;
		break;
	case BACKWARD_MOTION:
		if(back == 0){
			usleep(10000);
			back = 1;
			go = 0;
			printf("GoBackward motion was set\n");
		}
		else{
			back = 0;
			go = 0;
			printf("GoBackward motion was not set\n");
		}
		result = back;
		break;
	case LEFTTURN_MOTION:
		if(dm_left == 0){
			dm_right = 0;
			dm_left = 1;
			printf("TurnLeft motion was set\n");
		}
		else{
			dm_left = 0;
			printf("TurnLeft motion was not set\n");
		}
		result = dm_left;
		break;
	case STOP_MOTION:
		back = 0;
		go = 0;
		dm_right = 0;
		dm_left = 0;
		printf("Stop motion was set\n");
		break;
	}
	return result;
}

int car_init(device_argument *args, int id)
{
    int i;
    char *devname;

    args->argv[0] = VIRTUAL_CAR_NAME;
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
    " Help for virtual car device..: "VIRTUAL_CAR_NAME"\n" \
    " ---------------------------------------------------------------\n" \
    " The following parameters can be passed to this plugin:\n\n" \
    " [-d | --device ].......: car device to open\n" \
    " ---------------------------------------------------------------\n\n");
}

int car_stop(int id)
{
    return 0;
}

int car_run(int id)
{
    return 0;
}

int car_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
{
    int ret = -1;
    int i = 0, command;
    float velocity, angular;
    unsigned short sensors[5];
    device *dev= (device *) service->dev;

    if (strstr(service->type, "MotionController")) {
        switch(command_id) {
        case FORWARD_MOTION:
            command = input[0].i;
            printf("command=%d\n", command);
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
        case FRONT_DISTANCE:
            command = input[0].i;
            get_sensor_data(DISTANCE_SENSOR, sensors);
            output[0].b = sensors[1];
            break;
        case BACK_DISTANCE:
            command = input[0].i;
            get_sensor_data(DISTANCE_SENSOR, sensors);
            output[0].b = sensors[2];
            break;
        case LEFT_DISTANCE:
            command = input[0].i;
            get_sensor_data(DISTANCE_SENSOR, sensors);
            output[0].b = sensors[3];
            break;
        case RIGHT_DISTANCE:
            command = input[0].i;
            get_sensor_data(DISTANCE_SENSOR, sensors);
            output[0].b = sensors[4];
            break;
        case GET_DISTANCES:
            command = input[0].i;
            get_sensor_data(DISTANCE_SENSOR, sensors);
            output[0].b = sensors[1];
            output[1].b = sensors[2];
            output[2].b = sensors[3];
            output[3].b = sensors[4];
            break;
        case GET_ACCELERATOR:
            command = input[0].i;
            get_sensor_data(ACCELERATOR_SENSOR, sensors);
            output[0].b = sensors[1];
            output[1].b = sensors[2];
            output[2].b = sensors[3];
            break;
        }
    }

    //DBG("Requested cmd (id: %d) for the %d plugin. Group: %d value: %d\n", control_id, plugin_number, group, value);

    return 0;
}

/*
int car_cmd(struct service_t *service, unsigned int command_id, unsigned int nargs, ...)
{
    int ret = -1;
    int i = 0, command;
    float velocity, angular;
    //v4l2priv *priv = (v4l2priv *) dev->priv_data;
    va_list ap;

    va_start(ap, nargs);
    //for (i=0; i < nargs; i++) {
    //    va_arg(ap, int);
    //}

    if (strstr(service->type, "MotionController")) {
        switch(command_id) {
        case FORWARD_MOTION:
            command = va_arg(ap, int);
            set_motion(FORWARD_MOTION, command);
            break;
        case BACKWARD_MOTION:
            command = va_arg(ap, int);
            set_motion(BACKWARD_MOTION, command);
            break;
        case STOP_MOTION:
            command = va_arg(ap, int);
            set_motion(STOP_MOTION, command);
            break;
        case LEFTTURN_MOTION:
            command = va_arg(ap, int);
            set_motion(LEFTTURN_MOTION, command);
            break;
        case RIGHTTURN_MOTION:
            command = va_arg(ap, int);
            set_motion(RIGHTTURN_MOTION, command);
            break;
        case MOVE_MOTION:
            velocity = (float)va_arg(ap, double);
            angular = (float)va_arg(ap, double);
            set_basecontrol(velocity, angular);
            break;
        }
    }
    else if (strstr(service->type, "SensorMonitoring")) {
        switch(command_id) {
        }
    }

    //DBG("Requested cmd (id: %d) for the %d plugin. Group: %d value: %d\n", control_id, plugin_number, group, value);
    va_end(ap);

    return 0;
}
*/
