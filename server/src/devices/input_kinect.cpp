/*******************************************************************************
# Turtlebot-Kinect streaming input-plugin for uRobot                           #
#                                                                              #
# This package work with the MS Kinect based robots with the mjpeg feature     #
#                                                                              #
# Copyright (C) 2012 Seong-Woo Kim                                             #
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
#include <XnVersion.h>
#include <XnCppWrapper.h>
#include <XnTypes.h>

#include "urobot.h"
#include "device.h"
#include "services.h"
#include "utils.h"
//#include "kobuki.h"
#include "devices/openni_kinect.h"
#include "devices/jpeg_utils.h"
#include "cam.h"

#define INPUT_PLUGIN_NAME "Kinect camera grabber"

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

using namespace xn;

//xn::Context g_context;
Context g_context;
#if XN_VERSION >= 103000000
//xn::ScriptNode g_scriptNode;
ScriptNode g_scriptNode;
#if XN_VERSION >= 105000000
Device g_device;
#endif
#endif
//xn::ImageGenerator g_image;
//xn::ImageMetaData g_imageMD;
//xn::EnumerationErrors errors;
ImageGenerator g_image;
ImageMetaData g_imageMD;
EnumerationErrors errors;

void *cam_thread(void *);
void cam_cleanup(void *);
static void help(void);


/*** plugin interface functions ***/
/******************************************************************************
Description.: This function ializes the plugin. It parses the commandline-
              parameter and stores the default and parsed values in the
              appropriate variables.
Input Value.: param contains among others the command-line string
Return Value: 0 if everything is fine
              1 if "--help" was triggered, in this case the calling programm
              should stop running and leave.
******************************************************************************/
extern "C" int kinect_init(device_argument *args, int id)
{
    const char *dev = "/dev/usb";
    char *s;
    const char *xmlfile = "SamplesConfig.xml";
    int width = 640, height = 480, fps = 30, i;
    XnPixelFormat format = XN_PIXEL_FORMAT_RGB24;
    XnStatus rc = XN_STATUS_OK;
    struct vdIn *vd;

    /* initialize the mutes variable */
    if(pthread_mutex_init(&cams[id].controls_mutex, NULL) != 0) {
        DLOG("could not initialize mutex variable\n");
        exit(EXIT_FAILURE);
    }

    args->argv[0] = (char *)INPUT_PLUGIN_NAME;

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
            format = XN_PIXEL_FORMAT_YUV422;
            break;

            /* q, quality */
        case 10:
        case 11:
            DBG("case 10,11\n");
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
    cams[id].videoIn = (struct vdIn *) malloc(sizeof(struct vdIn));
    if(cams[id].videoIn == NULL) {
        DLOG("not enough memory for videoIn\n");
        exit(EXIT_FAILURE);
    }
    memset(cams[id].videoIn, 0, sizeof(struct vdIn));

    /* display the parsed values */
    DLOG("Using Kinect device.: %s\n", dev);
    DLOG("Desired Resolution: %i x %i\n", width, height);
    DLOG("Frames Per Second.: %i\n", fps);
    DLOG("Format............: %s\n", (format == XN_PIXEL_FORMAT_YUV422) ? "YUV" : "RGB");
    DLOG("JPEG Quality......: %d\n", gquality);

    DBG("vdIn pn: %d\n", id);
    // Initialize context object
    //rc = g_context.Init();
#if XN_VERSION < 103000000
    rc = g_context.InitFromXmlFile(xmlfile, &errors);
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }
    // Create a ImageGenerator node
    //rc = g_image.Create(g_context);
    rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }
#elif XN_VERSION < 105000000
    rc = g_context.InitFromXmlFile(xmlfile, g_scriptNode, &errors);
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }
    rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }
#else
    rc = g_context.Init();
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }

    // find devices
    NodeInfoList list;
    rc = g_context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, list);
    if(rc != XN_STATUS_OK && list.Begin() != list.End()) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }
    else if(list.Begin() == list.End()) {
        fprintf(stderr, "No device found!\n");
        exit(EXIT_FAILURE);
    }
    for (NodeInfoList::Iterator it = list.Begin(); it != list.End(); ++it) {
        NodeInfo deviceNodeInfo = *it;

        Device deviceNode;
        deviceNodeInfo.GetInstance(deviceNode);
        XnBool bExists = deviceNode.IsValid();
        if (!bExists) {
            g_context.CreateProductionTree(deviceNodeInfo, deviceNode);
               // this might fail.
        }

        if (deviceNode.IsValid() && deviceNode.IsCapabilitySupported(XN_CAPABILITY_DEVICE_IDENTIFICATION)) {
            const XnUInt32 nStringBufferSize = 200;
            XnChar strDeviceName[nStringBufferSize];
            XnChar strSerialNumber[nStringBufferSize];

            XnUInt32 nLength = nStringBufferSize;
            deviceNode.GetIdentificationCap().GetDeviceName(strDeviceName, nLength);
            printf("[%d] %s (%s)\n", i, strDeviceName, strSerialNumber);
        }
        else {
            printf("[%d] %s\n", i, deviceNodeInfo.GetCreationInfo());
        }

        // release the device if we created it
        if (!bExists && deviceNode.IsValid()) {
            deviceNode.Release();
        }
    }

    NodeInfoList image_nodes;
    rc = g_context.EnumerateProductionTrees(XN_NODE_TYPE_IMAGE, NULL, image_nodes, NULL);
    if (rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }
    //NodeInfo image_node;
    for (NodeInfoList::Iterator it = image_nodes.Begin(); it != image_nodes.End(); ++it) {
        /* first node */
        NodeInfo image_node = *it;
        rc = image_node.GetInstance(g_image);
        if (rc != XN_STATUS_OK) {
            fprintf(stderr, "%s\n", xnGetStatusString(rc));
            exit(EXIT_FAILURE);
        }
        break;
    }
#endif

    // Set resolution
    XnMapOutputMode Mode;
    g_image.GetMapOutputMode(Mode);
    //Mode.nXRes = xn::Resolution(XN_RES_VGA).GetXResolution();
    //Mode.nYRes = xn::Resolution(XN_RES_VGA).GetYResolution();
    Mode.nXRes = Resolution(XN_RES_VGA).GetXResolution();
    Mode.nYRes = Resolution(XN_RES_VGA).GetYResolution();
    Mode.nFPS = 30;
    rc = g_image.SetMapOutputMode(Mode);

    vd = cams[id].videoIn;
    vd->pImageMD = &g_imageMD;
    vd->width = Mode.nXRes;
    vd->height = Mode.nYRes;
    vd->fps = Mode.nFPS;
    vd->framesizeIn = (vd->width * vd->height << 1);
    vd->formatIn = format;

    cameraBuffer *cambuf = (cameraBuffer *) calloc(1, sizeof(cameraBuffer));
    cambuf->width = vd->width;
    cambuf->height = vd->height;
    cambuf->fps = vd->fps;
    //cambuf->formatIn = vd->formatIn;
    cambuf->framesizeIn = vd->framesizeIn;
    //cambuf->framebuffer = vd->pImageMD;
    cams[id].cambuf = cambuf;

    // Make it start generating data
    rc = g_context.StartGeneratingAll();

    return 0;
}

/******************************************************************************
Description.: Stops the execution of worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
extern "C" int kinect_stop(int id)
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
extern "C" int kinect_run(int id)
{
    cams[id].ut->comm[id]->buf = (unsigned char *)malloc(cams[id].videoIn->framesizeIn);
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
    XnStatus rc = XN_STATUS_OK;

    context *pcontext = (context *)arg;
    ut = pcontext->ut;

    /* set cleanup handler to cleanup allocated ressources */
    pthread_cleanup_push(cam_cleanup, pcontext);

    while(!ut->stop) {
        /* grab a frame */

        // Wait for new data to be available
        rc = g_context.WaitOneUpdateAll(g_image);
        if (rc != XN_STATUS_OK)
        {
            DLOG("Failed updating data: %s\n", xnGetStatusString(rc));
            continue;
        }

        // Take current image map
        if (g_image.IsValid()) {
            g_image.GetMetaData(g_imageMD);
            DBG("Frame %d: YOffset=%d, XRes=%d, YRes=%d, PixelFormat=%s\n", pcontext->videoIn->pImageMD->FrameID(), pcontext->videoIn->pImageMD->YOffset(), pcontext->videoIn->pImageMD->XRes(), pcontext->videoIn->pImageMD->YRes(), pcontext->videoIn->pImageMD->PixelFormat() == XN_PIXEL_FORMAT_RGB24?"RGB":"YUV");
        }
        //pcontext->videoIn->pImageMap = g_image.GetYUV422ImageMap();
        pcontext->videoIn->pImageMap = pcontext->videoIn->pImageMD->Data();

        /* copy JPG picture to global buffer */
        pthread_mutex_lock(&ut->comm[pcontext->id]->mutex);

        /*
         * If capturing in YUV mode convert to JPEG now.
         */
        switch (pcontext->videoIn->pImageMD->PixelFormat()) {
        case XN_PIXEL_FORMAT_YUV422:
            ut->comm[pcontext->id]->size = compress_yuyv_to_jpeg(pcontext->cambuf, ut->comm[pcontext->id]->buf, pcontext->videoIn->framesizeIn, gquality);
            break;
        case XN_PIXEL_FORMAT_RGB24:
            ut->comm[pcontext->id]->size = compress_rgb_to_jpeg(pcontext->cambuf, ut->comm[pcontext->id]->buf, pcontext->videoIn->framesizeIn, gquality);
            break;
        }

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
    context *pcontext = (context *)arg;
    ut = pcontext->ut;
    if(!first_run) {
        DBG("already cleaned up ressources\n");
        return;
    }

    first_run = 0;
    DLOG("cleaning up ressources allocated by input thread\n");

    g_image.Release();
#if XN_VERSION < 103000000
    g_context.Shutdown();
#else
    g_scriptNode.Release();
    g_context.Release();
#endif
    if(pcontext->videoIn != NULL) free(pcontext->videoIn);
    if(ut->comm[pcontext->id]->buf != NULL)
        free(ut->comm[pcontext->id]->buf);
}

/******************************************************************************
Description.: spins of a worker thread
Input Value.: -
Return Value: always 0
******************************************************************************/
extern "C" int kinect_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
{
    int ret = -1;
    int id;
    struct vdIn *vd;
    cameraBuffer *cambuf;
    device *dev= (device *) service->dev;
    XnStatus rc = XN_STATUS_OK;

    id = input[0].i;

    DBG("Requested cmd (id: %d) for the uvc device.\n", command_id);
    switch(command_id) {
    case GET_ENCODING:
    case SET_ENCODING: {
        } break;
    case GET_RESOLUTION: {
        // Set resolution
        vd = cams[id].videoIn;
        output[0].i = vd->width;
        output[1].i = vd->height;
        } break;
    case SET_RESOLUTION: {
        int value = input[1].i;
        // the value points to the current formats nth resolution
        XnMapOutputMode Mode;
        g_image.GetMapOutputMode(Mode);
        Mode.nXRes = resolutions[value].width;
        Mode.nYRes = resolutions[value].height;
        rc = g_image.SetMapOutputMode(Mode);

        vd = cams[id].videoIn;
        vd->width = Mode.nXRes;
        vd->height = Mode.nYRes;
        vd->framesizeIn = (vd->width * vd->height << 1);

        cambuf = cams[id].cambuf;
        cambuf->width = vd->width;
        cambuf->height = vd->height;
        cambuf->framesizeIn = vd->framesizeIn;
        } break;
    case GET_FRAMERATE: {
        vd = cams[id].videoIn;
        output[0].i = vd->fps;
        } break;
    case SET_FRAMERATE: {
    //case IN_CMD_V4L2: {
        int value = input[1].i;
        XnMapOutputMode Mode;
        g_image.GetMapOutputMode(Mode);
        Mode.nFPS = value;
        rc = g_image.SetMapOutputMode(Mode);

        vd = cams[id].videoIn;
        vd->fps = Mode.nFPS;

        cambuf = cams[id].cambuf;
        cambuf->fps = vd->fps;
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
