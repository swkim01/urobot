#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "urobot.h"
#include "target.h"

//#include "utils.h"
#include "turtlebot.h"
#include "roomba.h"

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

struct termios oldtio;

int open_serial(void)
{
        struct termios newtio;
        int fd;

        fd = open(USBSERIAL, O_RDWR | O_NOCTTY /*| O_NONBLOCK*/);
        if (fd < 0){
                perror("/dev/ttyUSB0 is not Exist!");
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
        int write_buf = 26;
        int     read_buf = 0;

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
        tcflush(fd, TCOFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);
	cfmakeraw(&newtio);

        //write(fd, &write_buf, 1);
        //read(fd, &read_buf, 1);

        //close(fd);
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
	/*printf("velhigh=%x, vellow=%x, radhigh=%x, radlow=%x\n", 
				(unsigned char)(velocity>>8),
				(unsigned char)(velocity&0xff),
				(unsigned char)(radius >> 8),
				(unsigned char)(radius & 0xff));
	*/
	result = send_bytes (cmd, 5);
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
}

int
main(void)
{ 
    int fd;
    int nread;
    char ch;
    struct termios init_attr, new_attr;

    roomba_fd = open_serial();
    //check_target();
    start_roomba();
    control_roomba();

    fd = open(ttyname(fileno(stdin)), O_RDWR);
    tcgetattr(fd, &init_attr);
    new_attr = init_attr;
    new_attr.c_lflag &= ~ICANON;
    new_attr.c_lflag &= ~ECHO; /* ~(ECHO | ECHOE | ECHOK | ECHONL); */
    new_attr.c_cc[VMIN] = 1;
    new_attr.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &new_attr) != 0) {
	fprintf(stderr, "터미널 속성을 설정할 수 없음.\n");
    }
    printf("Please input command key as follows.\n");
    printf("         i(forward)\n");
    printf("  j(left)  k(stop)  l(right)\n");
    printf("         m(backward)\n");
    printf("\nand \\n(exit)\n");
    while ((nread=read(fd, &ch, 1)) > 0 && ch != '\n') {
        if (ch == 'i')
            go_forward();
        else if (ch == 'j')
            turn_left();
        else if (ch == 'k')
            stop();
        else if (ch == 'l')
            turn_right();
        else if (ch == 'm')
            go_backward();
        else if (ch == 's')
            read_sensors();
    }
    tcsetattr(fd, TCSANOW, &init_attr);
    close(fd);

    close_serial();
}
