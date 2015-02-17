#ifndef __CAR_H_
#define __CAR_H_

#define CAR_DEVICE_TYPE 0

#ifdef __cplusplus
extern "C" {
#endif
int car_init(device_argument *args, int id);
int car_stop(int id);
int car_run(int id);
int car_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output);

#ifdef __cplusplus
};
#endif

#endif
