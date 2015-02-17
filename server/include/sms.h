/*
 * sms.h : uRobot Sensor Monitoring Service header.
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

#ifndef SMS_H_
#define SMS_H_

#define SMS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
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
"      <name>GetAccelerator</name>" \
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
"    <stateVariable sendEvents=\"yes\">" \
"      <name>AcceleratorX</name>" \
"      <dataType>i4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0</minimum>" \
"        <maximum>1000</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>AcceleratorY</name>" \
"      <dataType>i4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0</minimum>" \
"        <maximum>1000</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>AcceleratorZ</name>" \
"      <dataType>i4</dataType>" \
"      <allowedValueRange>" \
"        <minimum>0</minimum>" \
"        <maximum>1000</maximum>" \
"      </allowedValueRange>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

#define SMS_DESCRIPTION_LEN strlen (SMS_DESCRIPTION)

#define SMS_LOCATION "/web/sms.xml"
#define SMS_CONTROL_LOCATION "/web/sms_control"
#define SMS_EVENTSUB_LOCATION "/web/sms_event"

#define SMS_SERVICE_INDEX 2
#define SMS_SERVICE_NAME "SensorMonitoring"
#define SMS_SERVICE_ID "urn:urobot-org:serviceId:SensorMonitoring"
#define SMS_SERVICE_TYPE "urn:schemas-urobot-org:service:SensorMonitoring:1"
#define SMS_SERVICE_VARIABLE_COUNT 7

/* The total count of sensor variables may be variable */

#define FRONT_DISTANCE	0
#define BACK_DISTANCE	1
#define LEFT_DISTANCE	2
#define RIGHT_DISTANCE	3
#define GET_DISTANCES	4
#define GET_ACCELERATOR	5

#define CGI_SMS_ACTION_GET_DISTANCES "GetDistances"
#define CGI_SMS_LABEL_FRONT_DISTANCE	 "Front"
#define CGI_SMS_LABEL_BACK_DISTANCE	 "Back"
#define CGI_SMS_LABEL_LEFT_DISTANCE	 "Left"
#define CGI_SMS_LABEL_RIGHT_DISTANCE	 "Right"

/* web content */
#define SMS_SCRIPT \
  "{{%AUTOESCAPE context=\"JAVASCRIPT\"}}"\
  "var smsHttp;\n\
  function poll() {\n\
    smsHttp = createXMLHttpRequest();\n\
    var url = \"{{UROBOT_CGI}}?service={{SERVICE_NAME}}&action={{CGI_SMS_ACTION_GET_DISTANCES}}\";\n\
    smsHttp.open(\"GET\", url, true);\n\
    smsHttp.setRequestHeader(\"pragma\", \"no-cache\");\n\
    smsHttp.onreadystatechange = pollCallback;\n\
    smsHttp.send(null);\n\
  }\n\
  function pollCallback() {\n\
    if (smsHttp.readyState == 4) {\n\
      if (smsHttp.status == 200) {\n\
        var result1 = smsHttp.responseXML.getElementsByTagName(\"result\")[0].firstChild.data;\n\
        var result2 = smsHttp.responseXML.getElementsByTagName(\"result\")[1].firstChild.data;\n\
        var result3 = smsHttp.responseXML.getElementsByTagName(\"result\")[2].firstChild.data;\n\
        var result4 = smsHttp.responseXML.getElementsByTagName(\"result\")[3].firstChild.data;\n\
        fd(parseInt(result1));\n\
        bd(parseInt(result2));\n\
        ld(parseInt(result3));\n\
        rd(parseInt(result4));\n\
        setTimeout(\"poll()\", 1000);\n\
      }\n\
    }\n\
  }\n\
  function s(el,n,max,text) {\n\
    e(el).innerHTML = '<table style=font-size:20pt width=604 height=40 border=0 cellpadding=1 cellspacing=0>'+\n\
                      '<tr><td width=250>' +\n\
                      text + '</td>' +\n\
                      '<td width=' + (350*n/max) + ' bgcolor=\"gray\">&nbsp;</td>' +\n\
                      '<td width=' + (350-350*n/max) + ' bgcolor=\"lightgray\">&nbsp;</td>' +\n\
                      '</table>';\n\
  }\n\
  function dc(n,d) {\n\
    return n.toFixed(d);\n\
  }\n\
  function fd(m) {\n\
    n = dc(m, 1);\n\
    s('{{CGI_SMS_LABEL_FRONT_DISTANCE}}',n,1000,'{{CGI_SMS_LABEL_FRONT_DISTANCE}} '+n+' m');\n\
  }\n\
  function bd(m) {\n\
    n = dc(m, 1);\n\
    s('{{CGI_SMS_LABEL_BACK_DISTANCE}}',n,1000,'{{CGI_SMS_LABEL_BACK_DISTANCE}} '+n+' m');\n\
  }\n\
  function ld(m) {\n\
    n = dc(m, 1);\n\
    s('{{CGI_SMS_LABEL_LEFT_DISTANCE}}',n,1000,'{{CGI_SMS_LABEL_LEFT_DISTANCE}} '+n+' m');\n\
  }\n\
  function rd(m) {\n\
    n = dc(m, 1);\n\
    s('{{CGI_SMS_LABEL_RIGHT_DISTANCE}}',n,1000,'{{CGI_SMS_LABEL_RIGHT_DISTANCE}} '+n+' m');\n\
  }\n\
  function i(n) {\n\
    e('i').innerHTML = n;\n\
  }\n\
  "
#define SMS_BODY \
  "{{%AUTOESCAPE context=\"HTML\"}}"\
  "<b>Sensor Monitoring</b><br/>\n\
  <input type=\"button\" style=font-size:20pt;width:250;height:60 value=\"Start\" name=\"Start\" id=\"start\" onclick=\"poll()\"/>\n\
 <div id=\"{{CGI_SMS_LABEL_FRONT_DISTANCE}}\"></div>\n\
 <div id=\"{{CGI_SMS_LABEL_BACK_DISTANCE}}\"></div>\n\
 <div id=\"{{CGI_SMS_LABEL_LEFT_DISTANCE}}\"></div>\n\
 <div id=\"{{CGI_SMS_LABEL_RIGHT_DISTANCE}}\"></div>\n\
  "

extern struct service_action_t sms_service_actions[];
extern char *sms_variablenames[];
extern char *sms_variables[SMS_SERVICE_VARIABLE_COUNT];
extern char *sms_defaultvalues[];

#ifdef __cplusplus
extern "C" {
#endif
int sms_init(struct service_t *service, int id);
#ifdef __cplusplus
};
#endif

#endif /* SMS_H_ */
