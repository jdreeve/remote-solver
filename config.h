#ifndef CONFIG_H
#define CONFIG_H
#include <pthread.h>
#include "cache.h"

#define CONFIG_FILE ".server-config"
#define DEBUG 1
typedef struct {
	char* server_address;
	char* server_port;
	char* cache_path;
	char* listen_port;
	char* security_token;
	char* cache_only_flag;
	cache* cache;
} config_data;

typedef struct {
	config_data* config;
	char* model_md5;
} config_and_md5;

config_data* new_config_data();
config_and_md5* config_and_md5_create();
void load_config_data(char* config_file, config_data* config);
void print_config_data(config_data* config);
void erase_config_data(config_data* config);

#endif
