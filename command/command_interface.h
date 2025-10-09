#ifndef __COMMAND_INTERFACE_H__
#define __COMMAND_INTERFACE_H__

#include <pthread.h>

typedef struct {
	int				sock;
	pthread_t worker_id;
}command_interface_t;

command_interface_t*	ci_create();
void									ci_free(command_interface_t* self);

#endif //__COMMAND_INTERFACE_H__
