#ifndef FETCH_H
#define FETCH_H

#include "config.h"
#include "cache.h"
#include "csapp.h"

#define MIN_PORT_NUMBER 1025
#define MAX_PORT_NUMBER 65535

void sigpipe_handler(int sig);
void fetch_from_server(char* model_md5, config_data* config);
void* cache_query(void* config);
void query(config_data* config);
void send_message(int* fd_remote, char* message);
ssize_t read_message(rio_t* rio_remote, char* buf, int read_this_many);
void open_connection_as_client(config_data* config, int* fd_remote, rio_t* rio_remote);
void request(config_data* config, int server_index, char* model_md5);
void write_file(char* file_path, char* model_md5, char* payload, size_t payload_size, int isModel);
void get_data_from_file(char* cache_path, char* payload, char* model_md5, int isModel);
void send_model();
#endif
