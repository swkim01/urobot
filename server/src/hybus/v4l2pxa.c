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

#include <stdlib.h>
#include <sys/poll.h>
#include "v4l2pxa.h"

static int debug = 0;
static int init_v4l2(struct vdIn *vd);

int init_videoIn(struct vdIn *vd, char *device, int width, int height, int fps, int format, int grabmethod)
{
  if (vd == NULL || device == NULL)
    return -1;
  if (width == 0 || height == 0)
    return -1;
  if (grabmethod < 0 || grabmethod > 1)
    grabmethod = 1;		//mmap by default;
  vd->videodevice = NULL;
  vd->status = NULL;
  vd->pictName = NULL;
  vd->videodevice = (char *) calloc (1, 16 * sizeof (char));
  vd->status = (char *) calloc (1, 100 * sizeof (char));
  vd->pictName = (char *) calloc (1, 80 * sizeof (char));
  snprintf (vd->videodevice, 12, "%s", device);
  vd->toggleAvi = 0;
  vd->getPict = 0;
  vd->signalquit = 1;
  vd->width = width;
  vd->height = height;
  vd->fps = fps;
  vd->formatIn = format;
  vd->grabmethod = grabmethod;
  if (init_v4l2 (vd) < 0) {
    fprintf (stderr, " Init v4L2 failed !! exit fatal \n");
    goto error;;
  }
  /* alloc a temp buffer to reconstruct the pict */
  vd->framesizeIn = (vd->width * vd->height << 1);
  switch (vd->formatIn) {
  case V4L2_PIX_FMT_MJPEG:
    vd->tmpbuffer = (unsigned char *) calloc(1, (size_t) vd->framesizeIn);
    if (!vd->tmpbuffer)
      goto error;
    vd->framebuffer =
        (unsigned char *) calloc(1, (size_t) vd->width * (vd->height + 8) * 2);
    break;
  case V4L2_PIX_FMT_YUV422P:
    vd->framebuffer =
        (unsigned char *) calloc(1, (size_t) vd->framesizeIn);
    break;
  default:
    fprintf(stderr, " should never arrive exit fatal !!\n");
    goto error;
    break;
  }
  if (!vd->framebuffer)
    goto error;
  return 0;
error:
  free(vd->videodevice);
  free(vd->status);
  free(vd->pictName);
  close(vd->fd);
  return -1;
}

static int init_v4l2(struct vdIn *vd)
{
  int i;
  int sensor = OV7660_SENSOR;
  int ret = 0;
  int mode = CAM_MODE_VIDEO;

  if ((vd->fd = open(vd->videodevice, O_RDONLY)) == -1) {
    perror("ERROR opening V4L interface");
    return -1;
  }

  ret = ioctl(vd->fd, VIDIOC_S_INPUT, &sensor);
  if (ret < 0) {
    fprintf(stderr, "Error opening device %s: unable to set input.\n", vd->videodevice);
    close(vd->fd);
    goto fatal;
  }

  memset(&vd->cap, 0, sizeof(struct v4l2_capability));
  ret = ioctl(vd->fd, VIDIOC_QUERYCAP, &vd->cap);
  if (ret < 0) {
    fprintf(stderr, "Error opening device %s: unable to query device.\n", vd->videodevice);
    goto fatal;
  }

  if ((vd->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
    fprintf(stderr, "Error opening device %s: video capture not supported.\n",
           vd->videodevice);
    goto fatal;;
  }

  if (vd->grabmethod) {
    if (!(vd->cap.capabilities & V4L2_CAP_STREAMING)) {
      fprintf(stderr, "%s does not support streaming i/o\n", vd->videodevice);
      goto fatal;
    }
  } else {
    if (!(vd->cap.capabilities & V4L2_CAP_READWRITE)) {
      fprintf(stderr, "%s does not support read i/o\n", vd->videodevice);
      goto fatal;
    }
  }

  /*
   * set framerate
   */
  struct v4l2_streamparm *setfps;
  setfps = (struct v4l2_streamparm *) calloc(1, sizeof(struct v4l2_streamparm));
  memset(setfps, 0, sizeof(struct v4l2_streamparm));
  setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  setfps->parm.capture.timeperframe.numerator = 0/*1*/;
  setfps->parm.capture.timeperframe.denominator = 0/*vd->fps*/;
  setfps->parm.capture.capturemode = mode;
  setfps->parm.capture.extendedmode = CI_SSU_SCALE_DISABLE;

  ret = ioctl(vd->fd, VIDIOC_S_PARM, setfps);
  if (ret < 0) {
    perror("Unable to set framerate");
    goto fatal;
  }

  /*
   * set format in
   */
  memset(&vd->fmt, 0, sizeof(struct v4l2_format));
  vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vd->fmt.fmt.pix.width = vd->width;
  vd->fmt.fmt.pix.height = vd->height;
  vd->fmt.fmt.pix.pixelformat = vd->formatIn;
  vd->fmt.fmt.pix.field = V4L2_FIELD_ANY;
  ret = ioctl(vd->fd, VIDIOC_S_FMT, &vd->fmt);
  if (ret < 0) {
    perror("Unable to set format");
    goto fatal;
  }

  if ((vd->fmt.fmt.pix.width != vd->width) ||
      (vd->fmt.fmt.pix.height != vd->height)) {
    fprintf(stderr, " format asked unavailable get width %d height %d \n", vd->fmt.fmt.pix.width, vd->fmt.fmt.pix.height);
    vd->width = vd->fmt.fmt.pix.width;
    vd->height = vd->fmt.fmt.pix.height;
    /*
     * look the format is not part of the deal ???
     */
    // vd->formatIn = vd->fmt.fmt.pix.pixelformat;
  }

  /*
   * request buffers
   */
  memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));

// VIDEOBUF_COUNT must be larger than STILLBUF_COUNT
#define VIDEOBUF_COUNT  3
#define STILLBUF_COUNT  2

  if (mode == CAM_MODE_VIDEO) {
    vd->rb.count = VIDEOBUF_COUNT;
  } else {
    vd->rb.count = STILLBUF_COUNT;
  }
  //vd->rb.count = NB_BUFFER;
  vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vd->rb.memory = V4L2_MEMORY_MMAP;

  ret = ioctl(vd->fd, VIDIOC_REQBUFS, &vd->rb);
  if (ret < 0) {
    perror("Unable to allocate buffers");
    goto fatal;
  }

  /*
   * map the buffers
   */
  for (i = 0; i < vd->rb.count; i++) {
    memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
    vd->buf.index = i;
    vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(vd->fd, VIDIOC_QUERYBUF, &vd->buf);
    if (ret < 0) {
      perror("Unable to query buffer");
      goto fatal;
    }

    if (debug)
      fprintf(stderr, "length: %u offset: %u\n", vd->buf.length, vd->buf.m.offset);

    vd->mem[i] = mmap(0 /* start anywhere */ ,
                      vd->buf.length, PROT_READ, MAP_SHARED, vd->fd,
                      vd->buf.m.offset);
    if (vd->mem[i] == MAP_FAILED) {
      perror("Unable to map buffer");
      goto fatal;
    }
    if (debug)
      fprintf(stderr, "Buffer mapped at address %p.\n", vd->mem[i]);
  }

  /*
   * Queue the buffers.
   */
  for (i = 0; i < vd->rb.count; i++) {
    memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
    vd->buf.index = i;
    vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
    if (ret < 0) {
      perror("Unable to queue buffer");
      goto fatal;;
    }
  }
  return 0;
fatal:
  return -1;

}

int video_enable(struct vdIn *vd)
{
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret;

  ret = ioctl(vd->fd, VIDIOC_STREAMON, &type);
  if (ret < 0) {
    perror("Unable to start capture");
    return ret;
  }
  vd->streamingState = STREAMING_ON;
  return 0;
}

static int video_disable(struct vdIn *vd, streaming_state disabledState)
{
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret;

  ret = ioctl(vd->fd, VIDIOC_STREAMOFF, &type);
  if (ret < 0) {
    perror("Unable to stop capture");
    return ret;
  }
  vd->streamingState = disabledState;
  return 0;
}

/******************************************************************************
Description.: 
Input Value.: 
Return Value: 
******************************************************************************/
int is_huffman(unsigned char *buf)
{
  unsigned char *ptbuf;
  int i = 0;
  ptbuf = buf;
  while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda) {
    if (i++ > 2048)
      return 0;
    if (((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
      return 1;
    ptbuf++;
  }
  return 0;
}

int pxaGrab(struct vdIn *vd)
{
#define HEADERFRAME1 0xaf
  int ret;
  struct pollfd ufds;

  if (vd->streamingState == STREAMING_OFF) {
    if (video_enable(vd))
      goto err;
  }

  // FIXME
  // This function maybe obsolete
  ufds.fd         = vd->fd;
  ufds.events     = POLLIN;
  do {
    ret = poll(&ufds, 1, -1);
  } while (!ret);

  memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
  vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vd->buf.memory = V4L2_MEMORY_MMAP;

  ret = ioctl(vd->fd, VIDIOC_DQBUF, &vd->buf);
  if (ret < 0) {
    perror("Unable to dequeue buffer");
    goto err;
  }

  switch (vd->formatIn) {
    case V4L2_PIX_FMT_MJPEG:
      if (vd->buf.bytesused <= HEADERFRAME1) {    /* Prevent crash
                                                  * on empty image */
        fprintf(stderr, "Ignoring empty buffer ...\n");
        return 0;
      }

      memcpy(vd->tmpbuffer, vd->mem[vd->buf.index], vd->buf.bytesused);

      if (debug)
        fprintf(stderr, "bytes in used %d \n", vd->buf.bytesused);
      break;

    case V4L2_PIX_FMT_YUV422P:
      if (debug)
        fprintf(stderr, "bytes in used %d \n", vd->buf.bytesused);
      vd->framebuffer = vd->mem[vd->buf.index];
      //memcpy (vd->framebuffer, vd->mem[vd->buf.index], (size_t) vd->framesizeIn);
      /*
      if (vd->buf.bytesused > vd->framesizeIn)
        memcpy (vd->framebuffer, vd->mem[vd->buf.index], (size_t) vd->framesizeIn);
      else
        memcpy (vd->framebuffer, vd->mem[vd->buf.index], (size_t) vd->buf.bytesused);
      */
      break;

    default:
      goto err;
    break;
  }

  ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
  if (ret < 0) {
    perror("Unable to requeue buffer");
    goto err;
  }

  return vd->mem[vd->buf.index];

err:
  vd->signalquit = 0;
  return -1;
}

int close_v4l2(struct vdIn *vd)
{
  if (vd->streamingState == STREAMING_ON)
    video_disable(vd, STREAMING_OFF);
  if (vd->tmpbuffer)
    free(vd->tmpbuffer);
  vd->tmpbuffer = NULL;
  free(vd->framebuffer);
  vd->framebuffer = NULL;
  free(vd->videodevice);
  free(vd->status);
  free(vd->pictName);
  vd->videodevice = NULL;
  vd->status = NULL;
  vd->pictName = NULL;

  return 0;
}
