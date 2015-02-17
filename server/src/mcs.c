/*
 * mcs.c : uRobot Motion Controller Service.
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
#include "mcs.h"

/* Represent the MCS GoForward action. */
#define SERVICE_MCS_ACTION_GO_FORWARD "GoForward"

/* Represent the MCS GoBackward action. */
#define SERVICE_MCS_ACTION_GO_BACKWARD "GoBackward"

/* Represent the MCS Stop action. */
#define SERVICE_MCS_ACTION_STOP "Stop"

/* Represent the MCS TurnLeft action. */
#define SERVICE_MCS_ACTION_TURN_LEFT "TurnLeft"

/* Represent the MCS TurnRight action. */
#define SERVICE_MCS_ACTION_TURN_RIGHT "TurnRight"

/* Represent the MCS Move action. */
#define SERVICE_MCS_ACTION_MOVE "Move"

/* Represent the MCS GetCurrentMotionInfo action. */
#define SERVICE_MCS_ACTION_GET_MOTION_INFO "GetMotionInfo"

/* Represent the MCS Command argument. */
#define SERVICE_MCS_ARG_COMMAND "Command"

/* Represent the MCS Velocity argument. */
#define SERVICE_MCS_ARG_VELOCITY "Velocity"

/* Represent the MCS Angular argument. */
#define SERVICE_MCS_ARG_ANGULAR "Angular"

/* Represent the MCS Status argument. */
#define SERVICE_MCS_ARG_STATUS "Status"

/* Represent the MCS Status value. */
#define SERVICE_MCS_STATUS_OK "OK"

/* Represent the MCS MotionID argument. */
#define SERVICE_MCS_ARG_MOTION_ID "MotionID"

/* Represent the MCS Direction argument. */
#define SERVICE_MCS_ARG_DIRECTION "Direction"

/* Represent the MCS default connection ID value. */
#define SERVICE_MCS_DEFAULT_MOTION_ID "0"

/* Represent the MCS unknown connection ID value. */
#define SERVICE_MCS_UNKNOWN_ID "-1"

/* Represent the MCS Output value. */
#define SERVICE_MCS_OUTPUT "Output"

static bool
mcs_go_forward (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[10];

  if (!event)
    return false;

  /* Retrieve GoForward arguments */
  command = upnp_get_ui4 (event->request, SERVICE_MCS_ARG_COMMAND);

  // make action to go forward the car
  //set_motion(FORWARD_MOTION, command);
  in[0].i = command;
  dev->cmd(service, FORWARD_MOTION, in, NULL);

  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

static bool
mcs_go_backward (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[10];

  if (!event)
    return false;

  /* Retrieve GoBackward arguments */
  command = upnp_get_ui4 (event->request, SERVICE_MCS_ARG_COMMAND);

  // make action to go backward the car
  //set_motion(BACKWARD_MOTION, command);
  in[0].i = command;
  dev->cmd(service, BACKWARD_MOTION, in, NULL);

  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

static bool
mcs_stop (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[10];

  if (!event)
    return false;

  /* Retrieve Stop arguments */
  command = upnp_get_ui4 (event->request, SERVICE_MCS_ARG_COMMAND);

  // make action to stop the car
  //set_motion(STOP_MOTION, command);
  in[0].i = command;
  dev->cmd(service, STOP_MOTION, in, NULL);

  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

static bool
mcs_turn_left (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[10];

  if (!event)
    return false;

  /* Retrieve TurnLeft arguments */
  command = upnp_get_ui4 (event->request, SERVICE_MCS_ARG_COMMAND);

  // make action to turn left the car
  //set_motion(LEFTTURN_MOTION, command);
  in[0].i = command;
  dev->cmd(service, LEFTTURN_MOTION, in, NULL);

  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

static bool
mcs_turn_right (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  int command;
  action_arg_t in[10];

  if (!event)
    return false;

  /* Retrieve TurnRight arguments */
  command = upnp_get_ui4 (event->request, SERVICE_MCS_ARG_COMMAND);

  // make action to turn right the car
  //set_motion(RIGHTTURN_MOTION, command);
  in[0].i = command;
  dev->cmd(service, RIGHTTURN_MOTION, in, NULL);

  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

static bool
mcs_move (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  float velocity;
  float angular;
  action_arg_t in[10];

  if (!event)
    return false;

  /* Retrieve Move arguments */
  velocity = upnp_get_f4 (event->request, SERVICE_MCS_ARG_VELOCITY);
  angular = upnp_get_f4 (event->request, SERVICE_MCS_ARG_ANGULAR);

  //printf("mcs_move: velocity=%f, angular=%f\n", velocity, angular);
  // make action to move the robot with velocity and angular
  //set_basecontrol(velocity, angular);
  in[0].f = velocity; in[1].f = angular;
  dev->cmd(service, MOVE_MOTION, in, NULL);

  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

static bool
mcs_get_current_motion_info (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  char motion[2]="0";
  if (!event)
    return false;

  //dev->cmd(service, INFO_MOTION, 1, &motion);
  upnp_add_response (event, SERVICE_MCS_ARG_MOTION_ID, motion);

  upnp_add_response (event, SERVICE_MCS_ARG_DIRECTION, SERVICE_MCS_OUTPUT);
  upnp_add_response (event, SERVICE_MCS_ARG_STATUS, SERVICE_MCS_STATUS_OK);

  return event->status;
}

/* List of UPnP ConnectionManager Service actions */
struct service_action_t mcs_service_actions[] = {
  { SERVICE_MCS_ACTION_GO_FORWARD, mcs_go_forward, "I", "B" },
  { SERVICE_MCS_ACTION_GO_BACKWARD, mcs_go_backward, "I", "B" },
  { SERVICE_MCS_ACTION_STOP, mcs_stop, "I", "B" },
  { SERVICE_MCS_ACTION_TURN_LEFT, mcs_turn_left, "I", "B" },
  { SERVICE_MCS_ACTION_TURN_RIGHT, mcs_turn_right, "I", "B" },
  { SERVICE_MCS_ACTION_MOVE, mcs_move, "FF", "B" },
  { SERVICE_MCS_ACTION_GET_MOTION_INFO, mcs_get_current_motion_info, "V", "ISB" },
  { NULL, NULL, NULL, NULL }
};

struct web_template mcs_css = {
  "MCS_CSS",
  MCS_CSS,
  NULL};

struct template_dict mcs_script_dict[] = {
   {"UROBOT_CGI", UROBOT_CGI},
   {"SERVICE_NAME", MCS_SERVICE_NAME},
   {"CGI_MCS_ACTION_GO_FORWARD", CGI_MCS_ACTION_GO_FORWARD},
   {"CGI_MCS_ACTION_GO_BACKWARD", CGI_MCS_ACTION_GO_BACKWARD},
   {"CGI_MCS_ACTION_STOP", CGI_MCS_ACTION_STOP},
   {"CGI_MCS_ACTION_TURN_LEFT", CGI_MCS_ACTION_TURN_LEFT},
   {"CGI_MCS_ACTION_TURN_RIGHT", CGI_MCS_ACTION_TURN_RIGHT},
   NULL
  };

struct web_template mcs_script = {
  "MCS_SCRIPT",
  MCS_SCRIPT,
  mcs_script_dict
/*
  {{"UROBOT_CGI", UROBOT_CGI},
   {"CGI_MCS_ACTION_GO_FORWARD", CGI_MCS_ACTION_GO_FORWARD},
   {"CGI_MCS_ACTION_GO_BACKWARD", CGI_MCS_ACTION_GO_BACKWARD},
   {"CGI_MCS_ACTION_STOP", CGI_MCS_ACTION_STOP},
   {"CGI_MCS_ACTION_TURN_LEFT", CGI_MCS_ACTION_TURN_LEFT},
   {"CGI_MCS_ACTION_TURN_RIGHT", CGI_MCS_ACTION_TURN_RIGHT}
  }
*/};
//  </script>");
//  <script src=\"joystick.js\"></script>");

struct web_template mcs_onload = {
  "MCS_ONLOAD",
  MCS_ONLOAD,
  NULL};

struct template_dict mcs_body_dict[] = {
   {"CGI_MCS_ACTION_GO_FORWARD", CGI_MCS_ACTION_GO_FORWARD},
   {"CGI_MCS_ACTION_GO_BACKWARD", CGI_MCS_ACTION_GO_BACKWARD},
   {"CGI_MCS_ACTION_STOP", CGI_MCS_ACTION_STOP},
   {"CGI_MCS_ACTION_TURN_LEFT", CGI_MCS_ACTION_TURN_LEFT},
   {"CGI_MCS_ACTION_TURN_RIGHT", CGI_MCS_ACTION_TURN_RIGHT},
   NULL
  };
struct web_template mcs_body = {
  "MCS_BODY",
  MCS_BODY,
  mcs_body_dict
/*
  {{"CGI_MCS_ACTION_GO_FORWARD", CGI_MCS_ACTION_GO_FORWARD},
   {"CGI_MCS_ACTION_GO_BACKWARD", CGI_MCS_ACTION_GO_BACKWARD},
   {"CGI_MCS_ACTION_STOP", CGI_MCS_ACTION_STOP},
   {"CGI_MCS_ACTION_TURN_LEFT", CGI_MCS_ACTION_TURN_LEFT},
   {"CGI_MCS_ACTION_TURN_RIGHT", CGI_MCS_ACTION_TURN_RIGHT}
  }
*/};

struct web_content mcs_web = { &mcs_css, &mcs_script, &mcs_onload, &mcs_body };

int mcs_init(struct service_t *service, int id)
{
  service->SCPDURL = MCS_LOCATION;
  service->SCPDDESC = MCS_DESCRIPTION;
  service->controlURL = MCS_CONTROL_LOCATION;
  service->eventSubURL = MCS_EVENTSUB_LOCATION;
  service->web = &mcs_web;

  return 0;
}
