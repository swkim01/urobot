#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#endif

#if (defined(__APPLE__))
#include <net/route.h>
#endif

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#if (defined(HAVE_SETLOCALE) && defined(CONFIG_NLS))
# include <locale.h>
#endif

#include "config.h"
#include "urobot.h"
#include "device.h"
#include "services.h"
#include "http.h"
#include "cfgparser.h"
#include "gettext.h"
#include "trace.h"
#include "buffer.h"
#include "target.h"
#include "sms.h"

struct urobot_t *ut = NULL;

static struct urobot_t * urobot_new (void)
    __attribute__ ((malloc));

static struct urobot_t *
urobot_new (void)
{
  struct urobot_t *ut = (struct urobot_t *) malloc (sizeof (struct urobot_t));
  if (!ut)
    return NULL;

  ut->name = strdup (DEFAULT_UROBOT_NAME);
  ut->interface = strdup (DEFAULT_UROBOT_IFACE);
  ut->model_name = strdup (DEFAULT_UROBOT_NAME);
  ut->starting_id = STARTING_ENTRY_ID_DEFAULT;
  ut->init = 0;
  ut->handle = 0;
  ut->udn = NULL;
  ut->ip = NULL;
  ut->port = 49212; /* Randomly attributed by libupnp */
  ut->presentation = NULL;
  ut->use_presentation = true;
  ut->verbose = false;
  ut->daemon = false;
  ut->cfg_file = NULL;
  ut->pid_file = NULL;

  pthread_mutex_init (&ut->web_mutex, NULL);
  pthread_mutex_init (&ut->termination_mutex, NULL);
  pthread_cond_init (&ut->termination_cond, NULL);

  //init_device(ut, (int)NULL, (char **)NULL);

  return ut;
}

static void
urobot_free (struct urobot_t *ut)
{
  if (!ut)
    return;

  if (ut->name)
    free (ut->name);
  if (ut->interface)
    free (ut->interface);
  if (ut->model_name)
    free (ut->model_name);
  if (ut->udn)
    free (ut->udn);
  if (ut->ip)
    free (ut->ip);
  if (ut->presentation)
    buffer_free (ut->presentation);
  if (ut->cfg_file)
    free (ut->cfg_file);
    
  if (ut->pid_file)
    free (ut->pid_file);

  pthread_mutex_destroy (&ut->web_mutex);
  pthread_mutex_destroy (&ut->termination_mutex);
  pthread_cond_destroy (&ut->termination_cond);

  //finish_device (ut, (int)NULL, (char **)NULL);

  free (ut);
}

static void
urobot_signal_exit (void)
{
  pthread_mutex_lock (&ut->termination_mutex);
  pthread_cond_signal (&ut->termination_cond);
  pthread_mutex_unlock (&ut->termination_mutex);
}

static void
handle_action_request (struct Upnp_Action_Request *request)
{
  struct service_t *service;
  struct service_action_t *action;
  char val[32];
  uint32_t ip;
  int i;

  if (!request || !ut)
    return;

  if (request->ErrCode != UPNP_E_SUCCESS)
    return;

  if (strcmp (request->DevUDN + 5, ut->udn))
    return;

#if UPNP_VERSION < 10613
  ip = request->CtrlPtIPAddr.s_addr;
#else
  struct sockaddr_in LocalAddr;
  memcpy(&LocalAddr, &request->CtrlPtIPAddr, sizeof(struct sockaddr_in));
  ip = LocalAddr.sin_addr.s_addr;
#endif /* UPNP_VERSION < 10613 */
  ip = ntohl (ip);
  sprintf (val, "%d.%d.%d.%d",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

  if (ut->verbose)
  {
    DOMString str = ixmlPrintDocument (request->ActionRequest);
    log_verbose ("***************************************************\n");
    log_verbose ("**             New Action Request                **\n");
    log_verbose ("***************************************************\n");
    log_verbose ("ServiceID: %s\n", request->ServiceID);
    log_verbose ("ActionName: %s\n", request->ActionName);
    log_verbose ("CtrlPtIP: %s\n", val);
    log_verbose ("Action Request:\n%s\n", str);
    ixmlFreeDOMString (str);
  }

  for (i=0; i<ut->devcnt; i++) {
    if (find_service_action (ut->dev[i], request, &service, &action))
      {
        struct action_event_t event;

        event.request = request;
        event.status = true;
        event.service = service;

        if (action->function (ut->dev[i], service, &event) && event.status)
          request->ErrCode = UPNP_E_SUCCESS;

        if (ut->verbose)
        {
          DOMString str = ixmlPrintDocument (request->ActionResult);
          log_verbose ("Action Result:\n%s", str);
          log_verbose ("***************************************************\n");
          log_verbose ("\n");
          ixmlFreeDOMString (str);
        }
  
        return;
    }
  }

  if (service) /* Invalid Action name */
    strcpy (request->ErrStr, "Unknown Service Action");
  else /* Invalid Service name */
    strcpy (request->ErrStr, "Unknown Service ID");

  request->ActionResult = NULL;
  request->ErrCode = UPNP_SOAP_E_INVALID_ACTION;
}

static void
handle_subscription_request (struct Upnp_Subscription_Request *request)
{
  struct service_t *service;
  int i;

  if (!request || !ut)
    return;

  if (strcmp (request->UDN + 5, ut->udn))
    return;

  if (ut->verbose)
  {
    log_verbose ("***************************************************\n");
    log_verbose ("**          New Subscription Request             **\n");
    log_verbose ("***************************************************\n");
    log_verbose ("ServiceID: %s\n", request->ServiceId);
    log_verbose ("SubscriptionID: %s\n", request->Sid);
  }
  
  for (i=0; i<ut->devcnt; i++) {
    if (find_service (ut->dev[i], request, &service))
    {
      if (service->variablenames != NULL)
          printf("%s(0/%d): %s\n", service->variablenames[0], service->variable_count, service->variablevalues[0]);

      UpnpAcceptSubscription(ut->handle, request->UDN,
                                      request->ServiceId,
                                      (const char **)service->variablenames,
                                      (const char **)service->variablevalues,
                                      service->variable_count,
                                      request->Sid);
      //if (!strcmp (request->ServiceId, SMS_SERVICE_ID))
      //  start_sensor_thread();
      if (!service->running) {
        start_service_thread(service);
      }
      if (ut->verbose)
      {
        log_verbose ("Subscription Accepted:%s\n", request->ServiceId);
        log_verbose ("***************************************************\n");
        log_verbose ("\n");
      }
    }
  }

  /*ErrCode = UPNP_E_SUBSCRIBE_UNACCEPTED; */
}

static int
device_callback_event_handler (Upnp_EventType type, void *event,
                               void *cookie __attribute__((unused)))
{
  switch (type)
    {
    case UPNP_CONTROL_ACTION_REQUEST:
      handle_action_request ((struct Upnp_Action_Request *) event);
      break;
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
      handle_subscription_request ((struct Upnp_Subscription_Request *) event);
      break;
    case UPNP_CONTROL_ACTION_COMPLETE:
    case UPNP_CONTROL_GET_VAR_REQUEST:
      break;
    default:
      break;
    }

  return 0;
}

static int
finish_upnp (struct urobot_t *ut)
{
  if (!ut)
    return -1;

  log_info (_("Stopping UPnP Service ...\n"));
  UpnpUnRegisterRootDevice (ut->handle);
  UpnpFinish ();

  return UPNP_E_SUCCESS;
}

static int
init_upnp (struct urobot_t *ut)
{
  char *description = NULL;
  int res;
  size_t len;

  if (!ut || !ut->name || !ut->udn || !ut->ip)
    return -1;

  len = strlen (UPNP_DESCRIPTION) + strlen (ut->name)
    + strlen (ut->model_name) + strlen (ut->udn) + 1;
  description = (char *) malloc (len * sizeof (char));
  memset (description, 0, len);
  sprintf (description, UPNP_DESCRIPTION, ut->name, ut->model_name, ut->udn);

  log_info (_("Initializing UPnP subsystem ...\n"));
  res = UpnpInit (ut->ip, ut->port);
  log_info (_("ip=%s, port=%d\n"), ut->ip, ut->port);
  if (res != UPNP_E_SUCCESS)
  {
    log_error (_("Cannot initialize UPnP subsystem\n"));
    return -1;
  }

  if (UpnpSetMaxContentLength (UPNP_MAX_CONTENT_LENGTH) != UPNP_E_SUCCESS)
    log_info (_("Could not set Max content UPnP\n"));

  ut->port = UpnpGetServerPort();
  log_info (_("UPnP RobotServer listening on %s:%d\n"),
            UpnpGetServerIpAddress (), ut->port);

  UpnpEnableWebserver (TRUE);

#if UPNP_VERSION < 10613
  res = UpnpSetVirtualDirCallbacks (&virtual_dir_callbacks);
#else
  res = http_init();
#endif /* UPNP_VERSION < 10613 */
  if (res != UPNP_E_SUCCESS)
  {
    log_error (_("Cannot set virtual directory callbacks\n"));
    free (description);
    return -1;
  }

  res = UpnpAddVirtualDir (VIRTUAL_DIR);
  if (res != UPNP_E_SUCCESS)
  {
    log_error (_("Cannot add virtual directory for web server\n"));
    free (description);
    return -1;
  }

  res = UpnpRegisterRootDevice2 (UPNPREG_BUF_DESC, description, 0, 1,
                                 device_callback_event_handler,
                                 NULL, &(ut->handle));
  if (res != UPNP_E_SUCCESS)
  {
    log_error (_("Cannot register UPnP device\n"));
    free (description);
    return -1;
  }

  res = UpnpUnRegisterRootDevice (ut->handle);
  if (res != UPNP_E_SUCCESS)
  {
    log_error (_("Cannot unregister UPnP device\n"));
    free (description);
    return -1;
  }

  res = UpnpRegisterRootDevice2 (UPNPREG_BUF_DESC, description, 0, 1,
                                 device_callback_event_handler,
                                 NULL, &(ut->handle));
  if (res != UPNP_E_SUCCESS)
  {
    log_error (_("Cannot register UPnP device\n"));
    free (description);
    return -1;
  }

  log_info (_("Sending UPnP advertisement for device ...\n"));
  UpnpSendAdvertisement (ut->handle, 1800);

  log_info (_("Listening for control point connections ...\n"));

  if (description)
    free (description);

  init_variables();

  return 0;
}

static bool
has_iface (char *interface)
{
#ifdef HAVE_IFADDRS_H
  struct ifaddrs *itflist, *itf;

  if (!interface)
    return false;

  if (getifaddrs (&itflist) < 0)
  {
    perror ("getifaddrs");
    return false;
  }

  itf = itflist;
  while (itf)
  {
    if ((itf->ifa_flags & IFF_UP)
        && !strncmp (itf->ifa_name, interface, IFNAMSIZ))
    {
      log_error (_("Interface %s is down.\n"), interface);
      log_error (_("Recheck uRobot's configuration and try again !\n"));
      freeifaddrs (itflist);
      return true;
    }
    itf = itf->ifa_next;
  }

  freeifaddrs (itf);
#else  
  int sock, i, n;
  struct ifconf ifc;
  struct ifreq ifr;
  char buff[8192];

  if (!interface)
    return false;

  /* determine UDN according to MAC address */
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    return false;
  }

  /* get list of available interfaces */
  ifc.ifc_len = sizeof (buff);
  ifc.ifc_buf = buff;

  if (ioctl (sock, SIOCGIFCONF, &ifc) < 0)
  {
    perror ("ioctl");
    close (sock);
    return false;
  }

  n = ifc.ifc_len / sizeof (struct ifreq);
  for (i = n - 1 ; i >= 0 ; i--)
  {
    ifr = ifc.ifc_req[i];

    if (strncmp (ifr.ifr_name, interface, IFNAMSIZ))
      continue;

    if (ioctl (sock, SIOCGIFFLAGS, &ifr) < 0)
    {
      perror ("ioctl");
      close (sock);
      return false;
    }

    if (!(ifr.ifr_flags & IFF_UP))
    {
      /* interface is down */
      log_error (_("Interface %s is down.\n"), interface);
      log_error (_("Recheck uRobot's configuration and try again !\n"));
      close (sock);
      return false;
    }

    /* found right interface */
    close (sock);
    return true;
  }
  close (sock);
#endif
  
  log_error (_("Can't find interface %s.\n"),interface);
  log_error (_("Recheck uRobot's configuration and try again !\n"));

  return false;
}

static char *
create_udn (char *interface)
{
  int sock = -1;
  char *buf;
  unsigned char *ptr;

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
  int mib[6];
  size_t len;
  struct if_msghdr *ifm;
  struct sockaddr_dl *sdl;
#else /* Linux */
  struct ifreq ifr;
#endif

  if (!interface)
    return NULL;

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
  mib[0] = CTL_NET;
  mib[1] = AF_ROUTE;
  mib[2] = 0;
  mib[3] = AF_LINK;
  mib[4] = NET_RT_IFLIST;

  mib[5] = if_nametoindex (interface);
  if (mib[5] == 0)
  {
    perror ("if_nametoindex");
    return NULL;
  }

  if (sysctl (mib, 6, NULL, &len, NULL, 0) < 0)
  {
    perror ("sysctl");
    return NULL;
  }

  buf = malloc (len);
  if (sysctl (mib, 6, buf, &len, NULL, 0) < 0)
  {
    perror ("sysctl");
    return NULL;
  }

  ifm = (struct if_msghdr *) buf;
  sdl = (struct sockaddr_dl*) (ifm + 1);
  ptr = (unsigned char *) LLADDR (sdl);
#else /* Linux */
  /* determine UDN according to MAC address */
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    return NULL;
  }

  strcpy (ifr.ifr_name, interface);
  strcpy (ifr.ifr_hwaddr.sa_data, "");

  if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
  {
    perror ("ioctl");
    return NULL;
  }

  buf = (char *) malloc (64 * sizeof (char));
  memset (buf, 0, 64);
  ptr = (unsigned char *) ifr.ifr_hwaddr.sa_data;
#endif /* (defined(BSD) || defined(__FreeBSD__)) */

  srand(time(0));
  int random_uuid = rand() % 0xffff;
  snprintf (buf, 64, "%s-%04x-%02x%02x%02x%02x%02x%02x", DEFAULT_UUID,
   	    random_uuid,
            (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
            (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

  if (sock)
    close (sock);

  return buf;
}

static char *
get_iface_address (char *interface)
{
  int sock;
  uint32_t ip;
  struct ifreq ifr;
  char *val;

  if (!interface)
    return NULL;

  /* determine UDN according to MAC address */
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror ("socket");
    return NULL;
  }

  strcpy (ifr.ifr_name, interface);
  ifr.ifr_addr.sa_family = AF_INET;

  if (ioctl (sock, SIOCGIFADDR, &ifr) < 0)
  {
    perror ("ioctl");
    close (sock);
    return NULL;
  }

  val = (char *) malloc (16 * sizeof (char));
  ip = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;
  ip = ntohl (ip);
  sprintf (val, "%d.%d.%d.%d",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

  close (sock);

  return val;
}

static int
restart_upnp (struct urobot_t *ut)
{
  finish_upnp (ut);

  if (ut->udn)
    free (ut->udn);
  ut->udn = create_udn (ut->interface);
  if (!ut->udn)
    return -1;

  if (ut->ip)
    free (ut->ip);
  ut->ip = get_iface_address (ut->interface);
  if (!ut->ip)
    return -1;

  return (init_upnp (ut));
}

static void
UPnPBreak (int s __attribute__ ((unused)))
{
  urobot_signal_exit ();
}

static void
reload_config (int s __attribute__ ((unused)))
{
  struct urobot_t *ut2;
  bool reload = false;

  log_info (_("Reloading configuration...\n"));

  ut2 = urobot_new ();
  if (!ut || !ut2)
    return;

  if (parse_config_file (ut2) < 0)
    return;

  if (ut->name && strcmp (ut->name, ut2->name))
  {
    free (ut->name);
    ut->name = ut2->name;
    ut2->name = NULL;
    reload = true;
  }

  if (ut->interface && strcmp (ut->interface, ut2->interface))
  {
    if (!has_iface (ut2->interface))
    {
      urobot_free (ut2);
      raise (SIGINT);
    }
    else
    {
      free (ut->interface);
      ut->interface = ut2->interface;
      ut2->interface = NULL;
      reload = true;
    }
  }

  if (ut->port != ut2->port)
  {
    ut->port = ut2->port;
    reload = true;
  }

  if (reload)
  {
    if (restart_upnp (ut) < 0)
    {
      urobot_free (ut2);
      raise (SIGINT);
    }
  }

  urobot_free (ut2);
}

inline void
display_headers (void)
{
  printf (_("%s (version %s), a lightweight UPnP Robot Server.\n"),
          PACKAGE_NAME, VERSION);
  printf (_("Seong-Woo Kim (C) 2011-2015, for uRobot Team.\n"));
  printf (_("See https://github.com/swkim01/urobot/ for updates.\n"));
}

inline static void
setup_i18n(void)
{
#ifdef CONFIG_NLS
#ifdef HAVE_SETLOCALE
  setlocale (LC_ALL, "");
#endif
#if (!defined(BSD) && !defined(__FreeBSD__))
  bindtextdomain (PACKAGE, LOCALEDIR);
#endif
  textdomain (PACKAGE);
#endif
}


struct urobot_t *
urobot_init (int argc, char **argv)
{
  ut = urobot_new ();
  if (!ut)
    return (struct urobot_t *) EXIT_FAILURE;

  setup_i18n ();

  /* Parse args before cfg file, as we may override the default file */
  if (parse_command_line (ut, argc, argv) < 0)
  {
    urobot_free (ut);
    return (struct urobot_t *) EXIT_SUCCESS;
  }

  if (parse_config_file (ut) < 0)
  {
    /* fprintf here, because syslog not yet ready */
    fprintf (stderr, _("Warning: can't parse file \"%s\".\n"),
             ut->cfg_file ? ut->cfg_file : SYSCONFDIR "/" UROBOT_CONFIG_FILE);
  }

  if (ut->daemon)
  {
    /* starting syslog feature as soon as possible */
    start_log ();
  }

  if (!has_iface (ut->interface))
  {
    urobot_free (ut);
    return (struct urobot_t *) EXIT_FAILURE;
  }

  ut->udn = create_udn (ut->interface);
  if (!ut->udn)
  {
    urobot_free (ut);
    return (struct urobot_t *) EXIT_FAILURE;
  }

  ut->ip = get_iface_address (ut->interface);
  if (!ut->ip)
  {
    urobot_free (ut);
    return (struct urobot_t *) EXIT_FAILURE;
  }

  if (ut->daemon)
  {
    int err;
    err = daemon (0, 0);
    if (err == -1)
    {
      log_error (_("Error: failed to daemonize program : %s\n"),
                 strerror (err));
      urobot_free (ut);
      return (struct urobot_t *) EXIT_FAILURE;
    }
  }
  else
  {
    display_headers ();
  }

  if (ut->pid_file)
  {
  	FILE* fp;
  	fp = fopen(ut->pid_file, "w");
  	if (fp)
	{
		char buf[128] = "";
		sprintf(buf, "%d", getpid());
		fwrite(buf, sizeof(char), strlen(buf), fp);
		fclose(fp);
  	}
  }
  
  signal (SIGINT, UPnPBreak);
  signal (SIGHUP, reload_config);

  return ut;
}

int
urobot_run (struct urobot_t *ut)
{
  if (init_upnp (ut) < 0)
  {
    urobot_stop(ut);
    return EXIT_FAILURE;
  }

  /* Let main sleep until it's time to die... */
  pthread_mutex_lock (&ut->termination_mutex);
  pthread_cond_wait (&ut->termination_cond, &ut->termination_mutex);
  pthread_mutex_unlock (&ut->termination_mutex);

  return EXIT_SUCCESS;
}

void
urobot_stop(struct urobot_t *ut)
{
  finish_upnp (ut);
  urobot_free (ut);
}

#ifdef TEST
int
main (int argc, char **argv)
{
  struct urobot_t *ut;

  ut = urobot_init(argc, argv);

  if (urobot_run(ut) != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }

  urobot_stop(ut);

  /* it should never be executed */
  return EXIT_SUCCESS;
}
#endif
