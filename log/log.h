#ifndef __LOG_H___
#define __LOG_H___

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

typedef enum {
	LT_STD,
	LT_SYSLOG,
	LT_FILE
} LOG_TYPE;

typedef enum {
	LL_ALL,
	LL_DBG,
	LL_WARN,
	LL_INFO,
	LL_ERR
} LOG_LEVEL;

typedef struct {
	LOG_TYPE	type;
	LOG_LEVEL level;
	FILE*			out;

	pthread_mutex_t mutex;
}log_t;

log_t*	log_create();
void		log_free(log_t* self);

void logging(LOG_LEVEL level, const char* module, const char* fmt, ...);

LOG_TYPE  get_log_type(const char* str);
LOG_LEVEL get_log_level(const char* str);

#endif //__LOG_H___
