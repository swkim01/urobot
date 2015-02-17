#include <stdlib.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "cms.h"

/* Represent the CMS GetCurrentConnectionIDs action. */
#define SERVICE_CMS_ACTION_CON_ID "GetCurrentConnectionIDs"

/* Represent the CMS GetCurrentConnectionInfo action. */
#define SERVICE_CMS_ACTION_CON_INFO "GetCurrentConnectionInfo"

/* Represent the CMS SOURCE argument. */
#define SERVICE_CMS_ARG_SOURCE "Source"

/* Represent the CMS SINK argument. */
#define SERVICE_CMS_ARG_SINK "Sink"

/* Represent the CMS ConnectionIDs argument. */
#define SERVICE_CMS_ARG_CONNECTION_IDS "ConnectionIDs"

/* Represent the CMS ConnectionID argument. */
#define SERVICE_CMS_ARG_CONNECTION_ID "ConnectionID"

/* Represent the CMS RcsID argument. */
#define SERVICE_CMS_ARG_RCS_ID "RcsID"

/* Represent the CMS AVTransportID argument. */
#define SERVICE_CMS_ARG_TRANSPORT_ID "AVTransportID"

/* Represent the CMS PeerConnectionManager argument. */
#define SERVICE_CMS_ARG_PEER_CON_MANAGER "PeerConnectionManager"

/* Represent the CMS PeerConnectionID argument. */
#define SERVICE_CMS_ARG_PEER_CON_ID "PeerConnectionID"

/* Represent the CMS Direction argument. */
#define SERVICE_CMS_ARG_DIRECTION "Direction"

/* Represent the CMS Status argument. */
#define SERVICE_CMS_ARG_STATUS "Status"

/* Represent the CMS default connection ID value. */
#define SERVICE_CMS_DEFAULT_CON_ID "0"

/* Represent the CMS unknown connection ID value. */
#define SERVICE_CMS_UNKNOW_ID "-1"

/* Represent the CMS Output value. */
#define SERVICE_CMS_OUTPUT "Output"

/* Represent the CMS Success Status. */
#define SERVICE_CMS_STATUS_OK "OK"

static bool
cms_get_current_connection_ids (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  if (!event)
    return false;

  upnp_add_response (event, SERVICE_CMS_ARG_CONNECTION_IDS, "");

  return event->status;
}

static bool
cms_get_current_connection_info (struct device_t *dev, struct service_t *service, struct action_event_t *event)
{
  if (!event)
    return false;

  upnp_add_response (event, SERVICE_CMS_ARG_CONNECTION_ID,
                     SERVICE_CMS_DEFAULT_CON_ID);
  upnp_add_response (event, SERVICE_CMS_ARG_RCS_ID, SERVICE_CMS_UNKNOW_ID);
  upnp_add_response (event, SERVICE_CMS_ARG_TRANSPORT_ID,
                     SERVICE_CMS_UNKNOW_ID);

  upnp_add_response (event, SERVICE_CMS_ARG_PEER_CON_MANAGER, "");
  upnp_add_response (event, SERVICE_CMS_ARG_PEER_CON_ID,
                     SERVICE_CMS_UNKNOW_ID);
  upnp_add_response (event, SERVICE_CMS_ARG_DIRECTION, SERVICE_CMS_OUTPUT);
  upnp_add_response (event, SERVICE_CMS_ARG_STATUS, SERVICE_CMS_STATUS_OK);

  return event->status;
}

/* List of UPnP ConnectionManager Service actions */
struct service_action_t cms_service_actions[] = {
  { SERVICE_CMS_ACTION_CON_ID, cms_get_current_connection_ids },
  { SERVICE_CMS_ACTION_CON_INFO, cms_get_current_connection_info },
  { NULL, NULL }
};

int cms_init(struct service_t *service, int id)
{
  service->SCPDURL = CMS_LOCATION;
  service->SCPDDESC = CMS_DESCRIPTION;
  service->controlURL = CMS_CONTROL_LOCATION;
  service->eventSubURL = CMS_EVENTSUB_LOCATION;

  return 0;
}
