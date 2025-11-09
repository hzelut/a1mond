#include "sa.h"
#include <stdlib.h>
#include "log.h"

static const char* MM = "SA";

void* sa_worker(void* arg) {
	sa_job_t* args = (sa_job_t*)arg;
	sa_t* self = args->self;
	buffer_t* data = args->data;

	logging(LL_DBG, MM, "%d", data->size);
	free(args);
	return NULL;
}
