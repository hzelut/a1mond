#include <stdio.h>
#include "daemon.h"

#include <unistd.h>

int main() {
	daemon_create();
	printf("Hello world!!\n");

	while(g_daemon.is_running);
	return 0;
}
