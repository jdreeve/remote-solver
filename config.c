#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "csapp.h"
#include "cache.h"

config_data* new_config_data(void){
	config_data* config = Malloc(sizeof(config_data));
	if(DEBUG){
		printf("Configuration data struct allocated\n");
		fflush(stdout);
	}
	return config;
}
config_and_md5* config_and_md5_create(){
    config_and_md5* myself= (config_and_md5*)Malloc(sizeof(config_and_md5));
    if(NULL != myself){
        myself->config = NULL;
        myself->model_md5 = NULL;
    }
    return myself;
}

void erase_config_data(config_data* config){
	free(config->server_address);
	free(config->server_port);
	free(config->cache_path);
	free(config->listen_port);
	free(config->security_token);
	free(config->cache_only_flag);
	cache_destroy(config->cache);
	free(config);
}

void load_config_data(char* config_file_path, config_data* config){
	FILE *configfp = fopen(CONFIG_FILE, "r");
	char buf[MAXLINE];
	char* temp;

	fgets(buf, MAXLINE, configfp);
	temp = buf + strlen("server address: ");
	config->server_address = Malloc(strlen(temp)+1);
	temp[strlen(temp)-1] = '\0';
	strcpy(config->server_address, temp);
	
	fgets(buf, MAXLINE, configfp);
	temp = buf + strlen("server port: ");
	temp[strlen(temp)-1] = '\0';
	config->server_port = Malloc(strlen(temp)+1);
	strcpy(config->server_port, temp);

	fgets(buf, MAXLINE, configfp);
	temp = buf + strlen("cache path: ");
	temp[strlen(temp)-1] = '\0';
	config->cache_path = Malloc(strlen(temp)+1);
	strcpy(config->cache_path, temp);

	fgets(buf, MAXLINE, configfp);
	temp = buf + strlen("listen port: ");
	temp[strlen(temp)-1] = '\0';
	config->listen_port = Malloc(strlen(temp)+1);
	strcpy(config->listen_port, temp);
	
	fgets(buf, MAXLINE, configfp);
	temp = buf + strlen("security token: ");
	temp[strlen(temp)-1] = '\0';
	config->security_token = Malloc(strlen(temp)+1);
	strcpy(config->security_token, temp);

	fgets(buf, MAXLINE, configfp);
	temp = buf + strlen("cache only flag: ");
	temp[strlen(temp)-1] = '\0';
	config->cache_only_flag = Malloc(strlen(temp)+1);
	strcpy(config->cache_only_flag, temp);
	
	fclose(configfp);

	if(DEBUG){
		printf("Config data loaded from file %s\n", CONFIG_FILE);
		fflush(stdout);
		print_config_data(config);
	}
}

void print_config_data(config_data* config){
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("Printing configuration data:\n");
	printf("server address=%s\n", config->server_address);
	printf("server port=%s\n", config->server_port);
	printf("cache path=%s\n", config->cache_path);
	printf("port=%s\n", config->listen_port);
	printf("security token=%s\n", config->security_token);
	printf("cache-only flag=%s\n", config->cache_only_flag);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	fflush(stdout);
}
