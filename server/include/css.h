/*
 * css.h : uRobot Car Sensor Monitoring Service header.
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

#ifndef CSS_H_
#define CSS_H_

#define CSS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
"    <action>" \
"      <name>SetFrontLight</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"          <defaultValue>0</defaultValue>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>SetFogLight</name>" \
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
"      <name>SetBreakLight</name>" \
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
"      <name>SetWinkerLight</name>" \
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
"      <name>GetFrontDistance</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>SensorValue</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_SensorValue</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetBackDistance</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>SensorValue</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_SensorValue</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetLeftDistance</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>SensorValue</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_SensorValue</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetRightDistance</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Command</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Command</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>SensorValue</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_SensorValue</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetSensorInfo</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>SensorID</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_SensorID</relatedStateVariable>" \
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
"      <defaultValue>0</defaultValue>" \
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
"      <name>A_ARG_TYPE_SensorValue</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_SensorID</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>FrontLight</name>" \
"      <dataType>Boolean</dataType>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>FogLight</name>" \
"      <dataType>Boolean</dataType>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>BreakLight</name>" \
"      <dataType>Boolean</dataType>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>WinkerLight</name>" \
"      <dataType>Boolean</dataType>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>FrontDistance</name>" \
"      <dataType>r4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0.0</minimum>" \
"        <maximum>1000.0</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>100.0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>BackDistance</name>" \
"      <dataType>r4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0.0</minimum>" \
"        <maximum>1000.0</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>100.0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>LeftDistance</name>" \
"      <dataType>r4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0.0</minimum>" \
"        <maximum>1000.0</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>100.0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>RightDistance</name>" \
"      <dataType>r4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0.0</minimum>" \
"        <maximum>1000.0</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>100.0</defaultValue>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

#define CSS_DESCRIPTION_LEN strlen (CSS_DESCRIPTION)

#define CSS_LOCATION "/web/css.xml"
#define CSS_CONTROL_LOCATION "/web/css_control"
#define CSS_EVENTSUB_LOCATION "/web/css_event"

#define CSS_SERVICE_INDEX 2
#define CSS_SERVICE_NAME "CarSensorMonitoring"
#define CSS_SERVICE_ID "urn:urobot-org:serviceId:CarSensorMonitoring"
#define CSS_SERVICE_TYPE "urn:schemas-urobot-org:service:CarSensorMonitoring:1"
#define CSS_SERVICE_VARIABLE_COUNT 8
/* The total count of sensor variables may be variable */

#define SERVICE_CSS_INDEX_FRONT_LIGHT		0
#define SERVICE_CSS_INDEX_FOG_LIGHT		1
#define SERVICE_CSS_INDEX_BREAK_LIGHT		2
#define SERVICE_CSS_INDEX_WINKER_LIGHT		3
#define SERVICE_CSS_INDEX_FRONT_DISTANCE	4
#define SERVICE_CSS_INDEX_BACK_DISTANCE		5
#define SERVICE_CSS_INDEX_LEFT_DISTANCE		6
#define SERVICE_CSS_INDEX_RIGHT_DISTANCE	7

#define CGI_CSS_ACTION_SET_FRONT_LIGHT "FrontLight"
#define CGI_CSS_ACTION_SET_FOG_LIGHT "FogLight"
#define CGI_CSS_ACTION_SET_BREAK_LIGHT "BreakLight"
#define CGI_CSS_ACTION_SET_WINKER_LIGHT "WinkerLight"

/* web content */
#define CSS_SCRIPT \
  "{{%AUTOESCAPE context=\"JAVASCRIPT\"}}"\
  "var cssHttp;\n\
  function sensor(action) {\n\
    cssHttp = createXMLHttpRequest();\n\
    var url = \"{{UROBOT_CGI}}?service={{SERVICE_NAME}}&action=\"+action;\n\
    cssHttp.open(\"GET\", url, true);\n\
    cssHttp.setRequestHeader(\"pragma\", \"no-cache\");\n\
    cssHttp.onreadystatechange = sensorCallback;\n\
    cssHttp.send(null);\n\
  }\n\
  function sensorCallback() {\n\
    if (cssHttp.readyState == 4) {\n\
      if (cssHttp.status == 200) {\n\
        var result = cssHttp.responseXML.getElementsByTagName(\"result\")[0].firstChild.data;\n\
        e('sensor').innerHTML = result;\n\
      }\n\
    }\n\
  }\n\
  function sensor_fr() {\n\
    sensor('{{CGI_CSS_ACTION_SET_FRONT_LIGHT}}');\n\
  }\n\
  function sensor_fg() {\n\
    sensor('{{CGI_CSS_ACTION_SET_FOG_LIGHT}}');\n\
  }\n\
  function sensor_br() {\n\
    sensor('{{CGI_CSS_ACTION_SET_BREAK_LIGHT}}');\n\
  }\n\
  function sensor_wk() {\n\
    sensor('{{CGI_CSS_ACTION_SET_WINKER_LIGHT}}');\n\
  }\n\
  "
#define CSS_BODY \
  "{{%AUTOESCAPE context=\"HTML\"}}"\
  "<b>Sensor Control</b><br/>\n\
  <input type=\"button\" style=font-size:20pt;width:160;height:60 value=\"{{CGI_CSS_ACTION_SET_FRONT_LIGHT}}\" id=\"{{CGI_CSS_ACTION_SET_FRONT_LIGHT}}\" onclick=\"sensor_fr();\"/>\n\
  <input type=\"button\" style=font-size:20pt;width:160;height:60 value=\"{{CGI_CSS_ACTION_SET_FOG_LIGHT}}\" id=\"{{CGI_CSS_ACTION_SET_FOG_LIGHT}}\" onclick=\"sensor_fg();\"/>\n\
  <input type=\"button\" style=font-size:20pt;width:160;height:60 value=\"{{CGI_CSS_ACTION_SET_BREAK_LIGHT}}\" id=\"{{CGI_CSS_ACTION_SET_BREAK_LIGHT}}\" onclick=\"sensor_br();\"/>\n\
  <input type=\"button\" style=font-size:20pt;width:160;height:60 value=\"{{CGI_CSS_ACTION_SET_WINKER_LIGHT}}\" id=\"{{CGI_CSS_ACTION_SET_WINKER_LIGHT}}\" onclick=\"sensor_wk();\"/>\n\
  <br/>\n\
  <div id=\"sensor\"></div>\n\
  "
extern struct service_action_t css_service_actions[];
extern const char *css_variablenames[];
extern char *css_variables[CSS_SERVICE_VARIABLE_COUNT];
extern char *css_defaultvalues[];
#ifdef __cplusplus
extern "C" {
#endif
int css_init(struct service_t *service, int id);
#ifdef __cplusplus
};
#endif

#endif /* CSS_H_ */
