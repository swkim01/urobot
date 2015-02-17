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
#include "sts.h" 
#include "target.h" 

/* Represent the STS GetVideoURL action. */
#define SERVICE_STS_ACTION_GET_VIDEO_URL "GetVideoURL"

/* Represent the STS CameraIndex argument. */
#define SERVICE_STS_ARG_CAMERA_INDEX "CameraIndex"

/* Represent the STS CameraType argument. */
#define SERVICE_STS_ARG_CAMERA_TYPE "CameraType"

/* Represent the STS CameraType argument. */
#define SERVICE_STS_ARG_VIDEO_URL "VideoURL"

/* Represent the STS ObjectID argument. */
#define SERVICE_STS_ARG_OBJECT_ID "ObjectID"

/* Represent the STS Root Object ID argument. */
#define SERVICE_STS_ROOT_OBJECT_ID "0"

int
sts_browse_metadata (struct device_t *dev, struct service_t *service, struct buffer_t *buf, int index, int type)
{
  char url[30];
  //extern struct urobot_t *ut;
  action_arg_t in[10];
  static action_arg_t out[10];

  in[0].i = index;
  in[1].i = type;
  dev->cmd(service, GET_INFO, in, out);
  sprintf(url, "?action=%s/", type == 0? "stream":"snapshot"/*, index*/);
  buffer_appendf (buf, "http://%s:%d/%s",
                      UpnpGetServerIpAddress (), STS_PORT/*ut->port*/, url);
  return out[0].i++;
}

static bool
sts_get_video_url (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int result_count = 0, index, type, id;
  struct buffer_t *out = NULL;

  if (!event)
    return false;

  /* check if metadatas have been well inited */
  //if (!ut->init)
  //  return false;

  /* Retrieve Browse arguments */
  index = upnp_get_ui4 (event->request, SERVICE_STS_ARG_CAMERA_INDEX);
  type = upnp_get_ui4 (event->request, SERVICE_STS_ARG_CAMERA_TYPE);
  id = upnp_get_ui4 (event->request, SERVICE_STS_ARG_OBJECT_ID);

  out = buffer_new ();
  if (!out)
  {
    return false;
  }

  result_count = sts_browse_metadata (dev, service, out, index, type);
  printf("VideoURL=%s, result_count=%d\n", out->buf, result_count);

  if (result_count < 0)
  {
    buffer_free (out);
    return false;
  }

  upnp_add_response (event, SERVICE_STS_ARG_VIDEO_URL, out->buf);
  buffer_free (out);

  return event->status;
}

/* List of UPnP ContentDirectory Service actions */
struct service_action_t sts_service_actions[] = {
  { SERVICE_STS_ACTION_GET_VIDEO_URL, sts_get_video_url },
  { NULL, NULL }
};

struct template_dict sts_script_dict[] = {
   {"STS_STREAMING_ADDRESS", NULL},
   NULL
  };

struct web_template sts_script = {
  "STS_SCRIPT",
  STS_SCRIPT,
  sts_script_dict
/*
  {
   {"STS_STREAMING_ADDRESS", NULL}
  }
*/};

struct web_template sts_onload = {
  "STS_ONLOAD",
  STS_ONLOAD,
  NULL};

struct template_dict sts_body_dict[] = {
   {"STS_STREAMING_ADDRESS", NULL},
   NULL
  };

struct web_template sts_body = {
  "STS_BODY",
  STS_BODY,
  sts_body_dict
/*
  {
   {"STS_STREAMING_ADDRESS", NULL}
  }
*/};

struct web_content sts_web = { NULL, &sts_script, &sts_onload, &sts_body };

int sts_init(struct service_t *service, int id)
{
  extern struct urobot_t *ut;
  struct buffer_t *url = buffer_new ();

  service->SCPDURL = STS_LOCATION;
  service->SCPDDESC = STS_DESCRIPTION;
  service->controlURL = STS_CONTROL_LOCATION;
  service->eventSubURL = STS_EVENTSUB_LOCATION;
  service->web = &sts_web;
  buffer_appendf (url, "http://%s:%d/?action=snapshot",
                      ut->ip/*UpnpGetServerIpAddress ()*/, STS_PORT);
  sts_web.script->tdict[0].value = url->buf;
  sts_web.body->tdict[0].value = url->buf;

  return 0;
}
