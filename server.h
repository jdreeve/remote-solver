#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include "fetch.h"
#include "utils.h"
#include "cache.h"
#include "config.h"
typedef struct {
	config_data* config;
	int* connfd;
	pthread_mutex_t* cache_mutex;
	pthread_mutex_t* file_mutex;
} server_resources;

void run_solver_server(config_data* config);
server_resources* server_resources_create();
void* server_thread(void* server_rsc);
void serve(server_resources* server_resources);
void fork_exec_gillespy2(char* cache_path, char* model_md5);

#endif
