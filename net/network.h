#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "queue.h"
#include "buffer.h"
#include "netype.h"

typedef struct {
	int				sock;
	int				port;
	queue_t*	recv_que;
	pthread_t worker_id;
}network_t;

network_t*	net_create();
void				net_free(network_t* self);

void			net_send(ip4_addr src, ip4_addr dst, buffer_t* data);
buffer_t* net_recv(ip4_addr* src, ip4_addr* dst);

#endif //__NETWORK_H__
