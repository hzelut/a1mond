#include "command_interface.h"
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>
#include "daemon.h"
#include <stdio.h>

#define SOCK_PATH "/tmp/almond.sock"
#define BUF_SIZE 1024

void* _ci_worker(void* arg);

command_interface_t* ci_create() {
	command_interface_t* self = calloc(1, sizeof(command_interface_t));

	unlink(SOCK_PATH);
	self->sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(self->sock < 0) {
		return NULL;
	}

	// Binding
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);
	if(bind(self->sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return NULL;
	}

	// Listening
	if(listen(self->sock, 5) < 0) {
		return NULL;
	}

	// Run worker
	pthread_create(&self->worker_id, NULL, _ci_worker, NULL);

	return self;
}

void ci_free(command_interface_t* self) {
	shutdown(self->sock, SHUT_RD);

	// Wait worker
	pthread_join(self->worker_id, NULL);

	free(self);
}

void* _ci_worker(void* arg) {
	(void)arg;
	command_interface_t* self = g_daemon.ci;
	int sock;
	char buf[BUF_SIZE];

	printf("Running ci worker\n");
	while(g_daemon.is_running) {
		sock = accept(self->sock, NULL, NULL);
		if(sock < 0) {
			continue;
		}
		int len = read(sock, buf, BUF_SIZE - 1);
		if(len > 0) {
			printf("Received %d\n", len);
			write(sock, "ok\0", 3);

			if(strcmp(buf, "shutdown") == 0) {
				daemon_free();
				break;
			}

			memset(buf, 0, BUF_SIZE);
		}
		close(sock);
	}

	printf("Close ci worker\n");
	unlink(SOCK_PATH);
	return NULL;
}
