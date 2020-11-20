#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "cache.h"
#include "config.h"
#include "fetch.h"

char* parse_command_line(int argc, char* argv[]);

int main(int argc, char* argv[]){
	char* model_md5 = NULL;
	config_data* config = new_config_data();

	model_md5 = parse_command_line(argc, argv);

	if(DEBUG){
		printf("Model md5=%s\n", model_md5);
	}

	load_config_data(CONFIG_FILE, config);

	config->cache = cache_create();

	printf("Populating cache\n");
	fflush(stdout);
	cache_populate(config->cache, config->cache_path);

	printf("Printing cache\n");
	fflush(stdout);
	cache_print(config->cache);

	if(NULL != model_md5){
		printf("Fetching\n");
		fetch_from_server(model_md5, config);
	}

	if(NULL == model_md5){
		printf("Serving\n");
		run_solver_server(config);
	}

	printf("Cleaning up.\n");
	fflush(stdout);
	erase_config_data(config);

	printf("Complete.\n");
}

char* parse_command_line(int argc, char* argv[]){
	if(argc <2){
		return NULL;
	}
	else{
	return argv[1];
	}
}

