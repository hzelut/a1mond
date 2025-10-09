#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "queue.h"
#include "network.h"
#include "command_interface.h"
#include "log.h"
#include "configure.h"

#define WORKER_MAX 10

typedef struct {
	bool			is_running;
	pthread_t worker[WORKER_MAX];
	queue_t*	job_que;

	// Modules
	network_t*						net;
	command_interface_t*	ci;
	log_t*								log;
	configure_t*					cnf;
}daemon_t;

typedef struct {
	void*		(*func)(void*);
	void*		arg;
}job_t;

extern daemon_t g_daemon;

void daemon_create();
void daemon_free();

void push_job(void* (*func)(void*), void* arg);

#endif //__DAEMON_H__
