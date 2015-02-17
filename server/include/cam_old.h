#ifndef CAM_H_
#define CAM_H_

#define CAM_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
"    <action>" \
"      <name>GetSystemUpdateID</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Id</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>SystemUpdateID</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetVideoURL</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>CameraIndex</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_CameraIndex</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>CameraType</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_CameraType</relatedStateVariable>" \
"       </argument>" \
"       <argument>" \
"          <name>ObjectID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>VideoURL</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_VideoURL</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetEncoding</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Encoding</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>Encoding</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>SetEncoding</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Encoding</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>Encoding</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetResolution</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Resolution</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>Resolution</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>SetResolution</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Resolution</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>Resolution</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>Status</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Status</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetFrameRate</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>FrameRate</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>FrameRate</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>SetFrameRate</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>FrameRate</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>FrameRate</relatedStateVariable>" \
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
"    <stateVariable sendEvents=\"yes\">" \
"      <name>SystemUpdateID</name>" \
"      <dataType>i4</dataType>" \
"      <defaultValue>0</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>Encoding</name>" \
"      <dataType>string</dataType>" \
"      <defaultValue>image/jpeg</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>Resolution</name>" \
"      <dataType>string</dataType>" \
"      <defaultValue>640x480</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>FrameRate</name>" \
"      <dataType>i4</dataType>" \
"      <defaultValue>5</defaultValue>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_CameraIndex</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_CameraType</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_ObjectID</name>" \
"      <dataType>i4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_UpdateID</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Encoding</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Resolution</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_VideoURL</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_FrameRate</name>" \
"      <dataType>i4</dataType>" \
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
"  </serviceStateTable>" \
"</scpd>"

#define CAM_DESCRIPTION_LEN strlen (CAM_DESCRIPTION)

#define CAM_LOCATION "/web/cam.xml"
#define CAM_VIDEO_LOCATION "?action=stream"
#define CAM_PORT	8080

#define CAM_SERVICE_ID "urn:urobot-org:serviceId:CameraMonitoring"
#define CAM_SERVICE_TYPE "urn:schemas-urobot-org:service:CameraMonitoring:1"

/* action type */
#define GET_SYSTEM_UPDATE_ID	0
#define GET_ENCODING		1
#define SET_ENCODING		2
#define GET_RESOLUTION		3
#define SET_RESOLUTION		4
#define GET_FRAMERATE		5
#define SET_FRAMERATE		6

extern struct service_action_t cam_service_actions[];

#endif /* CAM_H_ */
