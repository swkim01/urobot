/*
 * sms.c : uRobot Sensor Monitering Service.
 * Originally developped for the uRobot project.
 * Copyright (C) 2011-2015 Seong-Woo Kim <swkim01@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "sms.h"
#include "target.h"

/* Represent the SMS GetFrontDistance action. */
#define SERVICE_SMS_ACTION_GET_FRONT_DISTANCE "GetFrontDistance"

/* Represent the SMS GetBackDistance action. */
#define SERVICE_SMS_ACTION_GET_BACK_DISTANCE "GetBackDistance"

/* Represent the SMS GetLeftDistance action. */
#define SERVICE_SMS_ACTION_GET_LEFT_DISTANCE "GetLeftDistance"

/* Represent the SMS GetRightDistance action. */
#define SERVICE_SMS_ACTION_GET_RIGHT_DISTANCE "GetRightDistance"

/* Represent the SMS GetDistances action. */
#define SERVICE_SMS_ACTION_GET_DISTANCES "GetDistances"

/* Represent the SMS GetAccelerator action. */
#define SERVICE_SMS_ACTION_GET_ACCELERATOR "GetAccelerator"

/* Represent the SMS GetCurrentSensorInfo action. */
#define SERVICE_SMS_ACTION_GET_SENSOR_INFO "GetSensorInfo"

/* Represent the SMS Command argument. */
#define SERVICE_SMS_ARG_COMMAND "Command"

/* Represent the SMS Status argument. */
#define SERVICE_SMS_ARG_STATUS "Status"

/* Represent the SMS Status value. */
#define SERVICE_SMS_STATUS_OK "OK"

/* Represent the SMS SensorID argument. */
#define SERVICE_SMS_ARG_SENSOR_ID "SensorID"

/* Represent the SMS default Sensor ID value. */
#define SERVICE_SMS_DEFAULT_SENSOR_ID "0"

/* Represent the SMS unknown connection ID value. */
#define SERVICE_SMS_UNKNOW_ID "-1"

/* Represent the SMS Sensor value. */
#define SERVICE_SMS_ARG_SENSOR_VALUE "SensorValue"

/* Represent the SMS Default Sensor value. */
#define SERVICE_SMS_DEFAULT_VALUE "0"

static bool
sms_get_front_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[1];
  action_arg_t out[1];
  char value[5];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_SMS_ARG_COMMAND);

  // make action to get front distance car
  //upnp_get_variable(service, SERVICE_SMS_INDEX_FRONT_DISTANCE, value);
  in[0].i = command;
  dev->cmd(service, FRONT_DISTANCE, in, out);
  sprintf(value, "%4d", out[0].b);

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
sms_get_back_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[1];
  action_arg_t out[1];
  char value[5];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_SMS_ARG_COMMAND);

  // make action to get back distance car
  //upnp_get_variable(service, SERVICE_SMS_INDEX_BACK_DISTANCE, value);
  in[0].i = command;
  dev->cmd(service, BACK_DISTANCE, in, out);
  sprintf(value, "%4d", out[0].b);

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
sms_get_left_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[1];
  action_arg_t out[1];
  char value[5];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_SMS_ARG_COMMAND);

  // make action to get left distance car
  //upnp_get_variable(service, SERVICE_SMS_INDEX_LEFT_DISTANCE, value);
  in[0].i = command;
  dev->cmd(service, LEFT_DISTANCE, in, out);
  sprintf(value, "%4d", out[0].b);

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
sms_get_right_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[1];
  action_arg_t out[1];
  char value[5];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_SMS_ARG_COMMAND);

  // make action to get right distnace car
  //upnp_get_variable(service, SERVICE_SMS_INDEX_RIGHT_DISTANCE, value);
  in[0].i = command;
  dev->cmd(service, RIGHT_DISTANCE, in, out);
  sprintf(value, "%4d", out[0].b);

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
sms_get_distances (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index;
  action_arg_t in[1];
  action_arg_t out[4];
  char value[20];

  if (!event)
    return false;

  /* Retrieve index arguments */
  index = upnp_get_ui4 (event->request, SERVICE_SMS_ARG_COMMAND);

  // make action to get accelerator sensor
  //upnp_get_variable(service, SERVICE_SMS_INDEX_ACCELERATOR_X-index, value);
  in[0].i = index;
  dev->cmd(service, GET_DISTANCES, in, out);
  sprintf(value, "%4d:%4d:%4d:%4d", out[0].b, out[1].b, out[2].b, out[3].b);

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
sms_get_accelerator (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int index;
  action_arg_t in[1];
  action_arg_t out[3];
  char value[15];

  if (!event)
    return false;

  /* Retrieve index arguments */
  index = upnp_get_ui4 (event->request, SERVICE_SMS_ARG_COMMAND);

  // make action to get accelerator sensor
  //upnp_get_variable(service, SERVICE_SMS_INDEX_ACCELERATOR_X-index, value);
  in[0].i = index;
  dev->cmd(service, GET_ACCELERATOR, in, out);
  sprintf(value, "%4d:%4d:%4d", out[0].b, out[1].b, out[2].b);

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
sms_get_current_sensor_info (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  if (!event)
    return false;

  upnp_add_response (event, SERVICE_SMS_ARG_SENSOR_ID,
                     SERVICE_SMS_DEFAULT_SENSOR_ID);

  upnp_add_response (event, SERVICE_SMS_ARG_STATUS, SERVICE_SMS_STATUS_OK);

  return event->status;
}

/* List of UPnP ConnectionManager Service actions */
struct service_action_t sms_service_actions[] = {
  { SERVICE_SMS_ACTION_GET_FRONT_DISTANCE, sms_get_front_distance, "I", "B" },
  { SERVICE_SMS_ACTION_GET_BACK_DISTANCE, sms_get_back_distance, "I", "B" },
  { SERVICE_SMS_ACTION_GET_LEFT_DISTANCE, sms_get_left_distance, "I", "B" },
  { SERVICE_SMS_ACTION_GET_RIGHT_DISTANCE, sms_get_right_distance, "I", "B" },
  { SERVICE_SMS_ACTION_GET_DISTANCES, sms_get_distances, "I", "BBBB" },
  { SERVICE_SMS_ACTION_GET_ACCELERATOR, sms_get_accelerator, "I", "BBB" },
  { SERVICE_SMS_ACTION_GET_SENSOR_INFO, sms_get_current_sensor_info, "V", "B" },
  { NULL, NULL, NULL, NULL }
};

char *sms_variablenames[] = {
  "FrontDistance",
  "BackDistance",
  "LeftDistance",
  "RightDistance",
  "AcceleratorX",
  "AcceleratorY",
  "AcceleratorZ",
};

char *sms_variables[SMS_SERVICE_VARIABLE_COUNT];

char *sms_defaultvalues[] = {
  "100.0",
  "100.0",
  "100.0",
  "100.0",
  "0",
  "0",
  "0",
};

struct template_dict sms_script_dict[] = {
   {"UROBOT_CGI", UROBOT_CGI},
   {"SERVICE_NAME", SMS_SERVICE_NAME},
   {"CGI_SMS_ACTION_GET_DISTANCES", CGI_SMS_ACTION_GET_DISTANCES},
   {"CGI_SMS_LABEL_FRONT_DISTANCE", CGI_SMS_LABEL_FRONT_DISTANCE},
   {"CGI_SMS_LABEL_BACK_DISTANCE", CGI_SMS_LABEL_BACK_DISTANCE},
   {"CGI_SMS_LABEL_LEFT_DISTANCE", CGI_SMS_LABEL_LEFT_DISTANCE},
   {"CGI_SMS_LABEL_RIGHT_DISTANCE", CGI_SMS_LABEL_RIGHT_DISTANCE},
   NULL
  };

struct web_template sms_script = {
  "SMS_SCRIPT",
  SMS_SCRIPT,
  sms_script_dict
/*
  {{"UROBOT_CGI", UROBOT_CGI},
   {"CGI_SMS_ACTION_GET_DISTANCES", CGI_SMS_ACTION_GET_DISTANCES},
   {"CGI_SMS_LABEL_FRONT_DISTANCE", CGI_SMS_LABEL_FRONT_DISTANCE},
   {"CGI_SMS_LABEL_BACK_DISTANCE", CGI_SMS_LABEL_BACK_DISTANCE},
   {"CGI_SMS_LABEL_LEFT_DISTANCE", CGI_SMS_LABEL_LEFT_DISTANCE},
   {"CGI_SMS_LABEL_RIGHT_DISTANCE", CGI_SMS_LABEL_RIGHT_DISTANCE}
  }
*/};

struct template_dict sms_body_dict[] = {
   {"CGI_SMS_LABEL_FRONT_DISTANCE", CGI_SMS_LABEL_FRONT_DISTANCE},
   {"CGI_SMS_LABEL_BACK_DISTANCE", CGI_SMS_LABEL_BACK_DISTANCE},
   {"CGI_SMS_LABEL_LEFT_DISTANCE", CGI_SMS_LABEL_LEFT_DISTANCE},
   {"CGI_SMS_LABEL_RIGHT_DISTANCE", CGI_SMS_LABEL_RIGHT_DISTANCE},
   NULL
  };

struct web_template sms_body = {
  "SMS_BODY",
  SMS_BODY,
  sms_body_dict
/*
  {{"CGI_SMS_LABEL_FRONT_DISTANCE", CGI_SMS_LABEL_FRONT_DISTANCE},
   {"CGI_SMS_LABEL_BACK_DISTANCE", CGI_SMS_LABEL_BACK_DISTANCE},
   {"CGI_SMS_LABEL_LEFT_DISTANCE", CGI_SMS_LABEL_LEFT_DISTANCE},
   {"CGI_SMS_LABEL_RIGHT_DISTANCE", CGI_SMS_LABEL_RIGHT_DISTANCE}
  }
*/};

struct web_content sms_web = { NULL, &sms_script, NULL, &sms_body };

int sms_init(struct service_t *service, int id)
{ 
  service->SCPDURL = SMS_LOCATION;
  service->SCPDDESC = SMS_DESCRIPTION;
  service->controlURL = SMS_CONTROL_LOCATION;
  service->eventSubURL = SMS_EVENTSUB_LOCATION;
  service->web = &sms_web;

  return 0;
}
