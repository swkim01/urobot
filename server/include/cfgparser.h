#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#include "urobot.h"

#define UROBOT_NAME               "UROBOT_NAME"
#define UROBOT_IFACE              "UROBOT_IFACE"
#define UROBOT_PORT               "UROBOT_PORT"
#define UROBOT_OVERRIDE_ICONV_ERR "UROBOT_OVERRIDE_ICONV_ERR"
#define UROBOT_ENABLE_WEB         "UROBOT_ENABLE_WEB"
#define UROBOT_PID_FILE		  "UROBOT_PID_FILE"

#define UROBOT_CONFIG_FILE        "urobot.conf"
#define DEFAULT_UROBOT_NAME       "uRobot"

#if (defined(BSD) || defined(__FreeBSD__))
#define DEFAULT_UROBOT_IFACE      "lnc0"
#else /* Linux */
#define DEFAULT_UROBOT_IFACE      "eth0"
#endif

int parse_config_file (struct urobot_t *ut)
    __attribute__ ((nonnull));
int parse_command_line (struct urobot_t *ut, int argc, char **argv)
    __attribute__ ((nonnull (1)));

typedef struct {
  char *name;
  void (*set_var) (struct urobot_t*, const char*);
} u_configline_t;

#endif /* _CONFIG_PARSER_H_ */
