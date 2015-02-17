#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "urobot.h"
#include "device.h"
#include "services.h"
#include "target.h"

struct device_t *
urobot_device_add (struct urobot_t *ut, int type, char *name, int (*init)(), int (*stop)(), int (*run)(), int (*cmd)())
{
  if (!ut)
    return NULL;

  struct device_t *dev = (struct device_t *) malloc (sizeof (struct device_t));
  if (!dev)
    return NULL;

  struct device_common_t *comm = (struct device_common_t *) malloc (sizeof (struct device_common_t));
  if (!comm) {
    free(dev);
    return NULL;
  }
  ut->dev[ut->devcnt] = dev;
  ut->comm[ut->devcnt] = comm;
  dev->type = type;
  dev->name = strdup (name);
  dev->udn = NULL;
  dev->presentation = NULL;
  dev->servicecount = 0;

  dev->init = init;
  dev->stop = stop;
  dev->run = run;
  dev->cmd = cmd;

  ut->devcnt++;

  return dev;
}

int
urobot_device_delete (struct urobot_t *ut, struct device_t *dev)
{
  int i, j;

  if (!ut)
    return 0;

  for (i=0; i < ut->devcnt; i++) {
    if (dev == ut->dev[i]) {
      free(dev);
      free(ut->comm[i]);
      for (j=i; j < ut->devcnt-1; j++) {
        ut->dev[j] = ut->dev[j+1];
        ut->comm[j] = ut->comm[j+1];
      }
      ut->devcnt--;
      return 1;
    }
  }
  return 0;
}


int split_parameters(char *parameter_string, int *argc, char **argv)
{
    int count = 1;
    argv[0] = NULL; // the plugin may set it to 'INPUT_PLUGIN_NAME'
    if(parameter_string != NULL && strlen(parameter_string) != 0) {
        char *arg = NULL, *saveptr = NULL, *token = NULL;

        arg = strdup(parameter_string);

        if(strchr(arg, ' ') != NULL) {
            token = strtok_r(arg, " ", &saveptr);
            if(token != NULL) {
                argv[count] = strdup(token);
                count++;
                while((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
                    argv[count] = strdup(token);
                    count++;
                    if(count >= MAX_DEVICE_ARGUMENTS) {
                        DLOG("ERROR: too many arguments to input plugin\n");
                        return 0;
                    }
                }
            }
        }
    }
    *argc = count;
    return 1;
}

void init_devices(struct urobot_t *ut, int argc, char *argv[])
{
  int i, j;

  for (i=0; i<ut->devcnt; i++) {
    struct device_t *dev;
    device_argument *args;

    if (pthread_mutex_init(&ut->comm[i]->mutex, NULL) != 0) {
        DLOG("could not initialize mutex variable\n");
        closelog();
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&ut->comm[i]->cond, NULL) != 0) {
        DLOG("could not initialize condition variable\n");
        closelog();
        exit(EXIT_FAILURE);
    }

    dev = ut->dev[i];
    args = &dev->args;

    args->arguments = strchr(argv[i], ' ');
    split_parameters(args->arguments, &args->argc, args->argv);
    args->dev = dev;
    args->ut = ut;
    args->id = 0;
    if ((dev->init)(args, 0)) {
        DLOG("device_init() return value signals to exit\n");
        closelog();
        exit(0);
    }

    DBG("starting device\n");
    if ((dev->run)(0)) {
        DLOG("can not run device\n");
        closelog();
        exit(0);
    }

    DBG("starting service\n");
    for (j=0; j<dev->servicecount; j++) {
      if (dev->service[j]->init) {
        printf("j=%d\n",j);
        if ((dev->service[j]->init)(dev->service[j],j)) {
            DLOG("service_init() return value signals to exit\n");
            closelog();
            exit(0);
        }
      }
    }
  }
}

void finish_devices(struct urobot_t *ut, int argc, char *argv[])
{
  int i;

  ut->stop = 1;
  for (i=0; i<ut->devcnt; i++) {
    usleep(1000);

    ut->dev[i]->stop(ut->dev[i]->args.id);
    pthread_cond_destroy(&ut->comm[i]->cond);
    pthread_mutex_destroy(&ut->comm[i]->mutex);
  }

  return;
}

