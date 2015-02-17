/*
 * urobot.h : uRobot UPnP Robot Server header.
 * Originally developped for the DEU robot project.
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

#ifndef _UROBOT_H_
#define _UROBOT_H_

#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <stdbool.h>
#include <pthread.h>

#include "buffer.h"

//#define UPNP_VERSION 10613

#define VIRTUAL_DIR "/web"
//#define DEFAULT_UUID "898f9738-d930-4db4-a3cg"
#define DEFAULT_UUID "898f9738-d930-4db4"

#define UPNP_MAX_CONTENT_LENGTH 4096

#define STARTING_ENTRY_ID_DEFAULT 0

#define UPNP_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>urn:www-urobot-org:device:RobotServer:1</deviceType>" \
"    <friendlyName>%s: 1</friendlyName>" \
"    <manufacturer>uRobot Team</manufacturer>" \
"    <manufacturerURL>https://github.com/swkim01/urobot/</manufacturerURL>" \
"    <modelDescription>UPnP Robot Server</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>001</modelNumber>" \
"    <modelURL>https://github.com/swkim01/urobot/</modelURL>" \
"    <serialNumber>UROBOT-01</serialNumber>" \
"    <UDN>uuid:%s</UDN>" \
"    <serviceList>" \
"      <service>" \
"        <serviceType>urn:schemes-upnp-org:service:ConnectionManager:1</serviceType>" \
"        <serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>" \
"        <SCPDURL>/web/cms.xml</SCPDURL>" \
"        <controlURL>/web/cms_control</controlURL>" \
"        <eventSubURL>/web/cms_event</eventSubURL>" \
"      </service>" \
"      <service>" \
"        <serviceType>urn:schemas-urobot-org:service:MotionController:1</serviceType>" \
"        <serviceId>urn:urobot-org:serviceId:MotionController</serviceId>" \
"        <SCPDURL>/web/mcs.xml</SCPDURL>" \
"        <controlURL>/web/mcs_control</controlURL>" \
"        <eventSubURL>/web/mcs_event</eventSubURL>" \
"      </service>" \
"      <service>" \
"        <serviceType>urn:schemas-urobot-org:service:SensorMonitoring:1</serviceType>" \
"        <serviceId>urn:urobot-org:serviceId:SensorMonitoring</serviceId>" \
"        <SCPDURL>/web/sms.xml</SCPDURL>" \
"        <controlURL>/web/sms_control</controlURL>" \
"        <eventSubURL>/web/sms_event</eventSubURL>" \
"      </service>" \
"      <service>" \
"        <serviceType>urn:schemas-urobot-org:service:CameraMonitoring:1</serviceType>" \
"        <serviceId>urn:urobot-org:serviceId:CameraMonitoring</serviceId>" \
"        <SCPDURL>/web/cam.xml</SCPDURL>" \
"        <controlURL>/web/cam_control</controlURL>" \
"        <eventSubURL>/web/cam_event</eventSubURL>" \
"      </service>" \
"      <service>" \
"        <serviceType>urn:schemas-urobot-org:service:Streaming:1</serviceType>" \
"        <serviceId>urn:urobot-org:serviceId:Streaming</serviceId>" \
"        <SCPDURL>/web/sts.xml</SCPDURL>" \
"        <controlURL>/web/sts_control</controlURL>" \
"        <eventSubURL>/web/sts_event</eventSubURL>" \
"      </service>" \
"    </serviceList>" \
"    <presentationURL>/web/urobot.html</presentationURL>" \
"  </device>" \
"</root>"

#define MAX_DEVICE_PLUGINS 10

typedef struct urobot_t urobot;

struct device_t;
struct device_common_t;
struct service_t;

struct urobot_t {
  char *name;
  char *interface;
  char *model_name;
  int starting_id;
  int init;
  UpnpDevice_Handle handle;
  char *udn;
  char *ip;
  unsigned short port;
  struct buffer_t *presentation;
  bool use_presentation;
  bool verbose;
  bool daemon;
  char *cfg_file;
  pthread_mutex_t web_mutex;
  pthread_mutex_t termination_mutex;
  pthread_cond_t termination_cond;
  char *pid_file;

  int stop;

  /* device */
  struct device_t *dev[MAX_DEVICE_PLUGINS];
  int devcnt;

  /* signal fresh common data */
  struct device_common_t *comm[MAX_DEVICE_PLUGINS];
};

struct action_event_t {
  struct Upnp_Action_Request *request;
  bool status;
  struct service_t *service;
};

inline void display_headers (void);

#ifdef __cplusplus
extern "C" {
#endif
struct urobot_t * urobot_init (int argc, char **argv);
int urobot_run (struct urobot_t *ut);
void urobot_stop(struct urobot_t *ut);

#ifdef __cplusplus
};
#endif

#endif /* _UROBOT_H_ */
