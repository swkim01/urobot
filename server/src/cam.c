#include <stdlib.h>
#include <stdio.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "buffer.h"
#include "utils.h"
#include "trace.h" 
#include "cam.h" 
#include "target.h" 

/* Represent the CAM GetSystemUpdateID action. */
#define SERVICE_CAM_ACTION_UPDATE_ID "GetSystemUpdateID"

/* Represent the CAM SetEncoding action. */
#define SERVICE_CAM_ACTION_SET_ENCODING "SetEncoding"

/* Represent the CAM GetEncoding action. */
#define SERVICE_CAM_ACTION_GET_ENCODING "SetEncoding"

/* Represent the CAM SetResolution action. */
#define SERVICE_CAM_ACTION_SET_RESOLUTION "SetResolution"

/* Represent the CAM GetResolution action. */
#define SERVICE_CAM_ACTION_GET_RESOLUTION "GetResolution"

/* Represent the CAM SetFrameRate action. */
#define SERVICE_CAM_ACTION_SET_FRAME_RATE "SetFrameRate"

/* Represent the CAM GetFrameRate action. */
#define SERVICE_CAM_ACTION_GET_FRAME_RATE "GetFrameRate"

/* Represent the CAM UpdateId argument. */
#define SERVICE_CAM_ARG_UPDATE_ID "UpdateID"

/* Represent the CAM Encoding argument. */
#define SERVICE_CAM_ARG_ENCODING "Encoding"

/* Represent the CAM Resolution argument. */
#define SERVICE_CAM_ARG_RESOLUTION "Resolution"

/* Represent the CAM CameraIndex argument. */
#define SERVICE_CAM_ARG_CAMERA_INDEX "CameraIndex"

/* Represent the CAM FrameRate argument. */
#define SERVICE_CAM_ARG_FRAME_RATE "FrameRate"

/* Represent the CAM FrameRate argument. */
#define SERVICE_CAM_ARG_FRAME_RATE "FrameRate"

/* Represent the CAM Root object id. */
#define SERVICE_CAM_ROOT_OBJECT_ID "0"

/* Represent the CAM Status argument. */
#define SERVICE_CAM_ARG_STATUS "Status"

/* Represent the CAM Status value. */
#define SERVICE_CAM_STATUS_OK "OK"

static bool
cam_get_system_update_id (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  upnp_add_response (event, SERVICE_CAM_ARG_UPDATE_ID,
                     SERVICE_CAM_ROOT_OBJECT_ID);

  return event->status;
}

static bool
cam_get_encoding (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index, action, result_count;
  char encoding[20];
  action_arg_t in[10], out[10];

  /* Retrieve camera index */
  index = upnp_get_ui4 (event->request, SERVICE_CAM_ARG_CAMERA_INDEX);

  //result_count = get_camera_data (index, &action, encoding);

  //dev->cmd(service, GET_ENCODING, 2, index, encoding);
  in[0].i = index; out[0].p = encoding;
  dev->cmd(service, GET_ENCODING, in, out);

  upnp_add_response (event, SERVICE_CAM_ARG_ENCODING, encoding);

  return event->status;
}

static bool
cam_set_encoding (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int result_count;
  int index, action = 0;
  char tmp_encoding[20];
  char *encoding = NULL;
  action_arg_t in[10];

  /* Retrieve camera index */
  index = upnp_get_ui4 (event->request, SERVICE_CAM_ARG_CAMERA_INDEX);

  /* Retrieve Browse arguments */
  encoding = upnp_get_string (event->request, SERVICE_CAM_ARG_ENCODING);

  // set encoding
  //result_count = get_camera_data (0, &action, tmp_encoding);
  //result_count = set_camera_data (0, action, encoding);

  //dev->cmd(service, SET_ENCODING, 1, encoding);
  in[0].i = index; in[1].p = encoding;
  dev->cmd(service, SET_ENCODING, in, NULL);

  upnp_add_response (event, SERVICE_CAM_ARG_STATUS, SERVICE_CAM_STATUS_OK);

  return event->status;
}

static bool
cam_get_resolution (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index;
  char *resolution;

  /* Retrieve camera index */
  index = upnp_get_ui4 (event->request, SERVICE_CAM_ARG_CAMERA_INDEX);

  upnp_add_response (event, SERVICE_CAM_ARG_RESOLUTION, resolution);

  return event->status;
}

static bool
cam_set_resolution (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index;
  // set resolution

  /* Retrieve camera index */
  index = upnp_get_ui4 (event->request, SERVICE_CAM_ARG_CAMERA_INDEX);

  upnp_add_response (event, SERVICE_CAM_ARG_STATUS, SERVICE_CAM_STATUS_OK);

  return event->status;
}

static bool
cam_get_frame_rate (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index;
  char *frame_rate;

  /* Retrieve camera index */
  index = upnp_get_ui4 (event->request, SERVICE_CAM_ARG_CAMERA_INDEX);

  upnp_add_response (event, SERVICE_CAM_ARG_FRAME_RATE, frame_rate);

  return event->status;
}
 
static bool
cam_set_frame_rate (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index;
  // set frame rate

  /* Retrieve camera index */
  index = upnp_get_ui4 (event->request, SERVICE_CAM_ARG_CAMERA_INDEX);

  upnp_add_response (event, SERVICE_CAM_ARG_STATUS, SERVICE_CAM_STATUS_OK);

  return event->status;
}

/* List of UPnP ContentDirectory Service actions */
struct service_action_t cam_service_actions[] = {
  { SERVICE_CAM_ACTION_UPDATE_ID, cam_get_system_update_id, "V", "I" },
  { SERVICE_CAM_ACTION_GET_ENCODING, cam_get_encoding, "I", "S" },
  { SERVICE_CAM_ACTION_SET_ENCODING, cam_set_encoding, "IS", "V" },
  { SERVICE_CAM_ACTION_GET_RESOLUTION, cam_get_resolution, "I", "S" },
  { SERVICE_CAM_ACTION_SET_RESOLUTION, cam_set_resolution, "IS", "B" },
  { SERVICE_CAM_ACTION_GET_FRAME_RATE, cam_get_frame_rate, "I", "I" },
  { SERVICE_CAM_ACTION_SET_FRAME_RATE, cam_set_frame_rate, "II", "B" },
  { NULL, NULL, NULL, NULL }
};

int cam_init(struct service_t *service, int id)
{
  service->SCPDURL = CAM_LOCATION;
  service->SCPDDESC = CAM_DESCRIPTION;
  service->controlURL = CAM_CONTROL_LOCATION;
  service->eventSubURL = CAM_EVENTSUB_LOCATION;

  return 0;
}
