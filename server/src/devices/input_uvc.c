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
#include <linux/videodev2.h>
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

#include "urobot.h"
#include "device.h"
#include "utils.h"
#include "devices/cambuf.h"
#include "devices/v4l2uvc.h" // this header will includes the ../../mjpg_streamer.h
#include "devices/huffman.h"
#include "devices/jpeg_utils.h"
//#include "uvcvideo.h"
#include "cam.h"

#define INPUT_PLUGIN_NAME "UVC webcam grabber"

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
static struct urobot_t *ut;
static int gquality = 80;
static unsigned int minimum_size = 0;

context cams[MAX_DEVICE_PLUGINS];

void *cam_thread(void *);
void cam_cleanup(void *);
static void help(void);

int convert_format2v4l2(int format)
{
    int v4l2Format=V4L2_PIX_FMT_MJPEG;

    switch(format) {
    case PIXEL_FORMAT_RGB:
        v4l2Format=V4L2_PIX_FMT_RGB32;
        break;
    case PIXEL_FORMAT_JPG:
        v4l2Format=V4L2_PIX_FMT_JPEG;
        break;
    case PIXEL_FORMAT_MJPG:
        v4l2Format=V4L2_PIX_FMT_MJPEG;
        break;
    case PIXEL_FORMAT_YUV:
        v4l2Format=V4L2_PIX_FMT_YUYV;
        break;
    case PIXEL_FORMAT_H264:
        v4l2Format=V4L2_PIX_FMT_H264;
        break;
    case PIXEL_FORMAT_MP4:
        v4l2Format=V4L2_PIX_FMT_MPEG4;
        break;
    }
    return v4l2Format;
}

int convert_format2urobot(int v4l2Format)
{
    int format=PIXEL_FORMAT_MJPG;

    switch(v4l2Format) {
    case V4L2_PIX_FMT_RGB32:
        format = PIXEL_FORMAT_RGB;
        break;
    case V4L2_PIX_FMT_JPEG:
        format =  PIXEL_FORMAT_JPG;
        break;
    case V4L2_PIX_FMT_MJPEG:
        format =  PIXEL_FORMAT_MJPG;
        break;
    case V4L2_PIX_FMT_YUYV:
        format = PIXEL_FORMAT_YUV;
        break;
    case V4L2_PIX_FMT_H264:
        format = PIXEL_FORMAT_H264;
        break;
    case V4L2_PIX_FMT_MPEG4:
        format = PIXEL_FORMAT_MP4;

    }
    return format;
}


/*** device interface functions ***/
/******************************************************************************
Description.: This function ializes the plugin. It parses the commandline-
              parameter and stores the default and parsed values in the
              appropriate variables.
Input Value.: param contains among others the command-line string
Return Value: 0 if everything is fine
              1 if "--help" was triggered, in this case the calling programm
              should stop running and leave.
******************************************************************************/
int uvc_init(device_argument *args, int id)
{
    char *dev = "/dev/video0", *s;
    int width = 640, height = 480, fps = 30, i;
    int format = V4L2_PIX_FMT_MJPEG;

    /* initialize the mutes variable */
    if(pthread_mutex_init(&cams[id].controls_mutex, NULL) != 0) {
        DLOG("could not initialize mutex variable\n");
        exit(EXIT_FAILURE);
    }

    args->argv[0] = INPUT_PLUGIN_NAME;

    /* show all parameters for DBG purposes */
    for(i = 0; i < args->argc; i++) {
        DBG("argv[%d]=%s\n", i, args->argv[i]);
    }

    /* parse the parameters */
    reset_getopt();
    while(1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"h", no_argument, 0, 0
            },
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
            {"led", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        /* parsing all parameters according to the list above is sufficent */
        c = getopt_long_only(args->argc, args->argv, "", long_options, &option_index);

        /* no more options to parse */
        if(c == -1) break;

        /* unrecognized option */
        if(c == '?') {
            help();
            return 1;
        }

        /* dispatch the given options */
        switch(option_index) {
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
            for(i = 0; i < LENGTH_OF(resolutions); i++) {
                if(strcmp(resolutions[i].string, optarg) == 0) {
                    width  = resolutions[i].width;
                    height = resolutions[i].height;
                }
            }
            /* done if width and height were set */
            if(width != -1 && height != -1)
                break;
            /* parse value as decimal value */
            width  = strtol(optarg, &s, 10);
            height = strtol(s + 1, NULL, 10);
            break;

            /* f, fps */
        case 6:
        case 7:
            DBG("case 6,7\n");
            fps = atoi(optarg);
            break;

            /* y, yuv */
        case 8:
        case 9:
            DBG("case 8,9\n");
            format = V4L2_PIX_FMT_YUYV;
            break;

            /* q, quality */
        case 10:
        case 11:
            DBG("case 10,11\n");
            format = V4L2_PIX_FMT_YUYV;
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
    DBG("input id: %d\n", id);
    cams[id].id = id;
    cams[id].ut = args->ut;

    /* allocate webcam datastructure */
    cams[id].videoIn = malloc(sizeof(struct vdIn));
    if(cams[id].videoIn == NULL) {
        DLOG("not enough memory for videoIn\n");
        exit(EXIT_FAILURE);
    }
    memset(cams[id].videoIn, 0, sizeof(struct vdIn));

    /* display the parsed values */
    DLOG("Using V4L2 device.: %s\n", dev);
    DLOG("Desired Resolution: %i x %i\n", width, height);
    DLOG("Frames Per Second.: %i\n", fps);
    DLOG("Format............: %s\n", (format == V4L2_PIX_FMT_YUYV) ? "YUV" : "MJPEG");
    if(format == V4L2_PIX_FMT_YUYV)
        DLOG("JPEG Quality......: %d\n", gquality);

    DBG("vdIn pn: %d\n", id);
    /* open video device and prepare data structure */
    if(init_videoIn(cams[id].videoIn, dev, width, height, fps, format, 1, cams[id].ut, args->dev) < 0) {
        DLOG("init_VideoIn failed\n");
        closelog();
        exit(EXIT_FAILURE);
    }

    enumerateControls(cams[id].videoIn, cams[id].ut, args->dev); // enumerate V4L2 controls after UVC extended mapping

    cameraBuffer *cambuf = (cameraBuffer *) calloc(1, sizeof(cameraBuffer));
    cambuf->width = cams[id].videoIn->width;
    cambuf->height = cams[id].videoIn->height;
    cambuf->fps = cams[id].videoIn->fps;
    cambuf->formatIn = convert_format2urobot(cams[id].videoIn->formatIn);
    cambuf->framebuffer = cams[id].videoIn->framebuffer;
    cams[id].cambuf = cambuf;

    return 0;
}

/******************************************************************************
Description.: Stops the execution of worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int uvc_stop(int id)
{
    DBG("will cancel camera thread #%02d\n", id);
    pthread_cancel(cams[id].threadID);
    return 0;
}

/******************************************************************************
Description.: spins of a worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
int uvc_run(int id)
{
    cams[id].ut->comm[id]->buf = malloc(cams[id].videoIn->framesizeIn);
    if(cams[id].ut->comm[id]->buf == NULL) {
        fprintf(stderr, "could not allocate memory\n");
        exit(EXIT_FAILURE);
    }

    DBG("launching camera thread #%02d\n", id);
    /* create thread and pass context to thread function */
    pthread_create(&(cams[id].threadID), NULL, cam_thread, &(cams[id]));
    pthread_detach(cams[id].threadID);
    return 0;
}

/*** private functions for this plugin below ***/
/******************************************************************************
Description.: print a help message to stderr
Input Value.: -
Return Value: -
******************************************************************************/
static void help(void)
{
    int i;

    fprintf(stderr, " ---------------------------------------------------------------\n" \
    " Help for input plugin..: "INPUT_PLUGIN_NAME"\n" \
    " ---------------------------------------------------------------\n" \
    " The following parameters can be passed to this plugin:\n\n" \
    " [-d | --device ].......: video device to open (your camera)\n" \
    " [-r | --resolution ]...: the resolution of the video device,\n" \
    "                          can be one of the following strings:\n" \
    "                          ");

    for(i = 0; i < LENGTH_OF(resolutions); i++) {
        fprintf(stderr, "%s ", resolutions[i].string);
        if((i + 1) % 6 == 0)
            fprintf(stderr, "\n                          ");
    }
    fprintf(stderr, "\n                          or a custom value like the following" \
    "\n                          example: 640x480\n");

    fprintf(stderr, " [-f | --fps ]..........: frames per second\n" \
    " [-y | --yuv ]..........: enable YUYV format and disable MJPEG mode\n" \
    " [-q | --quality ]......: JPEG compression quality in percent \n" \
    "                          (activates YUYV format, disables MJPEG)\n" \
    " [-m | --minimum_size ].: drop frames smaller then this limit, useful\n" \
    "                          if the webcam produces small-sized garbage frames\n" \
    "                          may happen under low light conditions\n" \
    " [-l | --led ]..........: switch the LED \"on\", \"off\", let it \"blink\" or leave\n" \
    "                          it up to the driver using the value \"auto\"\n" \
    " ---------------------------------------------------------------\n\n");
}

/******************************************************************************
Description.: this thread worker grabs a frame and copies it to the global buffer
Input Value.: unused
Return Value: unused, always NULL
******************************************************************************/
void *cam_thread(void *arg)
{

    context *pcontext = arg;
    ut = pcontext->ut;

    /* set cleanup handler to cleanup allocated ressources */
    pthread_cleanup_push(cam_cleanup, pcontext);

    while(!ut->stop) {
        while(pcontext->videoIn->streamingState == STREAMING_PAUSED) {
            usleep(1); // maybe not the best way so FIXME
        }

        /* grab a frame */
        if(uvcGrab(pcontext->videoIn) < 0) {
            DLOG("Error grabbing frames\n");
            exit(EXIT_FAILURE);
        }

        DBG("received frame of size: %d from plugin: %d\n", pcontext->videoIn->buf.bytesused, pcontext->id);

        /*
         * Workaround for broken, corrupted frames:
         * Under low light conditions corrupted frames may get captured.
         * The good thing is such frames are quite small compared to the regular pictures.
         * For example a VGA (640x480) webcam picture is normally >= 8kByte large,
         * corrupted frames are smaller.
         */
        if(pcontext->videoIn->buf.bytesused < minimum_size) {
            DBG("dropping too small frame, assuming it as broken\n");
            continue;
        }

        /* copy JPG picture to global buffer */
        pthread_mutex_lock(&ut->comm[pcontext->id]->mutex);

        /*
         * If capturing in YUV mode convert to JPEG now.
         * This compression requires many CPU cycles, so try to avoid YUV format.
         * Getting JPEGs straight from the webcam, is one of the major advantages of
         * Linux-UVC compatible devices.
         */
        if(pcontext->videoIn->formatIn == V4L2_PIX_FMT_YUYV) {
            DBG("compressing frame from input: %d\n", (int)pcontext->id);
            ut->comm[pcontext->id]->size = compress_yuyv_to_jpeg(pcontext->cambuf, ut->comm[pcontext->id]->buf, pcontext->videoIn->framesizeIn, gquality);
        } else {
            DBG("copying frame from input: %d\n", (int)pcontext->id);
            ut->comm[pcontext->id]->size = memcpy_picture(ut->comm[pcontext->id]->buf, pcontext->videoIn->tmpbuffer, pcontext->videoIn->buf.bytesused);
        }

#if 0
        /* motion detection can be done just by comparing the picture size, but it is not very accurate!! */
        if((prev_size - global->size)*(prev_size - global->size) > 4 * 1024 * 1024) {
            DBG("motion detected (delta: %d kB)\n", (prev_size - global->size) / 1024);
        }
        prev_size = global->size;
#endif

        /* copy this frame's timestamp to user space */
        ut->comm[pcontext->id]->timestamp = pcontext->videoIn->buf.timestamp;

        /* signal fresh_frame */
        pthread_cond_broadcast(&ut->comm[pcontext->id]->cond);
        pthread_mutex_unlock(&ut->comm[pcontext->id]->mutex);


        /* only use usleep if the fps is below 5, otherwise the overhead is too long */
        if(pcontext->videoIn->fps < 5) {
            DBG("waiting for next frame for %d us\n", 1000 * 1000 / pcontext->videoIn->fps);
            usleep(1000 * 1000 / pcontext->videoIn->fps);
        } else {
            DBG("waiting for next frame\n");
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
    static unsigned char first_run = 1;
    context *pcontext = arg;
    ut = pcontext->ut;
    if(!first_run) {
        DBG("already cleaned up ressources\n");
        return;
    }

    first_run = 0;
    DLOG("cleaning up ressources allocated by input thread\n");

    close_v4l2(pcontext->videoIn);
    if(pcontext->videoIn->tmpbuffer != NULL) free(pcontext->videoIn->tmpbuffer);
    if(pcontext->videoIn != NULL) free(pcontext->videoIn);
    if(pcontext->cambuf != NULL) free(pcontext->cambuf);
    if(ut->comm[pcontext->id]->buf != NULL)
        free(ut->comm[pcontext->id]->buf);
}

/******************************************************************************
Description.: process commands, allows to set v4l2 controls
Input Value.: * control specifies the selected v4l2 control's id
                see struct v4l2_queryctr in the videodev2.h
              * value is used for control that make use of a parameter.
Return Value: depends in the command, for most cases 0 means no errors and
              -1 signals an error. This is just rule of thumb, not more!
******************************************************************************/
int uvc_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
{
    int ret = -1;
    int i = 0;
    int index;
    
    device *dev= (device *) service->dev;
    v4l2priv *priv = (v4l2priv *) dev->priv_data;

    index = input[0].i;

    DBG("Requested cmd (id: %d) for the uvc device.\n", command_id);
    switch(command_id) {
    case GET_ENCODING:
    case SET_ENCODING: {
        } break;
    case GET_RESOLUTION:
    case SET_RESOLUTION: {
        int value = input[1].i;
        // the value points to the current formats nth resolution
        if(value > (priv->in_formats[priv->currentFormat].resolutionCount - 1)) {
            DBG("The value is out of range");
            return -1;
        }
        int height = priv->in_formats[priv->currentFormat].supportedResolutions[value].height;
        int width = priv->in_formats[priv->currentFormat].supportedResolutions[value].width;
        ret = setResolution(cams[index].videoIn, width, height);
        if(ret == 0) {
            priv->in_formats[priv->currentFormat].currentResolution = value;
        }
        return ret;
        } break;
    case GET_FRAMERATE:
    case SET_FRAMERATE: {
    //case IN_CMD_V4L2: {
        int value = input[1].i;
            ret = v4l2SetControl(cams[index].videoIn, command_id, value, ut, dev);
            if(ret == 0) {
                dev->parameters[i].v4l2ctrl.value = value;
            } else {
                DBG("v4l2SetControl failed: %d\n", ret);
            }
            return ret;
        } break;
    /*
    case IN_CMD_JPEG_QUALITY:
        if((value >= 0) && (value < 101)) {
            priv->jpegcomp.quality = value;
            if(IOCTL_VIDEO(cams[index].videoIn->fd, VIDIOC_S_JPEGCOMP, &priv->jpegcomp) != EINVAL) {
                DBG("JPEG quality is set to %d\n", value);
                ret = 0;
            } else {
                DBG("Setting the JPEG quality is not supported\n");
            }
        } else {
            DBG("Quality is out of range\n");
        }
        break;
    */
    }
    return ret;
}

