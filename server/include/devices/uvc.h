#ifndef __UVC_H_
#define __UVC_H_

#define UVC_DEVICE_TYPE 1

#ifdef __cplusplus
extern "C" {
#endif
int uvc_init(device_argument *args, int id);
int uvc_stop(int id);
int uvc_run(int id);
int uvc_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output);

#ifdef __cplusplus
};
#endif

#endif
