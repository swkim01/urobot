/*
 * presentation.c : uRobot UPnP Presentation Page.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_LANGINFO_CODESET
# include <langinfo.h>
#endif

#include "config.h"
#include "urobot.h"
#include "device.h"
#include "services.h"
#include "buffer.h"
#include "presentation.h"
#include "gettext.h"
#include "cam.h"
#include "target.h"

#define CGI_SERVICE "service="
#define CGI_ACTION "action="
#define CGI_ACTION_REFRESH "refresh"
#define CGI_MCS_ACTION_GO_FORWARD "GoForward"
#define CGI_MCS_ACTION_GO_BACKWARD "GoBackward"
#define CGI_MCS_ACTION_STOP "Stop"
#define CGI_MCS_ACTION_TURN_LEFT "TurnLeft"
#define CGI_MCS_ACTION_TURN_RIGHT "TurnRight"
#define CGI_MCS_ACTION_MOVE "Move"
#define CGI_CSS_ACTION_SET_FRONT_LIGHT "FrontLight"
#define CGI_CSS_ACTION_SET_FOG_LIGHT "FogLight"
#define CGI_CSS_ACTION_SET_BREAK_LIGHT "BreakLight"
#define CGI_CSS_ACTION_SET_WINKER_LIGHT "WinkerLight"
#define CGI_SMS_ACTION_GET_DISTANCES "GetDistances"
#define CGI_SMS_LABEL_FRONT_DISTANCE	 "Front"
#define CGI_SMS_LABEL_BACK_DISTANCE	 "Back"
#define CGI_SMS_LABEL_LEFT_DISTANCE	 "Left"
#define CGI_SMS_LABEL_RIGHT_DISTANCE	 "Right"
#define CGI_PATH "path"
#define CGI_SHARE "share"

#define UTF8 "UTF-8"

static int
find_service_actionid (struct device_t *dev,
                       char *servicename,
                       char *actionname,
                       struct service_t **pservice)
{
  int i, j;

  *pservice = NULL;

  if (!servicename || !actionname)
    return -1;

  if (!dev)
    return -1;

  for (i=0; i < dev->servicecount; i++) {
    if (strstr (dev->service[i]->id, servicename))
    {
      for (j = 0; dev->service[i]->actions[j].name; j++)
      {
        if (!strcmp (dev->service[i]->actions[j].name, actionname))
        {
          *pservice = dev->service[i];
          return j;
        }
      }
      return -1;
    }
  }

  return -1;
}

int
process_cgi (struct urobot_t *ut, char *cgiargs)
{
  char *servicename = NULL;
  char *actionname = NULL;
  char *actionargs = NULL;
  struct service_t *service;
  int refresh = 0;
  int result = 0;
  int devid, actionid, outid;

  if (!ut || !cgiargs)
    return -1;

  //if (strncmp (cgiargs, CGI_ACTION, strlen (CGI_ACTION))
  //    || strncmp (cgiargs, CGI_SERVICE, strlen (CGI_SERVICE)))
  if (!strstr (cgiargs, CGI_ACTION) || !strstr (cgiargs, CGI_SERVICE))
    return -1;

  //servicename = cgiargs + strlen (CGI_SERVICE);
  servicename = strtok_r(cgiargs + strlen (CGI_SERVICE), "&", &actionname);
  actionname = strtok_r(actionname + strlen (CGI_ACTION), "&", &actionargs);

  //printf ("process_cgi: service=%s, actionname=%s, action=%s\n", servicename, actionname, actionargs?actionargs:"");
 
  for (devid=0; devid < ut->devcnt; devid++) {
    if ((actionid=find_service_actionid (ut->dev[devid], servicename, actionname, &service)) >= 0)
      {
        char *str1, *str2, *saveptr1, *saveptr2, *token, *subtoken;
        char outstr[20];
        int incnt=0, outcnt=0;
        char *inarg = service->actions[actionid].inputtype;
        char *outarg = service->actions[actionid].outputtype;
        action_arg_t in[10], out[10];

        for (str1 = actionargs; ; str1 = NULL) {
          token = strtok_r(str1, "&", &saveptr1);
          if (token == NULL)
            break;
          if (!strncmp (token, "args=", strlen("args=")))
            for (str2 = token+strlen("args="); ; str2 = NULL, incnt++) {
              subtoken = strtok_r(str2, ",", &saveptr2);
              if (subtoken == NULL)
                break;
              switch(inarg[incnt]) {
              case 'B':
              case 'I':
                in[incnt].i = atoi(subtoken);
                break;
              case 'C':
                in[incnt].c = (char)atoi(subtoken);
                break;
              case 'F':
                in[incnt].f = (float)atof(subtoken);
                break;
              case 'D':
                in[incnt].d = (double)atof(subtoken);
                break;
              case 'S':
                in[incnt].p = strdup(subtoken);
              }
            }
        }

        if (ut->dev[devid]->cmd != NULL)
          ut->dev[devid]->cmd(service, actionid, in, out);

        if (ut->presentation)
          buffer_free (ut->presentation);
        ut->presentation = buffer_new ();
        buffer_append (ut->presentation, "<response>");
        if (outarg != NULL)
            outcnt = strlen(outarg);
        for (outid = 0; outid < outcnt; outid++) {
          switch(outarg[outid]) {
          case 'B':
          case 'I':
            sprintf(outstr, "%4d", out[outid].i);
            break;
          case 'C':
            sprintf(outstr, "%3d", out[outid].c);
            break;
          case 'F':
            sprintf(outstr, "%f", out[outid].f);
            break;
          case 'D':
            sprintf(outstr, "%f", out[outid].d);
            break;
          case 'S':
            strcpy(outstr, out[outid].p);
          default:
            sprintf(outstr, "0");
          }
          buffer_appendf (ut->presentation, "<result>%s</result>", outstr);
        }
        buffer_append (ut->presentation, "</response>");
        return 1;
      }
  }
  //else if (!strncmp (actionname, CGI_ACTION_REFRESH, strlen (CGI_ACTION_REFRESH)))
  //  refresh = 1;
  printf("fff\n");

  if (ut->presentation)
    buffer_free (ut->presentation);
  ut->presentation = buffer_new ();

  buffer_append (ut->presentation, "<html>\n");
  buffer_append (ut->presentation, "<head>\n");
  buffer_appendf (ut->presentation, "<title>%s</title>\n",
                  _("uRobot Information Page"));
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"pragma\" content=\"no-cache\"/>\n");
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"expires\" content=\"1970-01-01\"/>\n");
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"refresh\" content=\"0; URL=/web/urobot.html\"/>\n");
  buffer_append (ut->presentation, "</head>\n");
  buffer_append (ut->presentation, "</html>\n");

  return 0;
}

void
build_css (struct urobot_t *ut)
{
  buffer_append (ut->presentation, "<style>\n");
  buffer_append (ut->presentation, "    body {\n");
  buffer_append (ut->presentation, "        overflow	: hidden;\n");
  buffer_append (ut->presentation, "        padding	: 0;\n");
  buffer_append (ut->presentation, "        margin	: 0;\n");
  buffer_append (ut->presentation, "    }\n");
  build_css_template(ut);
  buffer_append (ut->presentation, "</style>\n");
}

void
build_javascript (struct urobot_t *ut)
{
  struct buffer_t *cam = NULL;

  buffer_append (ut->presentation, "<script>\n");
  buffer_append (ut->presentation, "function createXMLHttpRequest() {\n");
  buffer_append (ut->presentation, "  var xmlHttp;\n");
  buffer_append (ut->presentation, "  if (window.ActiveXObject) {\n");
  buffer_append (ut->presentation, "    xmlHttp = new ActiveXObject(\"Microsoft.XMLHTTP\");\n");
  buffer_append (ut->presentation, "  }\n");
  buffer_append (ut->presentation, "  else {\n");
//  buffer_append (ut->presentation, "  else if (window.XMLHttpRequest) {\n");
  buffer_append (ut->presentation, "    xmlHttp = new XMLHttpRequest();\n");
  buffer_append (ut->presentation, "  }\n");
  buffer_append (ut->presentation, "  return xmlHttp;\n");
  buffer_append (ut->presentation, "}\n");

  buffer_append (ut->presentation, "function e(el) {\n");
  buffer_append (ut->presentation, "  d = document;\n");
  buffer_append (ut->presentation, "  if(d.getElementById) {\n");
  buffer_append (ut->presentation, "    return d.getElementById(el);\n");
  buffer_append (ut->presentation, "  } else if (d.all) {\n");
  buffer_append (ut->presentation, "    return d.all[el];\n");
  buffer_append (ut->presentation, "  }\n");
  buffer_append (ut->presentation, "}\n");

  build_script_template(ut);
}

void
build_onload (struct urobot_t *ut)
{
  buffer_append (ut->presentation, "window.onload = function() {\n");
  build_onload_template(ut);
  buffer_append (ut->presentation, "}\n");
  buffer_append (ut->presentation, "</script>\n");
  buffer_append (ut->presentation, "</head>\n");
}

void
build_body (struct urobot_t *ut)
{
  buffer_append (ut->presentation, "<body>\n");

  buffer_append (ut->presentation, "<h1 align=\"center\">");
  buffer_appendf (ut->presentation, "<tt>%s</tt><br/>",
                  _("uRobot UPnP Robot Server"));
  buffer_append (ut->presentation, _("Information Page"));
  buffer_append (ut->presentation, "</h1>");
  buffer_append (ut->presentation, "<br/>");

  buffer_append (ut->presentation, "<center>");
  buffer_append (ut->presentation, "<tr width=\"500\">");
  buffer_appendf (ut->presentation, "<b>%s :</b> %s<br/>",
                  _("Version"), VERSION);
  buffer_append (ut->presentation, "</tr>");
  buffer_appendf (ut->presentation, "<b>%s :</b> %s<br/>",
                  _("Device UDN"), ut->udn);
  buffer_append (ut->presentation, "</center><br/>");

  build_body_template(ut);

  buffer_appendf (ut->presentation,
                  "<form method=\"get\" action=\"%s\">\n", UROBOT_CGI);
  buffer_appendf (ut->presentation,
                  "<input type=\"hidden\" name=\"action\" value=\"%s\"/>\n",
                  CGI_ACTION_REFRESH);
  buffer_appendf (ut->presentation, "<input type=\"submit\" value=\"%s\"/>\n",
                  _("Refresh ..."));
  buffer_append (ut->presentation, "</form>\n");

  buffer_append (ut->presentation, "</body>\n");
}

int
build_presentation_page (struct urobot_t *ut)
{
  char *mycodeset = NULL;
  struct buffer_t *cam = NULL;

  printf ("build_presentation_page\n");
  if (!ut)
    return -1;

  if (ut->presentation)
    buffer_free (ut->presentation);
  ut->presentation = buffer_new ();

#if HAVE_LANGINFO_CODESET
  mycodeset = nl_langinfo (CODESET);
#endif
  if (!mycodeset)
    mycodeset = UTF8;

  buffer_append (ut->presentation, "<html>\n");
  buffer_append (ut->presentation, "<head>\n");
  buffer_appendf (ut->presentation, "<title>%s</title>\n",
                 _("uRobot Information Page"));
  buffer_appendf (ut->presentation,
                  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\"/>\n",
                  mycodeset);
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"pragma\" content=\"no-cache\"/>\n");
  buffer_append (ut->presentation,
                 "<meta http-equiv=\"expires\" content=\"1970-01-01\"/>\n");

  build_css (ut);
  build_javascript (ut);
  build_onload (ut);
  build_body (ut);

  buffer_append (ut->presentation, "</html>\n");

  return 0;
}
