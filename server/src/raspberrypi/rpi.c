#include <stdlib.h>
#include <stdio.h>
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
#include "sts.h"

#include "devices/raspberrypi.h"
#include "devices/uvc.h"
#include "devices/streamer.h"

#define ADCID		0x00AD
#define ACCID		0x00AC
#define CAMID		0x0C

/* globals */
static struct urobot_t *ut;

int
main (int argc, char **argv)
{
  int i;
  struct urobot_t *ut;
  struct device_t *rpi, *cam, *streamer;
  struct service_t *mcs, *cams, *sts;
  char *args[3] = {
     "",
     "--resolution 640x480 -y --fps 30 --device /dev/video0",
     "--port 8080"
    };

  ut = urobot_init(argc, argv);

  rpi = urobot_device_add(ut, CAR_DEVICE_TYPE, "Raspberrypi Car",
                          raspberrypi_init,
                          raspberrypi_stop,
                          raspberrypi_run,
                          raspberrypi_cmd);
  mcs = urobot_service_add(rpi, MCS_SERVICE_ID, MCS_SERVICE_TYPE,
                           mcs_service_actions, 0, 0, 0, mcs_init);

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
