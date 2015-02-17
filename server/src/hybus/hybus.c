#include "hybus.h"
#include "car_lib.h"

//#include "userapp.h"
#include "urobot.h"
#include "target.h"
#include "services.h"
#include "sms.h"
#include "ithread.h"

//#include "mjpg_streamer.h"

struct termios oldtio;

/*
int handle;
int handle_overlay2;
struct pxa_video_buf* vidbuf;
struct pxacam_setting camset;
struct pxa_video_buf vidbuf_overlay;
int len;
*/



/*---------------- Thread Create ----------------*/
pthread_t cam_ThreadID;
void *cam_ThreadFunc(void *arg);

pthread_t sensor_thread_id;
void *sensor_thread(void *arg);

pthread_mutex_t Gloval_Mutex;

/*-----------------------------------------------*/
int uart_fd;
int adc_fd, acc_fd;

char cmd_buf;
char cam_buf;
unsigned short sen_buf;

int cam_sockfd = 0;
int ccam_sockfd = 0;
struct sockaddr_in ccam_address;

unsigned char *cur; //RGB Buffer
unsigned char *y12; //YUV420 Buffer
unsigned char *jpegd; // Jpeg Buffer

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

/*------------------*/

short speed = 0x01ee;
char set_speed[2];

/*
struct pxa_camera{
	int handle;
	int status;
	int mode;
	int sensor;
	int ref_count;

	// Video Buffer
	int width;
	int height;
	enum    pxavid_format format;
};
*/
int car_ctrl = 1; // MF or CAR Control Select

int camera_id = 0;
int camera_type = 0;
char camera_encoding[] = "multipart-jpeg";

/* globals */
static globals global;

void print_info(void)
{
	if(car_ctrl == 1)
		printf("Car Control Program\n");
	else
		printf("Robot Control Program\n");
}

int open_serial(void)
{
	struct termios newtio;
	int fd;

	fd = open(SERIAL2, O_RDWR | O_NOCTTY );
	if( fd < 0){
		perror("/dev/ttyS2 is not Exist!");
		return -1;
	}

	tcgetattr(fd, &oldtio);
	bzero(&newtio, sizeof(newtio));

	if( car_ctrl == 0 )
		newtio.c_cflag = B4800|CS8|CLOCAL|CREAD;
	else
		newtio.c_cflag = B19200|CS8|CLOCAL|CREAD;

	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
}

int check_target(void)
{
	struct termios newtio;
	int fd;
	int write_buf = 26;
	int	read_buf = 0;

	fd = open(SERIAL2, O_RDWR | O_NOCTTY );
	if( fd < 0){
		perror("/dev/ttyS2 is not Exist!");
		return -1;
	}

	tcgetattr(fd, &oldtio);
	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = B4800|CS8|CLOCAL|CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 20;
	newtio.c_cc[VMIN] = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	write(fd, &write_buf, 1);
	read(fd, &read_buf, 1);

	if(read_buf == 26)
		car_ctrl = 0;
	else
		car_ctrl = 1;

	print_info();
	close(fd);
	return 0;
}


/*
int init_camera(void)
{
	handle = camera_open(NULL,0);
	ASSERT(handle);

	memset(&camset,0,sizeof(camset));
	camset.mode = CAM_MODE_VIDEO;
	camset.format = pxavid_ycbcr422;

	camset.width = 320;
	camset.height = 240;

	camera_config(handle,&camset);

	camera_start(handle);

	handle_overlay2 = overlay2_open(NULL,pxavid_ycbcr422,NULL, 320, 240, 0 , 0);

	overlay2_getbuf(handle_overlay2,&vidbuf_overlay);
	len = vidbuf_overlay.width*vidbuf_overlay.height;
}
*/

int init_sensor(void)
{
#if HAVE_ADC
	if((adc_fd = open(DEVICE_ADC, O_RDWR)) < 0){
		perror("ADC device open faild! \n");
		exit(-1);
	}
#endif
#if HAVE_ACC
	if((acc_fd = open(DEVICE_ACC, O_RDWR)) < 0){
		perror("ACC device open faild! \n");
		exit(-1);
	}

	ioctl(acc_fd, MMA_VIN_ON , 0 );         // power on 
	ioctl(acc_fd, MMA_SLEEP_MODE_ON, 0 );   // Logic input pin to enable product
	ioctl(acc_fd, MMA_SENS_15G , 0 );       // Sensitivity
#endif
	
	printf("Sensor Init Complite\n");
}

int get_sensor_data(int type, unsigned short *send_data)
{
	if (type == LIGHT_SENSOR) {
		send_data[0] = front;
		send_data[1] = fog;
		send_data[2] = breakl;
		send_data[3] = emer << 2 + right << 1 + left;
		return 0;
	}
	else if (type == ACCELERATOR_SENSOR)
		return get_acc_data(send_data);

	else if (type == DISTANCE_SENSOR)
		return get_adc_data(send_data);

	else
		return -1;
}

int get_adc_data(unsigned short *send_data)
{
	int i;
	unsigned short adc_data[5];
#if HAVE_ADC
	read(adc_fd, adc_data, sizeof(adc_data)); // ADC READ

	for(i=1; i<5; i++)
		send_data[i] = adc_data[i-1];
#else
	for(i=1; i<5; i++)
		send_data[i] = (unsigned short)(1000*(rand()/RAND_MAX+1.0));
#endif
	send_data[0] = ADCID;

#if HAVE_ADC
	if(car_ctrl == 0)
		send_data[4] = 0x00; //Robot ch1~ch3 Used
#endif

	return 0;
}

int get_acc_data(unsigned short *send_data)
{
	int i;
	unsigned short acc_data[3];

#if HAVE_ACC
	read(acc_fd, acc_data, sizeof(acc_data)); // ACC READ
	for(i=1;i<4;i++)
		send_data[i] = acc_data[i-1];
#else
	for(i=1;i<4;i++)
		send_data[i] = (unsigned short)(1000*(rand()/RAND_MAX+1.0));
#endif
	send_data[4] = 0x00; //dummy
	send_data[0] = ACCID;

	return 0;
}

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
		sprintf(values[0], "%d", send_data[1]);
		sprintf(values[1], "%d", send_data[2]);
		sprintf(values[2], "%d", send_data[3]);
		sprintf(values[3], "%d", send_data[4]);

		get_acc_data(send_data);
		/*
		sprintf(value, "%d", send_data[1]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_ACCELERATOR_X, value);
		sprintf(value, "%d", send_data[2]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_ACCELERATOR_Y, value);
		sprintf(value, "%d", send_data[3]);
		upnp_set_variable(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_ACCELERATOR_Z, value);
		*/
		sprintf(values[4], "%d", send_data[1]);
		sprintf(values[5], "%d", send_data[2]);
		sprintf(values[6], "%d", send_data[3]);
		printf("values=%s\n", *values);
		upnp_set_variables(SMS_SERVICE_INDEX, SERVICE_SMS_INDEX_FRONT_DISTANCE, pvalues, 7);
		
	}

	return NULL;
}

void init_car(void)
{
	set_speed[0] = 0x01;
	set_speed[1] = 0xee;
	stop();
	usleep(10000);
	front_light(OFF);
	usleep(10000);
	break_light(OFF);
	usleep(10000);
	winker_light(OFF);
	usleep(10000);
	fog_light(OFF);
	usleep(10000);
	cm_angle(0x05, 0xDC);
	usleep(10000);
	dm_angle(0x05, 0xDC);
	usleep(10000);
}

void car_speed(char h, char l, int mode)
{
	speed = ((short)h<<8)+l;
	if(mode == 0)
		speed -= 100;
	else
		speed += 100;
	if(speed < 100)
		speed = 64;
	else if(speed > 3800)
		speed = 3800;

	set_speed[0] = speed>>8;
	set_speed[1] = speed - set_speed[0];
}

int set_sensor_data(int type, int index, int value)
{
	char cmd_buf;
	int result = 0;

	if (car_ctrl == 0) {
		cmd_buf = value - 0x9F;
		write(uart_fd, &cmd_buf, 1);
	}
	else {
		if (type != LIGHT_SENSOR)
			return -1;

		switch(index){
		case FRONT_LIGHT_SENSOR:
			if(front == 0){
				front_light(ON);
				front = 1;
			}
			else{
				front_light(OFF);
				front = 0;
			}
			result = front;
			break;
		case FOG_LIGHT_SENSOR:
			if(fog == 0){
				fog_light(ON);
				fog = 1;
			}
			else{
				fog_light(OFF);
				fog = 0;
			}
			result = fog;
			break;
		case BREAK_LIGHT_SENSOR:
			if(breakl == 0){
				break_light(ON);
				breakl = 1;
			}
			else{
				break_light(OFF);
				breakl = 0;
			}
			result = breakl;
			break;
		case WINKER_LIGHT_SENSOR:
		    if (value == 0) {
			if(emer == 0){
				winker_light(EMERGENCY);
				emer = 1;
				right = 0;
				left = 0;
			}
			else{
				winker_light(OFF);
				emer = 0;
			}
			result = emer;
		    }
		    else if (value == 1) {
			if(left == 0){
				winker_light(LEFT);
				emer = 0;
				left = 1;
				right = 0;
			}
			else{
				winker_light(OFF);
				left = 0;
				right = 0;
			}
			result = left;
		    }
		    else if (value == 2) {
			if(right == 0){
				winker_light(RIGHT);
				emer = 0;
				right = 1;
				left = 0;
			}
			else{
				winker_light(OFF);
				right = 0;
				left = 0;
			}
			result = right;
		    }
		    break;
		}
	}
	return result;
}

int set_motion(int cmd, int value)
{
	char cmd_buf;
	int result = 0;

	if (car_ctrl == 0) {
		cmd_buf = cmd - 0x9F;
		write(uart_fd, &cmd_buf, 1);
	}
	else {
		switch(cmd){
		case FORWARD_MOTION:
			if (go == 0) {
				stop();
				usleep(10000);
				lm_speed(0x01, 0xee);
				distance(0xff, 0xff);
				accel(0xff);
				forward_dis();
				go = 1;
				back = 0;
				set_speed[0] = 0x01;
				set_speed[1] = 0xee;
			}
			else {
				stop();
				go = 0;
				back = 0;
			}
			result = go;
			break;
		case RIGHTTURN_MOTION:
			if(dm_right == 0){
				dm_angle(0x03, 0x20);
				dm_right = 1;
				dm_left = 0;
			}
			else{
				dm_angle(0x05, 0xDC);
				dm_right = 0;
			}
			result = dm_right;
			break;
		case BACKWARD_MOTION:
			if(back == 0){
				stop();
				usleep(10000);
				lm_speed(0x01, 0xee);
				distance(0xff, 0xff);
				accel(0xff);
				backward_dis();
				back = 1;
				go = 0;
				set_speed[0] = 0x01;
				set_speed[1] = 0xee;
			}
			else{
				stop();
				back = 0;
				go = 0;
			}
			result = back;
			break;
		case LEFTTURN_MOTION:
			if(dm_left == 0){
				dm_angle(0x08, 0x98);
				dm_right = 0;
				dm_left = 1;
			}
			else{
				dm_angle(0x05, 0xDC);
				dm_left = 0;
			}
			result = dm_left;
			break;
		case STOP_MOTION:
			stop();
			back = 0;
			go = 0;
			dm_right = 0;
			dm_left = 0;
			result = 0;
			break;
		case MO:
			car_speed(set_speed[0], set_speed[1], 1);
			if(back == 1){
				lm_speed(set_speed[0], set_speed[1]);
				distance(0xff, 0xff);
				accel(0xff);
				backward_dis();
			}
			else if(go == 1){
				lm_speed(set_speed[0], set_speed[1]);
				distance(0xff, 0xff);
				accel(0xff);
				forward_dis();
			}
			break;
		case SE:
			car_speed(set_speed[0], set_speed[1], 0);
			if(back == 1){
				lm_speed(set_speed[0], set_speed[1]);
				backward_dis();
			}
			else if(go == 1){
				lm_speed(set_speed[0], set_speed[1]);
				forward_dis();
			}
			break;
		}
	}
	return result;
}

/*
void *cam_ThreadFunc( void *arg )
{
	int i = 0; 
	char cam_buf;
	unsigned char YUV_buff[ BUFF_SIZE ];

	unsigned char *yuv_data = NULL;
	unsigned char *Y_offset = NULL;
	unsigned char *U_offset = NULL;
	unsigned char *V_offset = NULL;

	unsigned int recv_ret = 0;
	unsigned int select_ret = 0;
	
	int ccam_len;
	int f_len;
	int remained;
	char send_len[3];

	system( "echo b > /proc/invert/tb" );

	while(1)
	{
		vidbuf = camera_get_frame(handle);

		memcpy(vidbuf_overlay.ycbcr.y, vidbuf->ycbcr.y, len);
		memcpy(vidbuf_overlay.ycbcr.cb, vidbuf->ycbcr.cb, len/2);
		memcpy(vidbuf_overlay.ycbcr.cr, vidbuf->ycbcr.cr, len/2);
		
		recvfrom(cam_sockfd, &cam_buf,  sizeof(cam_buf), 0, (struct sockaddr*)&ccam_address, &ccam_len);

		if(cam_buf == 0x0C)
		{
			printf("Camera: Coverting image\n");
			yuv_data = (unsigned char *) malloc(BUFF_SIZE);

			cam_buf = 0x00;
			memcpy(&yuv_data[0], vidbuf->ycbcr.y, len);
			memcpy(&yuv_data[len], vidbuf->ycbcr.cb, len/2);
			memcpy(&yuv_data[len+(len/2)], vidbuf->ycbcr.cr,len/2);

			Y_offset = yuv_data;
			U_offset = yuv_data+len;
			V_offset = yuv_data+len+(len/2);
			
			//YUV422 Data Save
			//static FILE* fp=NULL;
			//fp=fopen("test.yuv","wb");
			//fwrite( Y_offset , 1, len   ,fp);
			//fwrite( U_offset , 1, len/2 ,fp);
			//fwrite( V_offset , 1, len/2 ,fp);
			//fclose(fp);
			
			Y12con(Y_offset, U_offset, V_offset, cur, WIDTH, HEIGHT);
			yuv2jpg();
			//yuv2rgb(Y_offset, U_offset, V_offset, cur, WIDTH, HEIGHT);
			free(yuv_data);
			//free(cur);
			f_len = jpeg_len();

			printf("jpeg size : %d\n", f_len);
			send_len[0] = f_len/512; // q
			if(f_len%512>256){
				send_len[1] = f_len%512 - 256;
				send_len[2] = 1;
				remained = send_len[1] + 256;
			}
			else{
				send_len[1] = f_len%512; // r
				send_len[2] = 0;
				remained = send_len[1];
			}
			printf("Cam Len is : %d, %d, %d\n", send_len[0], send_len[1], send_len[2]);

			sendto(cam_sockfd, (char*)&send_len, sizeof(send_len), 0, (struct sockaddr*)&ccam_address, sizeof(ccam_address)); //Send Length
			//usleep(10000);

			while(i<send_len[0]){
				sendto(cam_sockfd, (char*)&jpegd[512*i], 512, 0, (struct sockaddr*)&ccam_address, sizeof(ccam_address)); //Send 512byte Jpeg Data
				recvfrom(cam_sockfd, &cam_buf,  sizeof(cam_buf), 0, (struct sockaddr*)&ccam_address, &ccam_len);
				if(cam_buf == 0x0E)
					i++;
				
				else if(cam_buf == 0x0D || cam_buf == 0x00){
					printf("JPEG Data Send fail...\n");
					break;
				}
				printf("cam %x : %d\n", cam_buf, i);
				//usleep(100000);
			}
			recvfrom(cam_sockfd, &cam_buf, sizeof(cam_buf), 0, (struct sockaddr*)&ccam_address, &ccam_len);
			if(cam_buf == 0x0E){
				sendto(cam_sockfd, (char*)&jpegd[512*send_len[0]], remained, 0, (struct sockaddr*)&ccam_address, sizeof(ccam_address)); //Send remaind Jpeg Data
				printf("JPEG Data Send Done !!\n");
			}
			free(y12);

			i=0;

			pthread_mutex_lock( &Gloval_Mutex );
			pthread_mutex_unlock( &Gloval_Mutex );
		}
		camera_release_frame(handle,vidbuf);
	}
	return NULL;
}
*/
int jpeg_len(void)
{
	int size;
	FILE *jpeg = fopen("still.jpg", "rb");
	fseek(jpeg, 0, SEEK_END);
	size = ftell(jpeg);
	
	fseek(jpeg, 0, SEEK_SET);
	char buf[size];
	jpegd = (unsigned char*)malloc(size);

	fread(buf, size, 1, jpeg);
	
	memcpy(jpegd, buf, size);

	return size;
}

int Y12con(unsigned char *y,unsigned char *u,unsigned char *v,unsigned char *rgba,int width,int height)
{
	FILE *fp;

	int i, j;
	int len_y12 = (width*height)+width*height/2;

	y12 = (unsigned char*)malloc((320*240)+320*240/2);
	memset(y12, NULL, len_y12);

	for(j=0; j<HEIGHT; j++){
		for(i=0; i<WIDTH; i++){
			y12[j*width+i] = y[j*width+i];
		}
	}
	for(j=0; j<height/2; j++){
		for(i=0; i<width/2; i++){
			y12[(height*width)+((j*width)/2+i)] = (u[j*(width)+i]+u[j*(width)+i+1])/2;
			y12[(height*width)+(height*width/4)+((j*width)/2+i)] = (v[j*(width)+i]+v[j*(width)+i+1])/2;
		}
	}

	fp = fopen("yuvd.yuv", "wb");
	fwrite(y12, len_y12, 1, fp);
	fclose(fp);

	return 0;
}


				
int yuv2rgb(unsigned char *y,unsigned char *u,unsigned char *v,unsigned char *rgba,int width,int height)
{
	int j, i;
	int yy, cb, cr;
	cur = (unsigned char*)malloc(RGB_SIZE);
	memset(cur, 255, RGB_SIZE);

	for( j = 0; j < height; j++ )
	{
		for( i = 0; i < width; i++ )
		{
			yy = y[j*width+i] - 16;
			cb = u[j*(width>>1)+(i>>1)] - 128;
			cr = v[j*(width>>1)+(i>>1)] - 128;

			cur[(i*3)+(j*width*3)] = (unsigned char) CLIP(( yy + 1.772 * cb              + 0.5 ));
			cur[((i*3)+1)+(j*width*3)] = (unsigned char) CLIP(( yy - 0.344 * cb - 0.714 * cr + 0.5 ));
			cur[((i*3)+2)+(j*width*3)] = (unsigned char) CLIP(( yy + 1.402 * cr + 0.5 ));

		}
	}
	return 0;
}

int get_camera_data(int id, int *type, char *encoding)
{
	if (id != camera_id)
		return -1;

	*type = camera_type;  /* 0:stream, 1:still */

	strcpy(encoding, camera_encoding); /* motion-jpeg: "mjpg", "mjpeg" */

	return 0;
}

int set_camera_data(int id, int type, char *encoding)
{
	if (id != 0)
		return -1;

	if (type == 0 || type == 1) /* 0:stream,  1:still */
		camera_type = type;

	strcpy(camera_encoding, encoding); /* motion-jpeg: "mjpg", "mjpeg" */

	return 0;
}

void init_device(int argc, char *argv[])
{
	int status;

	/*-------------------- user select ---------------------
	if(argc == 1){
		printf("MetalFighter : %s mf\n", argv[0]);
		printf("   AI CAR    : %s car\n", argv[0]);
		exit(1);
	}
	else if(argc == 2)
		if(!(strncmp("car", argv[1], 3)))
			car_ctrl = 1;

	print_info();
	------------------------------------------------------*/
	
	///* Auto Select MF/CAR 
	check_target();
	//--------------------*/

	uart_fd = open_serial();
	global.stop = 0;
	if(car_ctrl == 1)
		init_car();
	init_sensor();
	start_camera_thread();
	//init_camera();

	return 0;
}

void finish_device(int argc, char *argv[])
{
	sensor_thread_run = 0;

	global.stop = 1;
	usleep(1000 * 1000);

	global.in[0].stop(0);
	global.out[0].stop(global.out[0].param.id);
	pthread_cond_destroy(&global.in[0].db_update);
	pthread_mutex_destroy(&global.in[0].db);

	return;
}

int start_sensor_thread()
{
	if (sensor_thread_run) {
		printf("Sensor thread is already running!\n");
		return -1;
	}
	printf("Thread Create\n");
	sensor_thread_run = 1;
	if(pthread_create(&sensor_thread_id, NULL, sensor_thread, NULL) < 0) {
		printf("sensor thread create error\n");
		return -1;
	}

	return 0;
}

int split_parameters(char *parameter_string, int *argc, char **argv)
{
    int count = 1;
    argv[0] = NULL; // the plugin may set it to 'INPUT_PLUGIN_NAME'
    if(parameter_string != NULL && strlen(parameter_string) != 0) {
        char *arg = NULL, *saveptr = NULL, *token = NULL;

        arg = strdup(parameter_string);

        if(strchr(arg, ' ') != NULL) {
            token = strtok_r(arg, " ", &saveptr);
            if(token != NULL) {
                argv[count] = strdup(token);
                count++;
                while((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
                    argv[count] = strdup(token);
                    count++;
                    if(count >= MAX_PLUGIN_ARGUMENTS) {
                        IPRINT("ERROR: too many arguments to input plugin\n");
                        return 0;
                    }
                }
            }
        }
    }
    *argc = count;
    printf("split_parameter address argc=%p\n", argc);
    printf("split_parameter count=0x%x\n", *argc);
    return 1;
}

int start_camera_thread()
{
    char *input[MAX_INPUT_PLUGINS];
    char *output[MAX_OUTPUT_PLUGINS];

    input[0] = " --resolution 320x240 --fps 5 --device /dev/video0";
    global.incnt = 1;

    output[0] = " --port 8080";
    global.outcnt = 1;

    if(pthread_mutex_init(&global.in[0].db, NULL) != 0) {
        LOG("could not initialize mutex variable\n");
        closelog();
        exit(EXIT_FAILURE);
    }
    if(pthread_cond_init(&global.in[0].db_update, NULL) != 0) {
        LOG("could not initialize condition variable\n");
        closelog();
        exit(EXIT_FAILURE);
    }
    global.in[0].stop = &input_stop;
    global.in[0].param.parameters = strchr(input[0], ' ');
    split_parameters(global.in[0].param.parameters, &global.in[0].param.argc, global.in[0].param.argv);
    global.in[0].param.global = &global;
    global.in[0].param.id = 0;
    printf("input param argc=%d\n", global.in[0].param.argc);
    printf("input param address argc=%p\n", &global.in[0].param.argc);
    printf("input param =%p\n", &global.in[0].param);
    if(input_init(&global.in[0].param, 0)) {
        LOG("input_init() return value signals to exit\n");
        closelog();
        exit(0);
    }
    global.out[0].param.parameters = strchr(output[0], ' ');
    split_parameters(global.out[0].param.parameters, &global.out[0].param.argc, global.out[0].param.argv);
    global.out[0].param.global = &global;
    global.out[0].param.id = 0;
    if(output_init(&global.out[0].param, 0)) {
        LOG("output_init() return value signals to exit\n");
        closelog();
        exit(0);
    }
    DBG("starting input plugin\n");
    if(input_run(0)) {
        LOG("can not run input plugin\n");
        closelog();
        return 1;
    }
    DBG("starting output plugin\n");
    output_run(0);

    return 0;
}
