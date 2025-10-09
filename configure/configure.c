#include "configure.h"
#include <stdlib.h>
#include "log.h"
#include <confuse.h>

cfg_opt_t addr_opts[] = {
	CFG_STR("ip", "0.0.0.0", CFGF_NONE),
	CFG_END()
};

cfg_opt_t peer_opts[] = {
	CFG_SEC("local", addr_opts, CFGF_NONE),
	CFG_SEC("remote", addr_opts, CFGF_NONE),
	CFG_STR("secret", "thisisaseret", CFGF_NONE),
	CFG_END()
};

cfg_opt_t log_opts[] = {
	CFG_STR("type", "stdout", CFGF_NONE),
	CFG_STR("level", "all", CFGF_NONE),
	CFG_END()
};

cfg_opt_t opts[] = {
	CFG_INT("port", 500, CFGF_NONE),
	CFG_SEC("log", log_opts, CFGF_NONE),
	CFG_SEC("peer", peer_opts, CFGF_MULTI),
	CFG_END()
};

configure_t* cnf_create(const char* filename) {
	configure_t* self = calloc(1, sizeof(configure_t));

	cfg_t* cfg = cfg_init(opts, CFGF_NONE);
	if(cfg_parse(cfg, filename) == CFG_PARSE_ERROR) {
		cfg_free(cfg);
		return NULL;
	}

	self->port = cfg_getint(cfg, "port");
	cfg_t* log_cfg = cfg_getsec(cfg, "log");
	self->log.type = get_log_type(cfg_getstr(log_cfg, "type"));
	self->log.level = get_log_level(cfg_getstr(log_cfg, "level"));

	int peer_len = cfg_size(cfg, "peer");
	self->peers = calloc(peer_len, sizeof(peer_t));
	for(int i = 0; i < peer_len; i++) {
		cfg_t* peer_cfg = cfg_getnsec(cfg, "peer", i);
		peer_t* peer = &self->peers[i];
		peer->secret = cfg_getstr(peer_cfg, "secret");
		{
			cfg_t* local_cfg = cfg_getsec(peer_cfg, "local");
			peer->local.ip = ip4_stoa(cfg_getstr(local_cfg, "ip"));
		}
		{
			cfg_t* remote_cfg = cfg_getsec(peer_cfg, "remote");
			peer->remote.ip = ip4_stoa(cfg_getstr(remote_cfg, "ip"));
		}
	}

	cfg_free(cfg);
	return self;
}

void cnf_free(configure_t* self) {
	free(self);
}
