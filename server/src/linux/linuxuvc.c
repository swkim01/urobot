#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "urobot.h"
#include "target.h"
#include "device.h"

#include "cam.h"
#include "mcs.h"
#include "sms.h"
#include "sts.h"

#include "devices/car.h"
#include "devices/uvc.h"
#include "devices/streamer.h"

#define ADCID		0x00AD
#define ACCID		0x00AC
#define CAMID		0x0C

/* globals */
static struct urobot_t *ut;

static char values[7][20];
static char *pvalues[7] = {values[0], values[1], values[2], values[3], values[4], values[5], values[6]};
void
update_sensors(struct service_t *service)
{
	unsigned short send_data[7];

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
	upnp_set_variables(service, 0, pvalues, 7);
}

int
main (int argc, char **argv)
{
  int i;
  char sms_variablevalues[SMS_SERVICE_VARIABLE_COUNT][20];
  struct urobot_t *ut;
  struct device_t *car, *cam, *streamer;
  struct service_t *mcs, *sms, *cams, *sts;
  char *args[3] = {
     "",
     "--resolution 640x480 -y --fps 30 --device /dev/video0",
     "--port 8080"
    };

  ut = urobot_init(argc, argv);

  car = urobot_device_add(ut, CAR_DEVICE_TYPE, "Virtual Car",
                          car_init,
                          car_stop,
                          car_run,
                          car_cmd);
  mcs = urobot_service_add(car, MCS_SERVICE_ID, MCS_SERVICE_TYPE,
                           mcs_service_actions, 0, 0, 0, mcs_init);
  for (i=0; i<7; i++) {
    sms_variables[i] = sms_variablevalues[i];
    strcpy(sms_variablevalues[i], sms_defaultvalues[i]);
  }
  sms = urobot_service_add(car, SMS_SERVICE_ID, SMS_SERVICE_TYPE,
                           sms_service_actions,
                           sms_variablenames,
                           sms_variables, 7, sms_init);
  register_service_callback(sms, update_sensors);

  cam = urobot_device_add(ut, UVC_DEVICE_TYPE, "Camera",
                          uvc_init,
                          uvc_stop,
                          uvc_run,
                          uvc_cmd);
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
