/*
 * services.h : uRobot UPnP services handler header.
 * Originally developped for the uRobot project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
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

#ifndef _SERVICES_H_
#define _SERVICES_H_

#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <pthread.h>
#include "urobot.h"
#include "device.h"
#include "presentation.h"

#define MAX_SERVICES	10

struct device_t;
struct action_event_t;
struct service_t;

union _action_arg_t {
  int    b;
  char   c;
  int    i;
  float  f;
  double d;
  char  *p;
};
typedef union _action_arg_t action_arg_t;

struct service_action_t {
  char *name;
  bool (*function) (struct device_t *, struct service_t *, struct action_event_t *);
  char *inputtype;
  char *outputtype;
};

struct service_t {
  char *id;
  char *type;
  struct service_action_t *actions;
  pthread_t thread_id;
  int running;
  int (*callback)(struct service_t *service);
  char **variablenames;
  char **variablevalues;
  int variable_count;
  struct device_t *dev;
  char *SCPDURL ;
  char *SCPDDESC ;
  char *controlURL;
  char *eventSubURL;
  struct web_content *web;

  int (*init)(struct service_t *service, int id);
};

#define SERVICE_CONTENT_TYPE "text/xml"

#ifdef __cplusplus
extern "C" {
#endif

void init_variables(void);

bool find_service_action (struct device_t *dev,
                          struct Upnp_Action_Request *request,
                          struct service_t **pservice,
                          struct service_action_t **action);

bool find_service (struct device_t *dev,
                   struct Upnp_Subscription_Request *request,
                   struct service_t **pservice);

bool upnp_add_response (struct action_event_t *event,
                        char *key, const char *value);

char * upnp_get_string (struct Upnp_Action_Request *request, const char *key);

int upnp_get_ui4 (struct Upnp_Action_Request *request, const char *key);
float upnp_get_f4 (struct Upnp_Action_Request *request, const char *key);

int upnp_set_variable (struct service_t *service, int variable, char *value);
int upnp_set_variables (struct service_t *service, int variable, char **value, int count);
int upnp_get_variable (struct service_t *service, int variable, char *value);
int
register_service_callback (struct service_t *service, void *callback);
struct service_t *
urobot_service_add (struct device_t *dev, char *id, char *type, struct service_action_t *actions, char **var_names, char **var_values, int var_count, int (*init)(struct service_t *, int));
int
urobot_service_delete (struct device_t *dev, struct service_t *service);
int
urobot_service_register_presentation (struct service_t *service, struct web_content *web);

#ifdef __cplusplus
};
#endif

#endif /* _SERVICES_H_ */
