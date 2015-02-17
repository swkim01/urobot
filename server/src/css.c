/*
 * css.c : uRobot Car Sensor Monitering Service.
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
#include <stdio.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "css.h"
#include "target.h"

/* Represent the CSS SetFrontLightSensor action. */
#define SERVICE_CSS_ACTION_SET_FRONT_LIGHT "SetFrontLight"

/* Represent the CSS SetFogLight action. */
#define SERVICE_CSS_ACTION_SET_FOG_LIGHT "SetFogLight"

/* Represent the CSS SetBreakLightSensor action. */
#define SERVICE_CSS_ACTION_SET_BREAK_LIGHT "SetBreakLight"

/* Represent the CSS SetWinkerLight action. */
#define SERVICE_CSS_ACTION_SET_WINKER_LIGHT "SetWinkerLight"

/* Represent the CSS GetFrontDistance action. */
#define SERVICE_CSS_ACTION_GET_FRONT_DISTANCE "GetFrontDistance"

/* Represent the CSS GetBackDistance action. */
#define SERVICE_CSS_ACTION_GET_BACK_DISTANCE "GetBackDistance"

/* Represent the CSS GetLeftDistance action. */
#define SERVICE_CSS_ACTION_GET_LEFT_DISTANCE "GetLeftDistance"

/* Represent the CSS GetRightDistance action. */
#define SERVICE_CSS_ACTION_GET_RIGHT_DISTANCE "GetRightDistance"

/* Represent the CSS GetCurrentSensorInfo action. */
#define SERVICE_CSS_ACTION_GET_SENSOR_INFO "GetSensorInfo"

/* Represent the CSS Command argument. */
#define SERVICE_CSS_ARG_COMMAND "Command"

/* Represent the CSS Status argument. */
#define SERVICE_CSS_ARG_STATUS "Status"

/* Represent the CSS Status value. */
#define SERVICE_CSS_STATUS_OK "OK"

/* Represent the CSS SensorID argument. */
#define SERVICE_CSS_ARG_SENSOR_ID "SensorID"

/* Represent the CSS default Sensor ID value. */
#define SERVICE_CSS_DEFAULT_SENSOR_ID "0"

/* Represent the CSS unknown connection ID value. */
#define SERVICE_CSS_UNKNOW_ID "-1"

/* Represent the CSS Sensor value. */
#define SERVICE_CSS_ARG_SENSOR_VALUE "SensorValue"

/* Represent the CSS Default Sensor value. */
#define SERVICE_CSS_DEFAULT_VALUE "0"

static bool
css_set_front_light (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve SetFrontLight arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to set front light of the car
  set_sensor_data(LIGHT_SENSOR, FRONT_LIGHT_SENSOR, command);

  sprintf(value, "%d", command);
  if (upnp_set_variable (service, SERVICE_CSS_INDEX_FRONT_LIGHT, value)) {
    upnp_add_response (event, SERVICE_CSS_ARG_STATUS, SERVICE_CSS_STATUS_OK);
  }

  return event->status;
}

static bool
css_set_fog_light (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve SetFogLight arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to set fog light of the car
  set_sensor_data(LIGHT_SENSOR, FOG_LIGHT_SENSOR, command);

  sprintf(value, "%d", command);
  if (upnp_set_variable (service, SERVICE_CSS_INDEX_FOG_LIGHT, value)) {
    upnp_add_response (event, SERVICE_CSS_ARG_STATUS, SERVICE_CSS_STATUS_OK);
  }

  return event->status;
}

static bool
css_set_break_light (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve SetBreakLight arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to set break light of the car
  set_sensor_data(LIGHT_SENSOR, BREAK_LIGHT_SENSOR, command);

  sprintf(value, "%d", command);
  if (upnp_set_variable (service, SERVICE_CSS_INDEX_BREAK_LIGHT, value)) {
    upnp_add_response (event, SERVICE_CSS_ARG_STATUS, SERVICE_CSS_STATUS_OK);
  }

  return event->status;
}

static bool
css_set_winker_light (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve SetWinkerLight arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to set winker light of the car
  set_sensor_data(LIGHT_SENSOR, WINKER_LIGHT_SENSOR, command);

  sprintf(value, "%d", command);
  if (upnp_set_variable (service, SERVICE_CSS_INDEX_WINKER_LIGHT, value)) {
    upnp_add_response (event, SERVICE_CSS_ARG_STATUS, SERVICE_CSS_STATUS_OK);
  }

  return event->status;
}

static bool
css_get_front_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to get front distance car
  upnp_get_variable(service, SERVICE_CSS_INDEX_FRONT_DISTANCE, value);

  upnp_add_response (event, SERVICE_CSS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
css_get_back_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to get back distance car
  upnp_get_variable(service, SERVICE_CSS_INDEX_BACK_DISTANCE, value);

  upnp_add_response (event, SERVICE_CSS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
css_get_left_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to get left distance car
  upnp_get_variable(service, SERVICE_CSS_INDEX_LEFT_DISTANCE, value);

  upnp_add_response (event, SERVICE_CSS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
css_get_right_distance (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  char value[20];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_CSS_ARG_COMMAND);

  // make action to get right distnace car
  upnp_get_variable(service, SERVICE_CSS_INDEX_RIGHT_DISTANCE, value);

  upnp_add_response (event, SERVICE_CSS_ARG_SENSOR_VALUE, value);

  return event->status;
}

static bool
css_get_current_sensor_info (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  if (!event)
    return false;

  upnp_add_response (event, SERVICE_CSS_ARG_SENSOR_ID,
                     SERVICE_CSS_DEFAULT_SENSOR_ID);

  upnp_add_response (event, SERVICE_CSS_ARG_STATUS, SERVICE_CSS_STATUS_OK);

  return event->status;
}

/* List of UPnP ConnectionManager Service actions */
struct service_action_t css_service_actions[] = {
  { SERVICE_CSS_ACTION_SET_FRONT_LIGHT, css_set_front_light },
  { SERVICE_CSS_ACTION_SET_FOG_LIGHT, css_set_fog_light },
  { SERVICE_CSS_ACTION_SET_BREAK_LIGHT, css_set_break_light },
  { SERVICE_CSS_ACTION_SET_WINKER_LIGHT, css_set_winker_light },
  { SERVICE_CSS_ACTION_GET_FRONT_DISTANCE, css_get_front_distance },
  { SERVICE_CSS_ACTION_GET_BACK_DISTANCE, css_get_back_distance },
  { SERVICE_CSS_ACTION_GET_LEFT_DISTANCE, css_get_left_distance },
  { SERVICE_CSS_ACTION_GET_RIGHT_DISTANCE, css_get_right_distance },
  { SERVICE_CSS_ACTION_GET_SENSOR_INFO, css_get_current_sensor_info },
  { NULL, NULL }
};

const char *css_variablenames[] = {
  "FrontLight",
  "FogLight",
  "BreakLight",
  "WinkerLight",
  "FrontDistance",
  "BackDistance",
  "LeftDistance",
  "RightDistance",
};

char *css_variables[CSS_SERVICE_VARIABLE_COUNT];

char *css_defaultvalues[] = {
  "0",
  "0",
  "0",
  "0",
  "100.0",
  "100.0",
  "100.0",
  "100.0",
};

struct template_dict css_script_dict[] = {
   {"UROBOT_CGI", UROBOT_CGI},
   {"SERVICE_NAME", CSS_SERVICE_NAME},
   {"CGI_CSS_ACTION_SET_FRONT_LIGHT", CGI_CSS_ACTION_SET_FRONT_LIGHT},
   {"CGI_CSS_ACTION_SET_FOG_LIGHT", CGI_CSS_ACTION_SET_FOG_LIGHT},
   {"CGI_CSS_ACTION_SET_BREAK_LIGHT", CGI_CSS_ACTION_SET_BREAK_LIGHT},
   {"CGI_CSS_ACTION_SET_WINKER_LIGHT", CGI_CSS_ACTION_SET_WINKER_LIGHT},
   NULL
  };

struct web_template css_script = {
  "CSS_SCRIPT",
  CSS_SCRIPT,
  css_script_dict};

struct template_dict css_body_dict[] = {
   {"CGI_CSS_ACTION_SET_FRONT_LIGHT", CGI_CSS_ACTION_SET_FRONT_LIGHT},
   {"CGI_CSS_ACTION_SET_FOG_LIGHT", CGI_CSS_ACTION_SET_FOG_LIGHT},
   {"CGI_CSS_ACTION_SET_BREAK_LIGHT", CGI_CSS_ACTION_SET_BREAK_LIGHT},
   {"CGI_CSS_ACTION_SET_WINKER_LIGHT", CGI_CSS_ACTION_SET_WINKER_LIGHT},
   NULL
  };

struct web_template css_body = {
  "CSS_BODY",
  CSS_BODY,
  css_body_dict};

struct web_content css_web = { NULL, &css_script, NULL, &css_body };

int css_init(struct service_t *service, int id)
{
  service->SCPDURL = CSS_LOCATION;
  service->SCPDDESC = CSS_DESCRIPTION;
  service->controlURL = CSS_CONTROL_LOCATION;
  service->eventSubURL = CSS_EVENTSUB_LOCATION;
  service->web = &css_web;

  return 0;
}
