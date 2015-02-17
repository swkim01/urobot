#ifndef _CAMBUF_H_
#define _CAMBUF_H_

enum pixelFormat {
    PIXEL_FORMAT_YUV = 0,
    PIXEL_FORMAT_RGB,
    PIXEL_FORMAT_JPG,
    PIXEL_FORMAT_MJPG,
    PIXEL_FORMAT_H264,
    PIXEL_FORMAT_MP4,
};

struct _cameraBuffer {
    int width;
    int height;
    int fps;
    enum pixelFormat formatIn;
    int formatOut;
    int framesizeIn;
    unsigned char * framebuffer;
};
typedef struct _cameraBuffer cameraBuffer;

#endif /*_CAMBUF_H_*/
