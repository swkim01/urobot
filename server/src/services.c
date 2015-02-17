/*
 * services.c : uRobot UPnP services handler.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "cms.h"
#include "mcs.h"
#include "sms.h"
#include "cam.h"
#include "trace.h"

/* Represent the ObjectID argument. */
#define ARG_OBJECT_ID "ObjectID"

/* Represent the ContainerID argument. */
#define ARG_CONTAINER_ID "ContainerID"

static char udn[80];

void init_variables()
{
	extern struct urobot_t *ut;
	sprintf(udn, "uuid:%s", ut->udn);
}

bool
find_service_action (struct device_t *dev,
                     struct Upnp_Action_Request *request,
                     struct service_t **pservice,
                     struct service_action_t **action)
{
  int i, j;

  *pservice = NULL;
  *action = NULL;

  if (!request || !request->ActionName)
    return false;

  if (!dev)
    return NULL;

  for (i=0; i < dev->servicecount; i++) {
    if (!strcmp (dev->service[i]->id, request->ServiceID))
    {
      *pservice = dev->service[i];
      for (j = 0; dev->service[i]->actions[j].name; j++)
      {
        if (!strcmp (dev->service[i]->actions[j].name, request->ActionName))
        {
          *action = &dev->service[i]->actions[j];
          return true;
        }
      }
      return false;
    }
  }

  return false;
}

bool
find_service (struct device_t *dev,
              struct Upnp_Subscription_Request *request,
              struct service_t **pservice)
{
  int i;

  *pservice = NULL;

  if (!request)
    return false;

  if (!dev)
    return NULL;

  for (i=0; i < dev->servicecount; i++) {
    if (!strcmp (dev->service[i]->id, request->ServiceId))
    {
      *pservice = dev->service[i];
      return true;
    }
  }
  return false;
}

bool
upnp_add_response (struct action_event_t *event, char *key, const char *value)
{
  char *val;
  int res;

  if (!event || !event->status || !key || !value)
    return false;

  val = strdup (value);
  if (!val)
    return false;

  res = UpnpAddToActionResponse (&event->request->ActionResult,
                                 event->request->ActionName,
                                 event->service->type, key, val);

  if (res != UPNP_E_SUCCESS)
    {
      free (val);
      return false;
    }

  free (val);
  return true;
}

char *
upnp_get_string (struct Upnp_Action_Request *request, const char *key)
{
  IXML_Node *node = NULL;

  if (!request || !request->ActionRequest || !key)
    return NULL;

  node = (IXML_Node *) request->ActionRequest;
  if (!node)
  {
    log_verbose ("Invalid action request document\n");
    return NULL;
  }

  node = ixmlNode_getFirstChild (node);
  if (!node)
  {
    log_verbose ("Invalid action request document\n");
    return NULL;
  }

  node = ixmlNode_getFirstChild (node);
  for (; node; node = ixmlNode_getNextSibling (node))
    if (!strcmp (ixmlNode_getNodeName (node), key))
    {
      node = ixmlNode_getFirstChild (node);
      if (!node)
        return strdup ("");
      return strdup (ixmlNode_getNodeValue (node));
    }

  log_verbose ("Missing action request argument (%s)\n", key);

  return NULL;
}

int
upnp_get_ui4 (struct Upnp_Action_Request *request, const char *key)
{
  char *value;
  int val;

  if (!request || !key)
    return 0;

  value = upnp_get_string (request, key);
  if (!value && !strcmp (key, ARG_OBJECT_ID))
    value = upnp_get_string (request, ARG_CONTAINER_ID);

  if (!value)
    return 0;

  val = atoi (value);
  free (value);

  return val;
}

float
upnp_get_f4 (struct Upnp_Action_Request *request, const char *key)
{
  char *value;
  float val;

  if (!request || !key)
    return 0;

  value = upnp_get_string (request, key);
  if (!value && !strcmp (key, ARG_OBJECT_ID))
    value = upnp_get_string (request, ARG_CONTAINER_ID);

  if (!value)
    return 0;

  val = atof (value);
  free (value);

  return val;
}

int
upnp_set_variable (struct service_t *service, int index, char *value)
{
  return upnp_set_variables (service, index, &value, 1);
}

int
upnp_set_variables (struct service_t *service, int index, char **values, int count)
{
  int i;

  if (!service || index+count > service->variable_count)
    return 0;

  for (i=0; i<count; i++) {
    //printf("variable[%d]=%s\n", i, values[i]);
    strcpy (service->variablevalues[index+i], values[i]);
  }

  return 1;
}
  
int
upnp_get_variable (struct service_t *service, int variable, char *value)
{
  if (!service || variable >= service->variable_count)
    return 0;

  strcpy (value, service->variablevalues[variable]);

  return 1;
}

int
register_service_callback (struct service_t *service, void *callback)
{
  if (!service || !callback)
    return 0;

  service->callback = callback;

  return 1;
}

int
unregister_service_callback (struct service_t *service)
{
  if (!service)
    return 0;

  service->callback = NULL;

  return 1;
}

struct service_t *
urobot_service_add (struct device_t *dev, char *id, char *type, struct service_action_t *actions, char **var_names, char **var_values, int var_count, int (*init)())
{
  if (!dev)
    return NULL;

  struct service_t *service = (struct service_t *) malloc (sizeof (struct service_t));
  if (!service)
    return NULL;

  dev->service[dev->servicecount] = service;
  service->id = strdup (id);
  service->type = strdup (type);
  service->actions = actions;
  service->dev = dev;
  service->variablenames = var_names;
  service->variablevalues = var_values;
  service->variable_count = var_count;
  service->init = init;
  service->web = NULL;

  dev->servicecount++;

  return service;
}

int
urobot_service_delete (struct device_t *dev, struct service_t *service)
{
  int i, j;

  if (!dev)
    return 0;

  for (i=0; i < dev->servicecount; i++) {
    if (service == dev->service[i]) {
      free(service);
      for (j=i; j < dev->servicecount-1; j++) {
        dev->service[j] = dev->service[j+1];
      }
      dev->servicecount--;
      return 1;
    }
  }
  return 0;
}

/*
int
urobot_service_register_presentation (struct service_t *service, struct web_content *web)
{
  if (!service)
    return 0;

  service->web = web;
  return 1;
}
*/

void * service_thread(void *arg)
{
  extern struct urobot_t *ut;
  struct service_t *service = (struct service_t *)arg;
  int sleeptime = 2;

  if (!ut || !service)
    return;

  while (service->running) {
    sleep((unsigned int)sleeptime);

    /* get variable values */
    if (service->callback) {
      (*service->callback)(service);
    }

    if (ut->verbose)
    {
      log_verbose ("***************************************************\n");
      log_verbose ("**                  SMS Notify                   **\n");
      log_verbose ("***************************************************\n");
      log_verbose ("ServiceID: %s\n", service->id);
      log_verbose ("Value: %s=%s\n", service->variablenames[0], service->variablevalues[0]);
    }
    UpnpNotify(ut->handle, udn,
                      service->id,
                      (const char **)service->variablenames,
                      (const char **)service->variablevalues,
                      service->variable_count);
  }
}

int start_service_thread(struct service_t *service)
{
  service->running = 1;
  if(pthread_create(&service->thread_id, NULL, service_thread, (void *)service) < 0) {
    printf("service thread create error\n");
    return -1;
  }
  return 0;
}
