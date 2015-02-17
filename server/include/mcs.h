/*
 * mcs.h : uRobot Motion Controller Service header.
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

#ifndef MCS_H_
#define MCS_H_

#define MCS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
"    <action>" \
"      <name>GoForward</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GoBackward</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>Stop</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>TurnLeft</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>TurnRight</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>Move</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Velocity</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_FloatValue</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Angular</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_FloatValue</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetMotionInfo</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>MotionID</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_MotionID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Direction</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"  </actionList>" \
"  <serviceStateTable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Command</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_FloatValue</name>" \
"      <dataType>f4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Status</name>" \
"     <dataType>string</dataType>" \
"      <allowedValueList>" \
"        <allowedValue>OK</allowedValue>" \
"        <allowedValue>ContentFormatMismatch</allowedValue>" \
"        <allowedValue>InsufficientBandwidth</allowedValue>" \
"        <allowedValue>UnreliableChannel</allowedValue>" \
"        <allowedValue>Unknown</allowedValue>" \
"      </allowedValueList>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_MotionID</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Direction</name>" \
"      <dataType>string</dataType>" \
"      <allowedValueList>" \
"        <allowedValue>Input</allowedValue>" \
"        <allowedValue>Output</allowedValue>" \
"      </allowedValueList>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

#define MCS_DESCRIPTION_LEN strlen (MCS_DESCRIPTION)

#define MCS_LOCATION "/web/mcs.xml"
#define MCS_CONTROL_LOCATION "/web/mcs_control"
#define MCS_EVENTSUB_LOCATION "/web/mcs_event"

#define MCS_SERVICE_NAME "MotionController"
#define MCS_SERVICE_ID "urn:urobot-org:serviceId:MotionController"
#define MCS_SERVICE_TYPE "urn:schemas-urobot-org:service:MotionController:1"

/* motion mode */
#define FORWARD_MOTION		0
#define BACKWARD_MOTION		1
#define STOP_MOTION		2
#define LEFTTURN_MOTION		3
#define RIGHTTURN_MOTION	4
#define MOVE_MOTION		5
#define INFO_MOTION		6

#define CGI_MCS_ACTION_GO_FORWARD "GoForward"
#define CGI_MCS_ACTION_GO_BACKWARD "GoBackward"
#define CGI_MCS_ACTION_STOP "Stop"
#define CGI_MCS_ACTION_TURN_LEFT "TurnLeft"
#define CGI_MCS_ACTION_TURN_RIGHT "TurnRight"
#define CGI_MCS_ACTION_MOVE "Move"

/* web content */
#define MCS_CSS \
  "{{%AUTOESCAPE context=\"CSS\"}}"\
  "  #joystick {\n\
         width	: 300px;\n\
         height	: 300px;\n\
         overflow	: hidden;\n\
         padding	: 0;\n\
         margin	: 0;\n\
         background-color: #BBB;\n\
         -webkit-user-select	: none;\n\
         -moz-user-select	: none;\n\
     }"

#define MCS_SCRIPT \
  "{{%AUTOESCAPE context=\"JAVASCRIPT\"}}"\
  "var prevTime = 0;\n\
  var nowTime = 0;\n\
  var mcsHttp = 0;\n\
  var ack = 1;\n\
  function motion(action, enableAck) {\n\
    if (enableAck != true || ack == 1) {\n\
      mcsHttp = createXMLHttpRequest();\n\
      var url = \"{{UROBOT_CGI}}?service={{SERVICE_NAME}}&action=\"+action;\n\
      mcsHttp.open(\"GET\", url, true);\n\
      mcsHttp.setRequestHeader(\"pragma\", \"no-cache\");\n\
      mcsHttp.onreadystatechange = motionCallback;\n\
      prevTime = new Date();\n\
      ack = 0;\n\
      mcsHttp.send(null);\n\
    }\n\
  }\n\
  function motionCallback() {\n\
    if (mcsHttp.readyState == 4) {\n\
      if (mcsHttp.status == 200) {\n\
        nowTime = new Date();\n\
        var result = mcsHttp.responseXML.getElementsByTagName(\"result\")[0].firstChild.data;\n\
        e('motion').innerHTML = result;\n\
        ack = 1;\n\
      }\n\
    }\n\
  }\n\
  function motion_f() {\n\
    motion('{{CGI_MCS_ACTION_GO_FORWARD}}', false);\n\
  }\n\
  function motion_b() {\n\
    motion('{{CGI_MCS_ACTION_GO_BACKWARD}}', false);\n\
  }\n\
  function motion_s() {\n\
    motion('{{CGI_MCS_ACTION_STOP}}', false);\n\
  }\n\
  function motion_l() {\n\
    motion('{{CGI_MCS_ACTION_TURN_LEFT}}', false);\n\
  }\n\
  function motion_r() {\n\
    motion('{{CGI_MCS_ACTION_TURN_RIGHT}}', false);\n\
  }\n\
  function toggleMotion() {\n\
    var motion = document.getElementById('controls')\n\
    var joy = document.getElementById('joystick');\n\
    showhide(motion);\n\
    showhide(joy);\n\
  }\n\
  function showhide(what){\n\
    if (what.style.display=='') {\n\
      what.style.display='none';\n\
      return ;\n\
    }\n\
    else if (what.style.display=='none') {\n\
      what.style.display='';\n\
    }\n\
  }\n\
  </script>\n\
  <script src=\"joystick.js\"></script>\n\
  <script>\n\
  var joystick;\n\
  "

#define MCS_ONLOAD \
  "{{%AUTOESCAPE context=\"JAVASCRIPT\"}}"\
  "  joystick	= new Joystick({\n\
      container	: document.getElementById('joystick'),\n\
      mouseSupport	: true\n\
    });\n\
    document.getElementById('joystick').style.display='none';\n\
  "

#define MCS_BODY \
  "{{%AUTOESCAPE context=\"HTML\"}}"\
  "<b>Motion Control</b><br/>\n\
  <input type=\"button\" style=font-size:20pt;width:250;height:60 value=\"Toggle Motion Method\" id=\"ToggleMotion\" onclick=\"toggleMotion();\"/><br/>\n\
  <div id=\"joystick\"></div>\n\
  <table id=\"controls\">\n\
  <tr><th/><th>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60; value=\"&uarr;\" id=\"{{CGI_MCS_ACTION_GO_FORWARD}}\" onclick=\"motion_f();\"/>\n\
  </th></tr>\n\
  <tr><th>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 value=\"&larr;\" id=\"{{CGI_MCS_ACTION_TURN_LEFT}}\" onclick=\"motion_l();\"/>\n\
  </th><th>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 value=\"X\" id=\"{{CGI_MCS_ACTION_STOP}}\" onclick=\"motion_s();\"/>\n\
  </th><th>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 value=\"&rarr;\" id=\"{{CGI_MCS_ACTION_TURN_RIGHT}}\" onclick=\"motion_r();\"/>\n\
  </th></tr>\n\
  <tr><th/><th>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 value=\"&darr;\" id=\"{{CGI_MCS_ACTION_GO_BACKWARD}}\" onclick=\"motion_b();\"/>\n\
  </th></tr>\n\
  </table><br/>\n\
  <div id=\"motion\"></div>\n\
  "

/*
  <div id=\"controls\">\n\
  <ul>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60; id=\"{{CGI_MCS_ACTION_GO_FORWARD}}\" value=\"&uarr;\" onClick=\"motion_f()\">\n\
  </ul>\n\
  <ul>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 id=\"{{CGI_MCS_ACTION_TURN_LEFT}}\" value=\"&larr;\" onClick=\"motion_l()\">\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 id=\"{{CGI_MCS_ACTION_STOP}}\" value=\"X\" onClick=\"motion_s()\">\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 id=\"{{CGI_MCS_ACTION_TURN_RIGHT}}\" value=\"&rarr;\" onClick=\"motion_r()\">\n\
  </ul>\n\
  <ul>\n\
    <input type=\"button\" style=font-size:20pt;width:60;height:60 id=\"{{CGI_MCS_ACTION_GO_BACKWARD}}\" value=\"&darr;\" onClick=\"motion_b()\">\n\
  </ul>\n\
  </div>\n\

*/

extern struct service_action_t mcs_service_actions[];

#ifdef __cplusplus
extern "C" {
#endif
int mcs_init(struct service_t *service, int id);
#ifdef __cplusplus
};
#endif

#endif /* MCS_H_ */
