#include "daemon.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

daemon_t g_daemon;

void _daemon_signal_handler(int sig);
void* _daemon_worker_loop(void* arg);

void daemon_create() {
	g_daemon.is_running = true;
	g_daemon.job_que = que_create(true);

	// Signal
	signal(SIGINT, _daemon_signal_handler);

	// Create modules
	g_daemon.cnf = cnf_create("conf/local.conf");
	g_daemon.log = log_create();
	g_daemon.net = net_create();
	g_daemon.ci = ci_create();
	
	// Run worker
	for(int i = 0; i < WORKER_MAX; i++) {
		int* arg = calloc(1, sizeof(int));
		*arg = i;
		if(pthread_create(&g_daemon.worker[i], NULL, _daemon_worker_loop, arg) != 0) {
			daemon_free();
			break;
		}
		printf("Create worker[%d]\n", i);
	}
}

void daemon_free() {
	g_daemon.is_running = false;

	// Close worker
	for(int i = 0; i < WORKER_MAX; i++) {
		if(!que_enque(g_daemon.job_que, NULL)) {
			// Fail
			return;
		}
	}

	// Wait for worker
	for(int i = 0; i < WORKER_MAX; i++)
		pthread_join(g_daemon.worker[i], NULL);

	// Free modules
	que_free(g_daemon.job_que);
	net_free(g_daemon.net);
	ci_free(g_daemon.ci);
}

void push_job(void* (*func)(void*), void* arg) {
	job_t* job = calloc(1, sizeof(job_t));
	job->func = func;
	job->arg = arg;

	if(!que_enque(g_daemon.job_que, job)) {
		// Fail
	}
}

void _daemon_signal_handler(int sig) {
	switch(sig) {
		case SIGINT:
			daemon_free();
			break;
	}
}

void* _daemon_worker_loop(void* arg) {
	int id = *(int*)arg;
	free(arg);

	while(g_daemon.is_running) {
		job_t* job = que_deque(g_daemon.job_que);
		if(job == NULL)
			break;
		else if(job->func) {
			job->func(job->arg);
			free(job);
		}
	}

	printf("Close worker[%d]\n", id);
	return NULL;
}
