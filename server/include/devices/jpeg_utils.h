#ifndef _JPEG_UTILS_H_
#define _JPEG_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif
int compress_yuyv_to_jpeg(cameraBuffer *cambuf, unsigned char *buffer, int size, int quality);
int compress_rgb_to_jpeg(cameraBuffer *cambuf, unsigned char *buffer, int size, int quality);
#ifdef __cplusplus
};
#endif

#endif
