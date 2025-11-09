#ifndef __SA_H__
#define __SA_H__

#include "netype.h"

typedef struct {
	struct {
		ip4_addr ip;
	}local, remote;
}sa_t;

typedef struct {
	sa_t*			self;
	buffer_t* data;
}sa_job_t;

void* sa_worker(void* arg);

#endif //__SA_H__
