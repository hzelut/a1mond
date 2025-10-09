#include "daemon.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

daemon_t g_daemon;

void _daemon_signal_handler(int sig);
void* _daemon_worker_loop(void* arg);
int _check_pid();
void _daemonize();

int daemon_create(const char* pid_file) {
	memset(g_daemon.pid_file, 0, sizeof(g_daemon.pid_file));
	strncpy(g_daemon.pid_file, pid_file, sizeof(g_daemon.pid_file)-1);
	g_daemon.pid_file[sizeof(g_daemon.pid_file)-1] = '\0';

	if(_check_pid()) {
		return 1;
	}
	_daemonize();

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
	return 0;
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

	// Clear pid
	if(g_daemon.pid_fd >= 0) {
		close(g_daemon.pid_fd);
		g_daemon.pid_fd = -1;
	}
	if(*g_daemon.pid_file)
		unlink(g_daemon.pid_file);
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

int _check_pid() {
	FILE *f = fopen(g_daemon.pid_file, "r");
	if(!f)
		return 0;

	pid_t pid;
	if(fscanf(f, "%d", &pid) != 1) {
		fclose(f);
		return 0;
	}
	fclose(f);

	if(kill(pid, 0) == 0) {
		return 1;
	} else {
		if(errno == ESRCH)
			unlink(g_daemon.pid_file);
		return 0;
	}
}

void _daemonize() {
	pid_t pid = 0;

	// First fork()
	pid = fork();
	if(pid < 0)
		exit(EXIT_FAILURE);
	if(pid > 0)
		exit(EXIT_SUCCESS);

	// Create new session
	if(setsid() < 0)
		exit(EXIT_FAILURE);
	signal(SIGCHLD, SIG_IGN);

	// Second fork()
	pid = fork();
	if(pid < 0)
		exit(EXIT_FAILURE);
	if(pid > 0)
		exit(EXIT_SUCCESS);

	// Set a new permission
	umask(0);
	chdir("/");

	// Close all FDs
	for(int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
		close(fd);

	open("/dev/null", O_RDONLY);	// STDIN
	open("/dev/null", O_RDWR);		// STDOUT
	open("/dev/null", O_RDWR);		// STDERR

	if(*g_daemon.pid_file) {
		g_daemon.pid_fd = open(g_daemon.pid_file, O_RDWR | O_CREAT, 0640);
		if(g_daemon.pid_fd < 0)
			exit(EXIT_FAILURE);
		if(lockf(g_daemon.pid_fd, F_TLOCK, 0) < 0)
			exit(EXIT_FAILURE);

		char buf[32];
		int len = snprintf(buf, sizeof(buf), "%d\n", getpid());
		if(write(g_daemon.pid_fd, buf, len) != len)
			exit(EXIT_FAILURE);
	}
}
