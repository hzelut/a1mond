#include "network.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include "daemon.h"
#include <stdio.h>

void* _net_worker(void* arg);

network_t* net_create() {
	network_t* self = calloc(1, sizeof(network_t));
	self->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(self->sock < 0) {
		return NULL;
	}
	self->port = g_daemon.cnf->port;
	self->recv_que = que_create(true);

	// Packet info
	int opt = 1;
	setsockopt(self->sock, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt));

	// Binding
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(self->port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(self->sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return NULL;
	}

	// Run worker
	pthread_create(&self->worker_id, NULL, _net_worker, NULL);

	return self;
}

void net_free(network_t* self) {
	que_enque(self->recv_que, NULL);
	shutdown(self->sock, SHUT_RD);

	// Wait worker
	pthread_join(self->worker_id, NULL);

	que_free(self->recv_que);
	free(self);
}

void* _net_worker(void* arg) {
	(void)arg;
	network_t* self = g_daemon.net;
	char buf[1024];
	char ancillary[64];

	printf("Running net worker\n");
	while(g_daemon.is_running) {
		struct msghdr msg;
		struct sockaddr_in client;
		struct iovec iov;
		iov.iov_base = buf;
		iov.iov_len = sizeof(buf);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_name = &client;
		msg.msg_namelen = sizeof(client);
		msg.msg_control = ancillary;
		msg.msg_controllen = sizeof(ancillary);
		msg.msg_flags = 0;

		int len = recvmsg(self->sock, &msg, 0);
		if(len > 0) {
			for(struct cmsghdr* cm = CMSG_FIRSTHDR(&msg);
					cm != NULL;
					cm = CMSG_NXTHDR(&msg, cm))
			{
				if(cm->cmsg_level == IPPROTO_IP) {
					struct in_pktinfo* info = (struct in_pktinfo*)CMSG_DATA(cm);
					buffer_t* data = buf_create(len);
					packet_t* pkt = pkt_create(
							client.sin_addr.s_addr,
							info->ipi_addr.s_addr,
							data
							);
					que_enque(self->recv_que, pkt);
					printf("Received %d\n", len);
					break;
				}
			}
		}
	}

	printf("Close net worker\n");
	return NULL;
}
