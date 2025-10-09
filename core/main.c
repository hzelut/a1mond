#include <stdio.h>
#include "daemon.h"

#include <unistd.h>

#define PID_FILENAME "/var/run/a1mond.pid"

int main() {
	if(daemon_create(PID_FILENAME))
		return 1;

	printf("Hello world!!\n");

	while(g_daemon.is_running);
	return 0;
}
