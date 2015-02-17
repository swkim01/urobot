#ifndef STS_H_
#define STS_H_

#define STS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
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
"  </actionList>" \
"  <serviceStateTable>" \
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
"      <name>A_ARG_TYPE_VideoURL</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

#define STS_DESCRIPTION_LEN strlen (STS_DESCRIPTION)

#define STS_LOCATION "/web/sts.xml"
#define STS_CONTROL_LOCATION "/web/sts_control"
#define STS_EVENTSUB_LOCATION "/web/sts_event"

#define STS_VIDEO_LOCATION "?action=stream"
#define STS_PORT	8008

#define STS_SERVICE_ID "urn:urobot-org:serviceId:Streaming"
#define STS_SERVICE_TYPE "urn:schemas-urobot-org:service:Streaming:1"

/* action type */
#define GET_INFO	0

/* web content */
#define STS_SCRIPT \
  "{{%AUTOESCAPE context=\"JAVASCRIPT\"}}"\
  "var imageNr = 0;\n\
  var finished = new Array();\n\
  var paused = false;\n\
  function createImageLayer() {\n\
    var img = new Image();\n\
    img.style.position = \"absolute\";\n\
    img.style.zIndex = -1;\n\
    img.onload = imageOnload;\n\
    img.onclick = imageOnclick;\n\
    img.src = \"{{STS_STREAMING_ADDRESS}}&n=\" + (++imageNr);\n\
    var webcam = e('webcam');\n\
    webcam.insertBefore(img, webcam.firstChild);\n\
  }\n\
  function imageOnload() {\n\
    this.style.zIndex = imageNr;\n\
    while (1 < finished.length) {\n\
      var del = finished.shift();\n\
      del.parentNode.removeChild(del);\n\
    }\n\
    finished.push(this);\n\
    if (!paused) createImageLayer();\n\
  }\n\
  function imageOnclick() {\n\
    paused = !paused;\n\
    if (!paused) createImageLayer();\n\
  }\n\
  "

#define STS_ONLOAD \
  "{{%AUTOESCAPE context=\"JAVASCRIPT\"}}"\
  "  createImageLayer();\n\
  "

#define STS_BODY \
  "{{%AUTOESCAPE context=\"HTML\"}}"\
  "<b>Camera Monitoring</b><br/>\n\
  <div id=\"webcam\"><noscript><img src=\"{{STS_STREAMING_ADDRESS}}\" width=\"320\" height=\"240\" /></noscript></div>\n\
  <br/>\n\
  "

extern struct service_action_t sts_service_actions[];
int sts_browse_metadata (struct device_t *dev, struct service_t *service, struct buffer_t *out, int index, int type);
#ifdef __cplusplus
extern "C" {
#endif
int sts_init(struct service_t *service, int id);
#ifdef __cplusplus
};
#endif

#endif /* STS_H_ */
