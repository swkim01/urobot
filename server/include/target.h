/* sensor type */
#define LIGHT_SENSOR		0
#define DISTANCE_SENSOR		1
#define ACCELERATOR_SENSOR	2

/* light sensor index */
#define FRONT_LIGHT_SENSOR	0
#define FOG_LIGHT_SENSOR	1
#define BREAK_LIGHT_SENSOR	2
#define WINKER_LIGHT_SENSOR	3

#ifdef __cplusplus
extern "C" {
#endif
void init_device(struct urobot_t *ut, int argc, char *argv[]);
void finish_device(struct urobot_t *ut, int argc, char *argv[]);
int  set_motion(int index, int value);
void set_basecontrol(float velocity, float angular);
int  set_sensor_data(int type, int index, int value);
int  get_sensor_data(int type, unsigned short *send_data);
int  get_camera_data(int id, int *type, char *encoding);
int  set_camera_data(int id, int type, char *encoding);
int  start_sensor_thread(void);
int  start_camera_thread(void);
#ifdef __cplusplus
};
#endif
