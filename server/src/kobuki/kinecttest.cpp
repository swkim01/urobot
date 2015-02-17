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

//#include "utils.h"
#include "kobuki.h"
#include "openni_kinect.h"
#include "jpeg_utils.h"

#define INPUT_PLUGIN_NAME "Kinect camera grabber"

/* private functions and variables to this plugin */
static int gquality = 80;

xn::Context g_context;
#if XN_VERSION >= 103000000
xn::ScriptNode g_scriptNode;
#endif
xn::ImageGenerator g_image;
xn::ImageMetaData g_imageMD;
xn::EnumerationErrors errors;

extern "C" int main()
{
    char *dev = "/dev/usb";
    char *xmlfile = "../SamplesConfig.xml";
    int width = 640, height = 480, fps = 30;
    XnPixelFormat format = XN_PIXEL_FORMAT_RGB24;
    XnStatus rc = XN_STATUS_OK;
    struct vdIn *vd;
    unsigned char *buf;
    int stop=0, size;

    /* allocate webcam datastructure */
    vd = (struct vdIn *) malloc(sizeof(struct vdIn));
    if(vd == NULL) {
        IPRINT("not enough memory for videoIn\n");
        exit(EXIT_FAILURE);
    }
    memset(vd, 0, sizeof(struct vdIn));

    /* display the parsed values */
    IPRINT("Using Kinect device.: %s\n", dev);
    IPRINT("Desired Resolution: %i x %i\n", width, height);
    IPRINT("Frames Per Second.: %i\n", fps);
    IPRINT("Format............: %s\n", (format == XN_PIXEL_FORMAT_YUV422) ? "YUV" : "RGB");
    IPRINT("JPEG Quality......: %d\n", gquality);

    DBG("vdIn pn: %d\n", id);
    // Initialize context object
    //rc = g_context.Init();
#if XN_VERSION < 103000000
    rc = g_context.InitFromXmlFile(xmlfile, &errors);
#else
    rc = g_context.InitFromXmlFile(xmlfile, g_scriptNode, &errors);
#endif
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }

    IPRINT("g_context.Init\n");
    // Create a ImageGenerator node
    //rc = g_image.Create(g_context);
    rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
    if(rc != XN_STATUS_OK) {
        fprintf(stderr, "%s\n", xnGetStatusString(rc));
        exit(EXIT_FAILURE);
    }

    IPRINT("g_image.Create\n");
    // Set resolution
    XnMapOutputMode Mode;
    g_image.GetMapOutputMode(Mode);
    Mode.nXRes = xn::Resolution(XN_RES_VGA).GetXResolution();
    Mode.nYRes = xn::Resolution(XN_RES_VGA).GetYResolution();
    Mode.nFPS = 30;
    rc = g_image.SetMapOutputMode(Mode);
    //rc = g_image.SetPixelFormat(format);

    vd->pImageMD = &g_imageMD;
    vd->width = Mode.nXRes;
    vd->height = Mode.nYRes;
    vd->fps = Mode.nFPS;
    vd->framesizeIn = (vd->width *vd->height << 1);
    vd->formatIn = format;
    IPRINT("width=%d, height=%d\n", vd->width, vd->height);

    IPRINT("g_image.SetMapOutputMode, g_image.SetPixelFormat\n");
    // Make it start generating data
    rc = g_context.StartGeneratingAll();

    IPRINT("g_context.StartGeneratingAll\n");
    buf = (unsigned char *)malloc(vd->framesizeIn);
    if(buf == NULL) {
        fprintf(stderr, "could not allocate memory\n");
        exit(EXIT_FAILURE);
    }

    while(!stop) {
        /* grab a frame */
        // Wait for new data to be available
        rc = g_context.WaitOneUpdateAll(g_image);
        if (rc != XN_STATUS_OK)
        {
            IPRINT("Failed updating data: %s\n", xnGetStatusString(rc));
            continue;
        }

        // Take current image map
        if (g_image.IsValid()) {
	        g_image.GetMetaData(g_imageMD);
        	IPRINT("Frame %d: YOffset=%d, XRes=%d, YRes=%d, PixelFormat=%s\n", vd->pImageMD->FrameID(), vd->pImageMD->YOffset(), vd->pImageMD->XRes(), vd->pImageMD->YRes(), vd->pImageMD->PixelFormat() == XN_PIXEL_FORMAT_RGB24?"RGB":"YUV");
	}
        //vd->pImageMap = g_image.GetYUV422ImageMap();
        //vd->pImageMap = g_ImageMD.Data();
        vd->pImageMap = vd->pImageMD->Data();

        /*
         * If capturing in YUV mode convert to JPEG now.
         */
        switch (vd->pImageMD->PixelFormat()) {
        case XN_PIXEL_FORMAT_YUV422:
            size = compress_yuyv_to_jpeg(vd, buf, vd->framesizeIn, gquality);
            break;
        case XN_PIXEL_FORMAT_RGB24:
            size = compress_rgb_to_jpeg(vd, buf, vd->framesizeIn, gquality);
            break;
        }

        /* only use usleep if the fps is below 5, otherwise the overhead is too long */
        if(vd->fps < 5) {
            DBG("waiting for next frame for %d us\n", 1000 * 1000 / vd->fps);
            usleep(1000 * 1000 / vd->fps);
        } else {
            DBG("waiting for next frame\n");
        }
        //stop = 1;
    }

    const char *name = "still.jpg";
    unlink(name);
    int tfd = creat (name, 0777);
    if (write (tfd, buf, size) != size) {
        perror("write on jpg");
        exit(1);
    }
    close(tfd);

    g_image.Release();
#if XN_VERSION < 103000000
    g_context.Shutdown();
#else
    g_scriptNode.Release();
    g_context.Release();
#endif
    if(vd != NULL) free(vd);
    if(buf != NULL) free(buf);
}
