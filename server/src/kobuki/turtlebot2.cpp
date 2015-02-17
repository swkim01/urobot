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

#include "cam.h"
#include "mcs.h"
#include "sms.h"
#include "sts.h"

#include "devices/kobuki.h"
#include "devices/kinect.h"
#include "devices/streamer.h"

/* globals */
static struct urobot_t *ut;

/*
static int sensor_thread_run = 0;
static char values[7][20];
static char *pvalues[7] = {values[0], values[1], values[2], values[3], values[4], values[5], values[6]};
void
update_sensors(struct service_t *service)
{
	unsigned char send_data[26];

	get_sensor_data(DISTANCE_SENSOR, (unsigned short *)send_data);
	sprintf(values[0], "%d", send_data[BUMPER]);
	sprintf(values[1], "%d", send_data[WHEELDROP]);
	sprintf(values[2], "%d", send_data[CLIFF]);

	sprintf(values[3], "%d", (short)((send_data[LEFTENCODER_HI] << 8) | (send_data[LEFTENCODER_LO] & 0xff)));
	sprintf(values[4], "%d", (short)((send_data[RIGHTENCODER_HI] << 8) | (send_data[RIGHTENCODER_LO] & 0xff)));
	sprintf(values[5], "%d", send_data[CHARGER] & 0xff);
	sprintf(values[6], "%d", send_data[BATTERY] & 0xff);
	//printf("values=%s\n", *values);
	upnp_set_variables(service, 0, pvalues, 7);
}
*/

extern "C" int main (int argc, char **argv)
{
  int i;
  char sms_variablevalues[SMS_SERVICE_VARIABLE_COUNT][20];
  struct urobot_t *ut;
  struct device_t *kobuki, *cam, *streamer;
  struct service_t *mcs, *sms, *cams, *sts;
  char *args[3] = {
     "",
     "--resolution 640x480 --fps 30 --device /dev/video0",
     "--port 8080"
    };

  ut = urobot_init(argc, argv);

  kobuki = urobot_device_add(ut, KOBUKI_DEVICE_TYPE, "Kobuki Actuator",
                          kobuki_init,
                          kobuki_stop,
                          kobuki_run,
                          kobuki_cmd);
  mcs = urobot_service_add(kobuki, MCS_SERVICE_ID, MCS_SERVICE_TYPE,
                           mcs_service_actions, 0, 0, 0, mcs_init);
  for (i=0; i<7; i++) {
    sms_variables[i] = sms_variablevalues[i];
    strcpy(sms_variablevalues[i], sms_defaultvalues[i]);
  }
  sms = urobot_service_add(kobuki, SMS_SERVICE_ID, SMS_SERVICE_TYPE,
                           sms_service_actions,
                           sms_variablenames,
                           sms_variables, 7, sms_init);
  //register_service_callback(sms, update_sensors);

  cam = urobot_device_add(ut, KINECT_DEVICE_TYPE, "Camera",
                          kinect_init,
                          kinect_stop,
                          kinect_run,
                          kinect_cmd);
  cams = urobot_service_add(cam, CAM_SERVICE_ID, CAM_SERVICE_TYPE,
                           cam_service_actions, 0, 0, 0, NULL);

  streamer = urobot_device_add(ut, STREAMER_DEVICE_TYPE, "Streamer",
                          streamer_init,
                          streamer_stop,
                          streamer_run,
                          streamer_cmd);
  sts = urobot_service_add(streamer, STS_SERVICE_ID, STS_SERVICE_TYPE,
                           sts_service_actions, 0, 0, 0, sts_init);

  init_devices(ut, 3, args
    //{"",
    // "--resolution 640x480 --fps 30 --device /dev/video0",
    // "--port 8080"
    //}
  );

  if (urobot_run(ut) != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }

  finish_devices(ut, (int)NULL, (char **)NULL);

  urobot_stop(ut);

  /* it should never be executed */
  return EXIT_SUCCESS;
}
