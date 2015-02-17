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
#include "devices/roomba.h"

#define ROOMBA_NAME "Roomba Actuator"

#define USBSERIAL	"/dev/ttyUSB0"

/* Car Status Check */
int go = 0;
int back = 0;
int left = 0;
int right = 0;
int fog = 0;
int front = 0;
int emer = 0;
int dm_right = 0;
int dm_left = 0;
int breakl = 0;

mode roomba_mode = MODE_UNKNOWN;
int roomba_fd;

static void help(void);

/* globals */
static struct urobot_t *ut;
static struct device_t *dev;
struct termios oldtio;

int open_serial(char *devname)
{
        struct termios newtio;
        int fd;

        if (devname == NULL)
            fd = open(USBSERIAL, O_RDWR | O_NOCTTY );
        else
            fd = open(devname, O_RDWR | O_NOCTTY );

        if (fd < 0){
                perror("The roomba device is not Exist!");
                exit(EXIT_FAILURE);
        }
        tcgetattr(fd, &oldtio);
        bzero(&newtio, sizeof(newtio));

        newtio.c_cflag = B115200|CS8|CLOCAL|CREAD;

        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN] = 1;

        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);

        return fd;
}

void close_serial()
{
	close(roomba_fd);
}

int check_target(void)
{
        struct termios newtio;
        int fd;
        unsigned char cmd[2] = { (unsigned char)SENSORS,
                                (unsigned char)SENSORS_ALL };
        unsigned char read_buf[32];

        fd = open(USBSERIAL, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd < 0){
                perror("/dev/ttyUSB) is not Exist!");
                exit(EXIT_FAILURE);
        }
        tcgetattr(fd, &oldtio);
        bzero(&newtio, sizeof(newtio));

        newtio.c_cflag = B115200|CS8|CLOCAL|CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME] = 20;
        newtio.c_cc[VMIN] = 0;

        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);

        write(fd, cmd, 2);
        usleep(1000);
        read(fd, read_buf, 26);

        close(fd);
        return 0;
}

int send_byte(unsigned char send_data)
{
	int value;
	value = write(roomba_fd, &send_data, 1);
	return value;
}

int send_bytes(unsigned char *send_data, int size)
{
	int value;
	value = write(roomba_fd, send_data, size);
	return value;
}

void start_roomba()
{
	roomba_mode = MODE_PASSIVE;
	send_byte(START);
}

void control_roomba()
{
	roomba_mode = MODE_SAFE;
	send_byte(CONTROL);
}

void safe_roomba()
{
	roomba_mode = MODE_SAFE;
	send_byte(SAFE);
}

void full_roomba()
{
	roomba_mode = MODE_FULL;
	send_byte(FULL);
}

void poweroff_roomba()
{
	roomba_mode = MODE_UNKNOWN;
	send_byte(POWER);
}

void clean_roomba()
{
	roomba_mode = MODE_PASSIVE;
	send_byte(CLEAN);
}

void drive(int velocity, int radius ) {
        int result;
        unsigned char cmd[5] = { (unsigned char)DRIVE,
				(unsigned char)(velocity>>8),
				(unsigned char)(velocity&0xff),
				(unsigned char)(radius >> 8),
				(unsigned char)(radius & 0xff) };
        result = send_bytes (cmd, 5);
}

void set_basecontrol(float velocity, float radius)
{
	drive((int)velocity, (int) radius);
}

void stop() {
	drive(0, 0);
}

void go_forward() {
	drive(200, 0x8000);
}

void go_backward() {
	drive(-200, 0x8000);
}

void turn_left() {
	drive(200, 129);
}

void turn_right() {
	drive(200, -129);
}

int set_motion(int cmd, int value)
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

void read_sensors() {
        int result;
        unsigned char cmd[2] = { (unsigned char)SENSORS,
                                (unsigned char)SENSORS_ALL };
        unsigned char buf[32];
        result = send_bytes (cmd, 2);
        usleep(1000);
        result = read(roomba_fd, buf, sizeof(buf));
        /*
        if (result == 26)
                printf("bump: %c%c wheel: %c%c%c wall: %c cliff: %c%c%c%c vwal: %d motr: %d dirtL: %d dirtR: %d remo: %x butt: %x dist: %d angl: %d chst: %d volt: %d curr: %d temp: %d chrg: %d capa: %d\n",
                        (buf[BUMPSWHEELDROPS] & BUMPLEFT_MASK)?"l":"_",
                        (buf[BUMPSWHEELDROPS] & BUMPRIGHT_MASK)?"r":"_",
                        (buf[BUMPSWHEELDROPS] & WHEELDROPLEFT_MASK)?"l":"_",
                        (buf[BUMPSWHEELDROPS] & WHEELDROPCENT_MASK)?"c":"_",
                        (buf[BUMPSWHEELDROPS] & WHEELDROPRIGHT_MASK)?"r":"_",
                        buf[WALL]?"Y":"n",
                        buf[CLIFFLEFT]?"l":"_",
                        buf[CLIFFFRONTLEFT]?"L":"_",
                        buf[CLIFFFRONTRIGHT]?"R":"_",
                        buf[CLIFFRIGHT]?"r":"_",
                        buf[VIRTUALWALL],
                        buf[MOTOROVERCURRENTS],
                        buf[DIRTLEFT] & 0xff,
                        buf[DIRTRIGHT] & 0xff,
                        buf[REMOTEOPCODE],
                        buf[BUTTONS],
                        (short)((buf[DISTANCE_HI] << 8) | (buf[DISTANCE_LO] & 0xff)),
                        (short)((buf[ANGLE_HI] << 8) | (buf[ANGLE_LO] & 0xff)),
                        buf[CHARGINGSTATE] & 0xff,
                        (int)(buf[VOLTAGE_HI] & 0xff) << 8 | buf[VOLTAGE_LO] & 0xff,
                        (short)((buf[CURRENT_HI] << 8) | (buf[CURRENT_LO] & 0xff)),
                        buf[TEMPERATURE],
                        (int)(buf[CHARGE_HI] & 0xff) << 8 | buf[CHARGE_LO] & 0xff,
                        (int)(buf[CAPACITY_HI] & 0xff) << 8 | buf[CAPACITY_LO] & 0xff);
        */
}

int get_sensor_data(int type, unsigned short *send_data)
{
	int result = 0;
	unsigned char cmd[] = {(unsigned char)SENSORS, (unsigned char)SENSORS_ALL};
	result = send_bytes(cmd, 2);

	usleep(5000);

	result = read(roomba_fd, send_data, 26); // SENSOR READ
	if (result != 26) {
		printf("Success to read sensor data!\n");
		return 1;
	} else {
		printf("Success to read sensor data!\n");
		return 0;
	}
}

int get_sensor_lightbumper(int type, unsigned short *send_data)
{
	int result = 0;
	unsigned char cmd[] = {(unsigned char)SENSORS, (unsigned char)SENSORS_LIGHTBUMPER};
	result = send_bytes(cmd, 2);

	usleep(5000);

	result = read(roomba_fd, send_data, 12); // SENSOR READ
	if (result != 12) {
		printf("Success to read sensor lightbumper data!\n");
		return 1;
	} else {
		printf("Success to read sensor lightbumper data!\n");
		return 0;
	}
}

int roomba_init(device_argument *args, int id)
{
    int i;
    char *devname=NULL;

    args->argv[0] = ROOMBA_NAME;
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

    roomba_fd = open_serial(devname);
    //check_target();
    start_roomba();
    control_roomba();

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
    " Help for virtual car device..: "ROOMBA_NAME"\n" \
    " ---------------------------------------------------------------\n" \
    " The following parameters can be passed to this plugin:\n\n" \
    " [-d | --device ].......: car device to open\n" \
    " ---------------------------------------------------------------\n\n");
}

int roomba_stop(int id)
{
    close_serial();

    return 0;
}

int roomba_run(int id)
{
    return 0;
}

int roomba_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
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
