/*
Copyright (c) 2012 Ben Croston / 2012-2013 Eric PTAK

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "urobot.h"
#include "target.h"
#include "device.h"
#include "services.h"
#include "mcs.h"
#include "css.h"

#include "utils.h"
#include "devices/raspberrypi.h"

#define RASPBERRYPI_NAME "RaspberryPi Actuator"

static volatile unsigned int BCM2708_PERI_BASE = 0x20000000; // variable for Pi2
#define GPIO_BASE           (BCM2708_PERI_BASE + 0x200000)
#define FSEL_OFFSET         0   // 0x0000
#define SET_OFFSET          7   // 0x001c / 4
#define CLR_OFFSET          10  // 0x0028 / 4
#define PINLEVEL_OFFSET     13  // 0x0034 / 4
#define EVENT_DETECT_OFFSET 16  // 0x0040 / 4
#define RISING_ED_OFFSET    19  // 0x004c / 4
#define FALLING_ED_OFFSET   22  // 0x0058 / 4
#define HIGH_DETECT_OFFSET  25  // 0x0064 / 4
#define LOW_DETECT_OFFSET   28  // 0x0070 / 4
#define PULLUPDN_OFFSET     37  // 0x0094 / 4
#define PULLUPDNCLK_OFFSET  38  // 0x0098 / 4

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)

static volatile uint32_t *gpio_map;

struct tspair {
	struct timespec up;
	struct timespec down;
};

static struct pulse gpio_pulses[GPIO_COUNT];
static struct tspair gpio_tspairs[GPIO_COUNT];
static pthread_t *gpio_threads[GPIO_COUNT];

void short_wait(void)
{
    int i;
    
    for (i=0; i<150; i++)     // wait 150 cycles
    {
		asm volatile("nop");
    }
}

int setup(void)
{
    FILE *cpuFd;
    char line[120];
    int mem_fd;
    uint8_t *gpio_mem;

    if ((cpuFd = fopen ("/proc/cpuinfo", "r")) == NULL)
        return SETUP_HARDWARE_FAIL;

    // Start by looking for the Architecture, then we can look for a B2 revision....
    while (fgets (line, 120, cpuFd) != NULL)
        if (strncmp (line, "Hardware", 8) == 0)
            break ;

    if (strncmp (line, "Hardware", 8) != 0) {
        fclose (cpuFd);
        return SETUP_HARDWARE_FAIL;
    }

    // See if it's BCM2708 or BCM2709
    if (strstr (line, "BCM2709") != NULL) {
        BCM2708_PERI_BASE = 0x3F000000 ; // Pi2
    } else if (strstr (line, "BCM2708") == NULL) {
        fclose (cpuFd);
        return SETUP_HARDWARE_FAIL;
    }
    fclose (cpuFd);

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
    {
        return SETUP_DEVMEM_FAIL;
    }

    if ((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL)
        return SETUP_MALLOC_FAIL;

    if ((uint32_t)gpio_mem % PAGE_SIZE)
        gpio_mem += PAGE_SIZE - ((uint32_t)gpio_mem % PAGE_SIZE);

    gpio_map = (uint32_t *)mmap( (caddr_t)gpio_mem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, mem_fd, GPIO_BASE);

    if ((uint32_t)gpio_map < 0)
        return SETUP_MMAP_FAIL;

    return SETUP_OK;
}

void set_pullupdn(int gpio, int pud)
{
    int clk_offset = PULLUPDNCLK_OFFSET + (gpio/32);
    int shift = (gpio%32);
    
    if (pud == PUD_DOWN)
       *(gpio_map+PULLUPDN_OFFSET) = (*(gpio_map+PULLUPDN_OFFSET) & ~3) | PUD_DOWN;
    else if (pud == PUD_UP)
       *(gpio_map+PULLUPDN_OFFSET) = (*(gpio_map+PULLUPDN_OFFSET) & ~3) | PUD_UP;
    else  // pud == PUD_OFF
       *(gpio_map+PULLUPDN_OFFSET) &= ~3;
    
    short_wait();
    *(gpio_map+clk_offset) = 1 << shift;
    short_wait();
    *(gpio_map+PULLUPDN_OFFSET) &= ~3;
    *(gpio_map+clk_offset) = 0;
}

//updated Eric PTAK - trouch.com
void set_function(int gpio, int function, int pud)
{
	if (function == PWM) {
		function = OUTPUT;
		enablePWM(gpio);
	}
	else {
		disablePWM(gpio);
	}

    int offset = FSEL_OFFSET + (gpio/10);
    int shift = (gpio%10)*3;

    set_pullupdn(gpio, pud);
	*(gpio_map+offset) = (*(gpio_map+offset) & ~(7<<shift)) | (function<<shift);
}

//added Eric PTAK - trouch.com
int get_function(int gpio)
{
   int offset = FSEL_OFFSET + (gpio/10);
   int shift = (gpio%10)*3;
   int value = *(gpio_map+offset);
   value >>= shift;
   value &= 7;
   if ((value == OUTPUT) && isPWMEnabled(gpio)) {
	   value = PWM;
   }
   return value; // 0=input, 1=output, 4=alt0
}

//updated Eric PTAK - trouch.com
int input(int gpio)
{
   int offset, value, mask;

   offset = PINLEVEL_OFFSET + (gpio/32);
   mask = (1 << gpio%32);
   value = *(gpio_map+offset) & mask;
   return value;
}

void output(int gpio, int value)
{
    int offset, shift;
    
    if (value) // value == HIGH
        offset = SET_OFFSET + (gpio/32);
    else       // value == LOW
        offset = CLR_OFFSET + (gpio/32);
    
    shift = (gpio%32);

    *(gpio_map+offset) = 1 << shift;
}

//added Eric PTAK - trouch.com
void outputSequence(int gpio, int period, char* sequence) {
	int i, value;
	struct timespec ts;
	ts.tv_sec = period/1000;
	ts.tv_nsec = (period%1000) * 1000000;

	for (i=0; sequence[i] != '\0'; i++) {
		if (sequence[i] == '1') {
			value = 1;
		}
		else {
			value = 0;
		}
		output(gpio, value);
	    nanosleep(&ts, NULL);
	}
}

void resetPWM(int gpio) {
	gpio_pulses[gpio].type = 0;
	gpio_pulses[gpio].value = 0;

	gpio_tspairs[gpio].up.tv_sec = 0;
	gpio_tspairs[gpio].up.tv_nsec = 0;
	gpio_tspairs[gpio].down.tv_sec = 0;
	gpio_tspairs[gpio].down.tv_nsec = 0;
}

//added Eric PTAK - trouch.com
void pulseTS(int gpio, struct timespec *up, struct timespec *down) {
	if ((up->tv_sec > 0) || (up->tv_nsec > 0)) {
		output(gpio, 1);
		nanosleep(up, NULL);
	}

	if ((down->tv_sec > 0) || (down->tv_nsec > 0)) {
		output(gpio, 0);
		nanosleep(down, NULL);
	}
}

//added Eric PTAK - trouch.com
void pulseOrSaveTS(int gpio, struct timespec *up, struct timespec *down) {
	if (gpio_threads[gpio] != NULL) {
		memcpy(&gpio_tspairs[gpio].up, up, sizeof(struct timespec));
		memcpy(&gpio_tspairs[gpio].down, down, sizeof(struct timespec));
	}
	else {
		pulseTS(gpio, up, down);
	}
}

//added Eric PTAK - trouch.com
void pulseMilli(int gpio, int up, int down) {
	struct timespec tsUP, tsDOWN;

	tsUP.tv_sec = up/1000;
	tsUP.tv_nsec = (up%1000) * 1000000;

	tsDOWN.tv_sec = down/1000;
	tsDOWN.tv_nsec = (down%1000) * 1000000;
	pulseOrSaveTS(gpio, &tsUP, &tsDOWN);
}

//added Eric PTAK - trouch.com
void pulseMilliRatio(int gpio, int width, float ratio) {
	int up = ratio*width;
	int down = width - up;
	pulseMilli(gpio, up, down);
}

//added Eric PTAK - trouch.com
void pulseMicro(int gpio, int up, int down) {
	struct timespec tsUP, tsDOWN;

	tsUP.tv_sec = 0;
	tsUP.tv_nsec = up * 1000;

	tsDOWN.tv_sec = 0;
	tsDOWN.tv_nsec = down * 1000;
	pulseOrSaveTS(gpio, &tsUP, &tsDOWN);
}

//added Eric PTAK - trouch.com
void pulseMicroRatio(int gpio, int width, float ratio) {
	int up = ratio*width;
	int down = width - up;
	pulseMicro(gpio, up, down);
}

//added Eric PTAK - trouch.com
void pulseAngle(int gpio, float angle) {
	gpio_pulses[gpio].type = ANGLE;
	gpio_pulses[gpio].value = angle;
	int up = 1520 + (angle*400)/45;
	int down = 20000-up;
	pulseMicro(gpio, up, down);
}

//added Eric PTAK - trouch.com
void pulseRatio(int gpio, float ratio) {
	gpio_pulses[gpio].type = RATIO;
	gpio_pulses[gpio].value = ratio;
	int up = ratio * 20000;
	int down = 20000 - up;
	pulseMicro(gpio, up, down);
}

struct pulse* getPulse(int gpio) {
	return &gpio_pulses[gpio];
}

//added Eric PTAK - trouch.com
void* pwmLoop(void* data) {
	int gpio = (int)data;

	while (1) {
		pulseTS(gpio, &gpio_tspairs[gpio].up, &gpio_tspairs[gpio].down);
	}
}

//added Eric PTAK - trouch.com
void enablePWM(int gpio) {
	pthread_t *thread = gpio_threads[gpio];
	if (thread != NULL) {
		return;
	}

	resetPWM(gpio);

	thread = (pthread_t*) malloc(sizeof(pthread_t));
	pthread_create(thread, NULL, pwmLoop, (void*)gpio);
	gpio_threads[gpio] = thread;
}

//added Eric PTAK - trouch.com
void disablePWM(int gpio) {
	pthread_t *thread = gpio_threads[gpio];
	if (thread == NULL) {
		return;
	}

	pthread_cancel(*thread);
	gpio_threads[gpio] = NULL;
	output(gpio, 0);
	resetPWM(gpio);
}

//added Eric PTAK - trouch.com
int isPWMEnabled(int gpio) {
	return gpio_threads[gpio] != NULL;
}


void cleanup(void)
{
    // fixme - set all gpios back to input
    munmap((caddr_t)gpio_map, BLOCK_SIZE);
}

/* Left/Drive motor GPIOs */
#define L1 4 /* H-Bridge 1 */
#define DD 4 /* H-Bridge 1 */
#define L2 18 /* H-Bridge 2 */
#define LS 17 /* H-Bridge 1,2EN */
#define DG 17 /* H-Bridge 1,2EN */

/* Right/Front motor GPIOs */
#define R1 25 /* H-Bridge 3 */
#define FD 25 /* H-Bridge 3 */
#define R2 24 /* H-Bridge 4 */
#define RS 10 /* H-Bridge 3,4EN */
#define FG 10 /* H-Bridge 3,4EN */

#define LOW 0
#define HIGH 1

int mode = 1; // 0: differential-drive, 1: car-type

// Set the speed of two motors
void set_speed(float speed)
{
    pulseRatio(LS, speed);
    pulseRatio(RS, speed);
}

// -------------------------------------------------- #
// Left Motor Functions                               #
// -------------------------------------------------- #

void left_stop()
{
    output(L1, LOW);
    output(L2, LOW);
}

void left_forward()
{
    output(L1, HIGH);
    output(L2, LOW);
}

void left_backward()
{
    output(L1, LOW);
    output(L2, HIGH);
}

// -------------------------------------------------- #
// Right Motor Functions                              #
// -------------------------------------------------- #
void right_stop()
{
    output(R1, LOW);
    output(R2, LOW);
}

void right_forward()
{
    output(R1, HIGH);
    output(R2, LOW);
}

void right_backward()
{
    output(R1, LOW);
    output(R2, HIGH);
}

// -------------------------------------------------- #
// Macro definition part                              #
// -------------------------------------------------- #
void go_forward()
{
    if (mode == 0) {
        left_forward();
        right_forward();
    }
    else {
        left_forward();
        pulseRatio(DG, 1.0);
        pulseRatio(FG, 0.5);
    }
}

void go_backward()
{
    if (mode == 0) {
        left_backward();
        right_backward();
    }
    else {
        left_backward();
        pulseRatio(DG, 1.0);
        pulseRatio(FG, 0.5);
    }
}

void turn_left()
{
    if (mode == 0) {
        left_backward();
        right_forward();
    }
    else {
        right_forward();
        pulseRatio(FG, 1.0);
    }
}

void turn_right()
{
    if (mode == 0) {
        left_forward();
        right_backward();
    }
    else {
        right_backward();
        pulseRatio(FG, 1.0);
    }
}

void stop()
{
    if (mode == 0) {
        left_stop();
        right_stop();
    }
    else {
        left_stop();
        right_stop();
        pulseRatio(DG, 0.0);
        pulseRatio(FG, 0.0);
    }
}

void set_basecontrol(float speed, float angle)
{
    float dir, v, omega;
    if (speed < 0.05)
        angle = M_PI / 2;
    if (mode == 0) {
    }
    else {
        if (angle > 0)
            left_backward();
        else
            left_forward();
        v = fabs(speed)/10;
        pulseRatio(DG, v);
        dir = abs(angle) - M_PI / 2;
        if (dir > 0)
            right_forward();
        else
            right_backward();
        if (v < 0.1)
            omega = 0;
        else
            omega = fabs(dir)*2/M_PI;
        pulseRatio(FG, omega);
    }
}

static void help(void);

/* globals */
static struct urobot_t *ut;
static struct device_t *dev;

int raspberrypi_init(device_argument *args, int id)
{
    int i;
    char *devname;

    args->argv[0] = RASPBERRYPI_NAME;
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

    if (setup() != SETUP_OK) {
        DBG("Unable to recognize hardware\n");
        exit(EXIT_FAILURE);
    };
    set_function(L1, OUTPUT, PUD_OFF);
    set_function(LS, PWM, PUD_OFF);
    set_function(R1, OUTPUT, PUD_OFF);
    set_function(RS, PWM, PUD_OFF);

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
    " Help for raspberrypi device..: "RASPBERRYPI_NAME"\n" \
    " ---------------------------------------------------------------\n" \
    " The following parameters can be passed to this plugin:\n\n" \
    " [-p | --port ].......: raspberrypi gpio ports to control\n" \
    " ---------------------------------------------------------------\n\n");
}

int raspberrypi_stop(int id)
{
    return 0;
}

int raspberrypi_run(int id)
{
    return 0;
}

int raspberrypi_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
{
    int ret = -1;
    int i = 0, command;
    float velocity, angular;
    device *dev= (device *) service->dev;

    if (strstr(service->type, "MotionController")) {
        switch(command_id) {
        case FORWARD_MOTION:
            command = input[0].i;
            go_forward();
            break;
        case BACKWARD_MOTION:
            command = input[0].i;
            go_backward();
            break;
        case STOP_MOTION:
            command = input[0].i;
            stop();
            break;
        case LEFTTURN_MOTION:
            command = input[0].i;
            turn_left();
            break;
        case RIGHTTURN_MOTION:
            command = input[0].i;
            turn_right();
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

    return 0;
}
