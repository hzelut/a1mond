#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include "netype.h"

typedef struct {
	struct {
		ip4_addr ip;
	}local, remote;
	char* secret;
}peer_t;

typedef struct {
	int port;
	struct {
		int type;
		int level;
	}log;
	peer_t* peers;
}configure_t;

configure_t*	cnf_create(const char* filename);
void					cnf_free(configure_t* self);

#endif //__CONFIGURE_H__
