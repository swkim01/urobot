#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <limits.h>

#include "config.h"
#include "gettext.h"
#include "cfgparser.h"
#include "urobot.h"
#include "trace.h"
#include "osdep.h"

static bool
ignore_line (const char *line)
{
  int i;
  size_t len;

  /* commented line */
  if (line[0] == '#' )
    return true;

  len = strlen (line);

  for (i = 0 ; i < (int) len ; i++ )
    if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
      return false;

  return true;
}

static char *
strdup_trim (const char *s)
{
  size_t begin, end;
  char *r = NULL;

  if (!s)
    return NULL;

  end = strlen (s) - 1;

  for (begin = 0 ; begin < end ; begin++)
    if (s[begin] != ' ' && s[begin] != '\t' && s[begin] != '"')
      break;

  for (; begin < end ; end--)
    if (s[end] != ' ' && s[end] != '\t' && s[end] != '"' && s[end] != '\n')
      break;

  r = strndup (s + begin, end - begin + 1);

  return r;
}

static void
urobot_set_name (struct urobot_t *ut, const char *name)
{
  if (!ut || !name)
    return;

  if (ut->name)
  {
    free (ut->name);
    ut->name = NULL;
  }

  ut->name = strdup_trim (name);
}

static void
urobot_set_interface (struct urobot_t *ut, const char *iface)
{
  if (!ut || !iface)
    return;

  if (ut->interface)
  {
    free (ut->interface);
    ut->interface = NULL;
  }

  ut->interface = strdup_trim (iface);
}

static void
urobot_set_cfg_file (struct urobot_t *ut, const char *file)
{
  if (!ut || !file)
    return;

  ut->cfg_file = strdup (file);
}

static void
urobot_set_pid_file (struct urobot_t *ut, const char *file)
{
  if (!ut || !file)
    return;

  ut->pid_file = strdup (file);
}

static void
urobot_set_port (struct urobot_t *ut, const char *port)
{
  if (!ut || !port)
    return;

  ut->port = atoi (port);
  if (ut->port < 49152)
  {
    fprintf (stderr,
             _("Warning: port doesn't fit IANA port assignements.\n"));

    fprintf (stderr, _("Warning: Only Dynamic or Private Ports can be used "
                       "(from 49152 through 65535)\n"));
    ut->port = 0;
  }
}

static void
urobot_use_web (struct urobot_t *ut, const char *val)
{
  if (!ut || !val)
    return;

  ut->use_presentation = (!strcmp (val, "yes")) ? true : false;
}

static u_configline_t configline[] = {
  { UROBOT_NAME,                 urobot_set_name                },
  { UROBOT_IFACE,                urobot_set_interface           },
  { UROBOT_PORT,                 urobot_set_port                },
  { UROBOT_ENABLE_WEB,           urobot_use_web                 },
  { UROBOT_PID_FILE, 	         urobot_set_pid_file 		},
  { NULL,                        NULL                           },
};

static void
parse_config_line (struct urobot_t *ut, const char *line,
                   u_configline_t *configline)
{
  char *s = NULL;

  s = strchr (line, '=');
  if (s && s[1] != '\0')
  {
    int i;
    for (i=0 ; configline[i].name ; i++)
    {
      if (!strncmp (line, configline[i].name, strlen (configline[i].name)))
      {
        configline[i].set_var (ut, s + 1);
        break;
      }
    }
  }
}

int
parse_config_file (struct urobot_t *ut)
{
  char filename[PATH_MAX];
  FILE *conffile;
  char *line = NULL;
  size_t size = 0;
  ssize_t read;

  if (!ut)
    return -1;

  if (!ut->cfg_file)
    snprintf (filename, PATH_MAX, "%s/%s", SYSCONFDIR, UROBOT_CONFIG_FILE); 
  else
    snprintf (filename, PATH_MAX, "%s", ut->cfg_file);

  conffile = fopen (filename, "r");
  if (!conffile)
    return -1;

  while ((read = getline (&line, &size, conffile)) != -1)
  {
    if (ignore_line (line))
      continue;

    if (line[read-1] == '\n')
      line[read-1] = '\0';

    while (line[0] == ' ' || line[0] == '\t')
      line++;

    parse_config_line (ut, line, configline);
  }

  fclose (conffile);

  if (line)
    free (line);

  return 0;
}

inline static void
display_usage (void)
{
  display_headers ();
  printf ("\n");
  printf (_("Usage: urobot [-n name] [-i interface] [-p port] [-f cfg_file] [-w] [-D]\n"));
  printf (_("Options:\n"));
  printf (_(" -n, --name=NAME\tSet UPnP Friendly Name (default is '%s')\n"),
          DEFAULT_UROBOT_NAME);
  printf (_(" -i, --interface=IFACE\tUse IFACE Network Interface (default is '%s')\n"),
          DEFAULT_UROBOT_IFACE);
  printf (_(" -f, --cfg=FILE\t\tConfig file to be used\n"));
  printf (_(" -p, --port=PORT\tForces the HTTP server to run on PORT\n"));
  printf (_(" -w, --no-web\t\tDisable the control web page (enabled by default)\n"));
  printf (_(" -v, --verbose\t\tSet verbose display\n"));
  printf (_(" -D, --daemon\t\tRun as a daemon\n"));
  printf (_(" -V, --version\t\tDisplay the version of uRobot and exit\n"));
  printf (_(" -h, --help\t\tDisplay this help\n"));
}

static inline void reset_getopt(void)
{
    /* optind=1; opterr=1; optopt=63; */
#ifdef __GLIBC__
  optind = 0;
#else
  optind = 1;
#endif

#ifdef HAVE_OPTRESET
  optreset = 1;
#endif
}

int
parse_command_line (struct urobot_t *ut, int argc, char **argv)
{
  int c, index;
  char short_options[] = "VhvDwdn:i:p:f:";
  struct option long_options [] = {
    {"version", no_argument, 0, 'V' },
    {"help", no_argument, 0, 'h' },
    {"verbose", no_argument, 0, 'v' },
    {"daemon", no_argument, 0, 'D' },
    {"name", required_argument, 0, 'n' },
    {"interface", required_argument, 0, 'i' },
    {"port", required_argument, 0, 'p' },
    {"no-web", no_argument, 0, 'w' },
    {"cfg", required_argument, 0, 'f' },
    {0, 0, 0, 0 }
  };

  /* command line argument processing */
  reset_getopt();
  while (true)
  {
    c = getopt_long (argc, argv, short_options, long_options, &index);

    if (c == EOF)
      break;

    switch (c)
    {
    case 0:
      /* opt = long_options[index].name; */
      break;

    case '?':
    case 'h':
      display_usage ();
      return -1;

    case 'V':
      display_headers ();
      return -1;

    case 'v':
      ut->verbose = true;
      break;

    case 'D':
      ut->daemon = true;
      break;

    case 'n':
      urobot_set_name (ut, optarg);
      break;

    case 'i':
      urobot_set_interface (ut, optarg);
      break;

    case 'p':
      urobot_set_port (ut, optarg);
      break;

    case 'w':
      ut->use_presentation = false;
      break;

    case 'f':
      urobot_set_cfg_file (ut, optarg);
      break;

    default:
      break;
    }
  }

  return 0;
}
