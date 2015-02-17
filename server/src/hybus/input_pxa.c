/*******************************************************************************
# Linux-UVC streaming input-plugin for MJPG-streamer                           #
#                                                                              #
# This package work with the Logitech UVC based webcams with the mjpeg feature #
#                                                                              #
# Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard                   #
#                    2007 Lucas van Staden                                     #
#                    2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <syslog.h>

#include "../utils.h"
//#include "mjpg_streamer.h"
#include "v4l2pxa.h"
#include "jpeg_utils.h"

#define INPUT_PLUGIN_NAME "PXA webcam grabber"
#define MAX_ARGUMENTS 32

static char *input_plugin=INPUT_PLUGIN_NAME;

/*
 * UVC resolutions mentioned at: (at least for some webcams)
 * http://www.quickcamteam.net/hcl/frame-format-matrix/
 */
static const struct {
  const char *string;
  const int width, height;
} resolutions[] = {
  { "QSIF", 160,  120  },
  { "QCIF", 176,  144  },
  { "CGA",  320,  200  },
  { "QVGA", 320,  240  },
  { "CIF",  352,  288  },
  { "VGA",  640,  480  },
  { "SVGA", 800,  600  },
  { "XGA",  1024, 768  },
  { "SXGA", 1280, 1024 }
};

/* private functions and variables to this plugin */
static globals *pglobal;
static int gquality = 80;
static unsigned int minimum_size = 0;

context cams[MAX_INPUT_PLUGINS];

void *cam_thread( void *);
void cam_cleanup(void *);
static void help(void);

/*** plugin interface functions ***/
/******************************************************************************
Description.: This function initializes the plugin. It parses the commandline-
              parameter and stores the default and parsed values in the
              appropriate variables.
Input Value.: param contains among others the command-line string
Return Value: 0 if everything is fine
              1 if "--help" was triggered, in this case the calling programm
              should stop running and leave.
******************************************************************************/
int input_init(input_parameter *param, int id) {
  char *dev = "/dev/video0", *s;
  int width=320, height=240, fps=5, i;
  int format=V4L2_PIX_FMT_YUV422P;

  /* initialize the mutes variable */
  if( pthread_mutex_init(&cams[id].controls_mutex, NULL) != 0 ) {
    IPRINT("could not initialize mutex variable\n");
    exit(EXIT_FAILURE);
  }

  //param->argv[0] = INPUT_PLUGIN_NAME;
  param->argv[0] = input_plugin;

  /* show all parameters for DBG purposes */
  for (i=0; i<param->argc; i++) {
    DBG("argv[%d]=%s\n", i, param->argv[i]);
  }

  /* parse the parameters */
  reset_getopt();
  while(1) {
    int option_index = 0, c=0;
    static struct option long_options[] = \
    {
      {"h", no_argument, 0, 0},
      {"help", no_argument, 0, 0},
      {"d", required_argument, 0, 0},
      {"device", required_argument, 0, 0},
      {"r", required_argument, 0, 0},
      {"resolution", required_argument, 0, 0},
      {"f", required_argument, 0, 0},
      {"fps", required_argument, 0, 0},
      {"y", no_argument, 0, 0},
      {"yuv", no_argument, 0, 0},
      {"q", required_argument, 0, 0},
      {"quality", required_argument, 0, 0},
      {"m", required_argument, 0, 0},
      {"minimum_size", required_argument, 0, 0},
      {"n", no_argument, 0, 0},
      {"l", required_argument, 0, 0},
      {0, 0, 0, 0}
    };

    /* parsing all parameters according to the list above is sufficent */
    c = getopt_long_only(param->argc, param->argv, "", long_options, &option_index);

    /* no more options to parse */
    if (c == -1) break;

    /* unrecognized option */
    if (c == '?'){
      help();
      return 1;
    }

    /* dispatch the given options */
    switch (option_index) {
      /* h, help */
      case 0:
      case 1:
        DBG("case 0,1\n");
        help();
        return 1;
        break;

      /* d, device */
      case 2:
      case 3:
        DBG("case 2,3\n");
        dev = strdup(optarg);
        break;

      /* r, resolution */
      case 4:
      case 5:
        DBG("case 4,5\n");
        width = -1;
        height = -1;

        /* try to find the resolution in lookup table "resolutions" */
        for ( i=0; i < LENGTH_OF(resolutions); i++ ) {
          if ( strcmp(resolutions[i].string, optarg) == 0 ) {
            width  = resolutions[i].width;
            height = resolutions[i].height;
          }
        }
        /* done if width and height were set */
        if(width != -1 && height != -1)
          break;
        /* parse value as decimal value */
        width  = strtol(optarg, &s, 10);
        height = strtol(s+1, NULL, 10);
        break;

      /* f, fps */
      case 6:
      case 7:
        DBG("case 6,7\n");
        fps=atoi(optarg);
        break;

      /* y, yuv */
      case 8:
      case 9:
        DBG("case 8,9\n");
        format = V4L2_PIX_FMT_YUV422P;
        break;

      /* q, quality */
      case 10:
      case 11:
        DBG("case 10,11\n");
        format = V4L2_PIX_FMT_YUV422P;
        gquality = MIN(MAX(atoi(optarg), 0), 100);
        break;

      /* m, minimum_size */
      case 12:
      case 13:
        DBG("case 12,13\n");
        minimum_size = MAX(atoi(optarg), 0);
        break;

      default:
        DBG("default case\n");
        help();
        return 1;
    }
  }

  /* keep a pointer to the global variables */
  cams[id].id = id;
  cams[id].pglobal = param->global;

  /* allocate webcam datastructure */
  cams[id].videoIn = malloc(sizeof(struct vdIn));
  if (cams[id].videoIn == NULL) {
    IPRINT("not enough memory for videoIn\n");
    exit(EXIT_FAILURE);
  }
  memset(cams[id].videoIn, 0, sizeof(struct vdIn));

  /* display the parsed values */
  IPRINT("Using V4L2 device.: %s\n", dev);
  IPRINT("Desired Resolution: %i x %i\n", width, height);
  IPRINT("Frames Per Second.: %i\n", fps);
  IPRINT("Format............: %s\n", (format==V4L2_PIX_FMT_YUV422P)?"YUV":"MJPEG");
  if ( format == V4L2_PIX_FMT_YUV422P )
    IPRINT("JPEG Quality......: %d\n", gquality);

  /* open video device and prepare data structure */
  if (init_videoIn(cams[id].videoIn, dev, width, height, fps, format, 1) < 0) {
    IPRINT("init_VideoIn failed\n");
    closelog();
    exit(EXIT_FAILURE);
  }

  return 0;
}

/******************************************************************************
Description.: Stops the execution of worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_stop(int id) {
  DBG("will cancel input thread\n");
  pthread_cancel(cams[id].threadID);

  return 0;
}

/******************************************************************************
Description.: spins of a worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int input_run(int id) {
  cams[id].pglobal->in[id].buf = malloc(cams[id].videoIn->framesizeIn);
  if (cams[id].pglobal->in[id].buf == NULL) {
    fprintf(stderr, "could not allocate memory\n");
    exit(EXIT_FAILURE);
  }

  pthread_create(&(cams[id].threadID), 0, cam_thread, &(cams[id]));
  pthread_detach(cams[id].threadID);

  return 0;
}

/*** private functions for this plugin below ***/
/******************************************************************************
Description.: print a help message to stderr
Input Value.: -
Return Value: -
******************************************************************************/
static void help(void) {
  int i;

  fprintf(stderr, " ---------------------------------------------------------------\n" \
                  " Help for input plugin..: "INPUT_PLUGIN_NAME"\n" \
                  " ---------------------------------------------------------------\n" \
                  " The following parameters can be passed to this plugin:\n\n" \
                  " [-d | --device ].......: video device to open (your camera)\n" \
                  " [-r | --resolution ]...: the resolution of the video device,\n" \
                  "                          can be one of the following strings:\n" \
                  "                          ");

  for ( i=0; i < LENGTH_OF(resolutions); i++ ) {
    fprintf(stderr, "%s ", resolutions[i].string);
    if ( (i+1)%6 == 0)
      fprintf(stderr, "\n                          ");
  }
  fprintf(stderr, "\n                          or a custom value like the following" \
                  "\n                          example: 640x480\n");

  fprintf(stderr, " [-f | --fps ]..........: frames per second\n" \
                  " [-y | --yuv ]..........: enable YUV422 format and disable MJPEG mode\n" \
                  " [-q | --quality ]......: JPEG compression quality in percent \n" \
                  "                          (activates YUV422 format, disables MJPEG)\n" \
                  " [-m | --minimum_size ].: drop frames smaller then this limit, useful\n" \
                  "                          if the webcam produces small-sized garbage frames\n" \
                  "                          may happen under low light conditions\n" \
                  " ---------------------------------------------------------------\n\n");
}

/******************************************************************************
Description.: this thread worker grabs a frame and copies it to the global buffer
Input Value.: unused
Return Value: unused, always NULL
******************************************************************************/
void *cam_thread( void *arg )
{

  context *pcontext = arg;
  pglobal = pcontext->pglobal;

  /* set cleanup handler to cleanup allocated ressources */
  pthread_cleanup_push(cam_cleanup, pcontext);

  //video_enable(videoIn);
  while( !pglobal->stop ) {
    /*
    while(pcontext->videoIn->streamingState == STREAMING_PAUSED) {
            usleep(1); // maybe not the best way so FIXME
    }
    */
    /* grab a frame */
    if( pxaGrab(pcontext->videoIn) < 0 ) {
      IPRINT("Error grabbing frames\n");
      exit(EXIT_FAILURE);
    }
  
    //DBG("received frame of size: %d\n", pcontext->videoIn->buf.bytesused);

    /*
     * Workaround for broken, corrupted frames:
     * Under low light conditions corrupted frames may get captured.
     * The good thing is such frames are quite small compared to the regular pictures.
     * For example a VGA (640x480) webcam picture is normally >= 8kByte large,
     * corrupted frames are smaller.
     */
    if (pcontext->videoIn->buf.bytesused < minimum_size) {
      DBG("dropping too small frame, assuming it as broken\n");
      continue;
    }

    /* copy JPG picture to global buffer */
    pthread_mutex_lock(&pglobal->in[pcontext->id].db);

    /*
     * If capturing in YUV mode convert to JPEG now.
     * This compression requires many CPU cycles, so try to avoid YUV format.
     * Getting JPEGs straight from the webcam, is one of the major advantages of
     * Linux-UVC compatible devices.
     */
    if (pcontext->videoIn->formatIn == V4L2_PIX_FMT_YUV422P) {
      //DBG("compressing frame\n");
      //pglobal->in[pcontext->id].size = compress_yuv422_to_jpeg(pcontext->videoIn, pglobal->in[pcontext->id].buf, pcontext->videoIn->framesizeIn, gquality);
      pglobal->in[pcontext->id].size = reverse_compress_yuv422_to_jpeg(pcontext->videoIn, pglobal->in[pcontext->id].buf, pcontext->videoIn->framesizeIn, gquality);
      //printf("vidbuf: y=0x%x, cb=0x%x, cr=0x%x\n", *((unsigned char *)videoIn->framebuffer), *((unsigned char *)videoIn->framebuffer + (int)(videoIn->width*videoIn->height)), *((unsigned char *)videoIn->framebuffer+(int)(videoIn->width*videoIn->height*1.5)));
    }
    /*
    else {
      DBG("copying frame\n");
      pglobal->in[pcontext->id].size = memcpy_picture(pglobal->in[pcontext->id].buf, pcontext->videoIn->tmpbuffer, pcontext->videoIn->buf.bytesused);
    }
    */

#if 0
    /* motion detection can be done just by comparing the picture size, but it is not very accurate!! */
    if ( (prev_size - global->size)*(prev_size - global->size) > 4*1024*1024 ) {
        DBG("motion detected (delta: %d kB)\n", (prev_size - global->size) / 1024);
    }
    prev_size = global->size;
#endif

    /* copy this frame's timestamp to user space */
    pglobal->in[pcontext->id].timestamp = pcontext->videoIn->buf.timestamp;

    /* signal fresh_frame */
    pthread_cond_broadcast(&pglobal->in[pcontext->id].db_update);
    pthread_mutex_unlock(&pglobal->in[pcontext->id].db);

    //DBG("waiting for next frame\n");

    /* only use usleep if the fps is below 5, otherwise the overhead is too long */
    if (pcontext->videoIn->fps < 5) {
      //usleep(1000*1000/pcontext->videoIn->fps);
      usleep(1000);
    }
  }

  DBG("leaving input thread, calling cleanup function now\n");
  pthread_cleanup_pop(1);

  return NULL;
}

/******************************************************************************
Description.: 
Input Value.: 
Return Value: 
******************************************************************************/
void cam_cleanup(void *arg)
{
  static unsigned char first_run=1;
  context *pcontext = arg;
  pglobal = pcontext->pglobal;
  if (!first_run) {
    DBG("already cleaned up ressources\n");
    return;
  }

  first_run = 0;
  IPRINT("cleaning up ressources allocated by input thread\n");

  close_v4l2(pcontext->videoIn);
  if (pcontext->videoIn->tmpbuffer != NULL) free(pcontext->videoIn->tmpbuffer);
  if (pcontext->videoIn != NULL) free(pcontext->videoIn);
  if (pglobal->in[pcontext->id].buf != NULL)
    free(pglobal->in[pcontext->id].buf);
}

