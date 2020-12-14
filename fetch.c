#include <stdio.h>
#include <pthread.h>
#include "utils.h"
#include "fetch.h"
#include "config.h"
#include "cache.h"

void sigpipe_handler(int sig){
	if(DEBUG){
		printf("SIGPIPE handled\n");
		fflush(stdout);
	}
	return;
}

void fetch_from_server(char* model_md5, config_data* config){
	node* node;
	int server_index = -1;

	
	if((node = cache_search(config->cache, model_md5)) != NULL){//result already in local cache
		printf("Results for %s found in local cache.\n", model_md5);
		return;
	}
	

	config_and_md5* config_and_md5 = config_and_md5_create();
	config_and_md5->config=config;
	config_and_md5->model_md5=model_md5;
	//if multiple servers implemented, thread this for concurrent queries
	pthread_t tid;
	Pthread_create(&tid, NULL, cache_query, config_and_md5); 
//	Pthread_detach(tid);
	Pthread_join(tid, (void**)&server_index);
	//end concurrent section
	printf("Server index = %d\n", server_index);
	if(server_index >= 0){
		printf("Remote-solver: Results file cached at %s\n", config->server_address);
		printf("Retrieving Results file...\n");
	}
	else{
		printf("Remote-solver: no cached solution. Requesting simulation.\n");
		printf("This could take a while...\n");
	}

	//serial - only request from one server at a time
	request(config, server_index, model_md5);
}

void* cache_query(void* configuration_and_md5){
	printf("Cache query active\n");
	config_and_md5* this_config_and_md5 = NULL;
	this_config_and_md5 = (config_and_md5*)configuration_and_md5;
	config_data* this_config = (config_data*)this_config_and_md5->config;
	char* this_model_md5 = this_config_and_md5->model_md5;
	unsigned this_tid = Pthread_self();
	char buf[MAXLINE];
	int ret = 0;

	if(DEBUG){
		printf("Thread created. tid = %u\n", this_tid);
		fflush(stdout);
	}
	Signal(SIGPIPE, sigpipe_handler);

	if(DEBUG){
		printf("Thread management complete.\n");
		fflush(stdout);
	}

	rio_t rio_remote;
	int fd_remote;
	open_connection_as_client(this_config, &fd_remote, &rio_remote);
	send_message(&fd_remote, "CACHEQUERY\n");

	printf("Sent CACHEQUERY\n");
	fflush(stdout);

	read_message(&rio_remote, buf, MAXLINE);
	if(!strcmp(buf, "CACHEQUERY detected\n")){
		printf("%s", buf);
		send_message(&fd_remote, this_model_md5);
	}
	else{
		ret = -1;
		pthread_exit((void*)&ret);
	}

	read_message(&rio_remote, buf, MAXLINE);

	printf("Cache query result: %s", buf);
	if(!strcmp(buf, "FOUND\n")){
		ret = 1;
	}
	printf("Thread closing. tid = %u\n", this_tid);
	Pthread_exit((void*) &ret);

	return NULL;
}

void send_message(int* fd_remote, char* message){
	Rio_writen(*fd_remote, message, strlen(message));
}

ssize_t read_message(rio_t* rio_contact, char* buf, int read_this_many){
	ssize_t ret = Rio_readlineb(rio_contact, buf, read_this_many);
	return ret;
}

void open_connection_as_client(config_data* config, int* fd_remote, rio_t* rio_remote){
	*fd_remote = Open_clientfd(config->server_address, config->server_port);
	printf("Open_clientfd complete\n");
	if(DEBUG){
		printf("Open_clientfd complete\n");
		fflush(stdout);
	}
	Rio_readinitb(rio_remote, *fd_remote);
	if(DEBUG){
		printf("open_client_connection finished execution\n");
		fflush(stdout);
	}
}

void request(config_data* config, int server_index, char* model_md5){
	printf("Requesting Results pickle\n");

	char this_model_md5[33];
	char buf[MAXLINE];
	rio_t rio_remote;
	int fd_remote;
	char payload[MAX_OBJECT_SIZE];

	strcpy(this_model_md5, model_md5);
	this_model_md5[strlen(model_md5)] = '\n';
	this_model_md5[strlen(model_md5)+1] = '\0';
	open_connection_as_client(config, &fd_remote, &rio_remote);
	send_message(&fd_remote, "REQUEST\n");
	printf("Sent REQUEST\n");
	fflush(stdout);

	read_message(&rio_remote, buf, MAXLINE);
	if(!strcmp(buf, "REQUEST detected\n")){
		printf("%s\n", buf);
		send_message(&fd_remote, this_model_md5);
		printf("Sent model md5\n");
	}
	else{
		printf("Bad response from server: %s\n", buf);
	       	return;
	}
	read_message(&rio_remote, buf, MAXLINE);
	if(strcmp(buf, "NOTCACHED\n") && strcmp(buf, "CACHED\n")){
		printf("Bad response from server: %s\n", buf);
		return;
	}
	if(!strcmp(buf, "NOTCACHED\n")){
		printf("%s\n", buf);
		printf("Sending model to server\n");
		get_data_from_file(config->cache_path, payload, model_md5, 1);
		printf("Model loaded from file: %s\n", payload);
		size_t payload_size = strlen(payload);
		char payload_size_str[20];
		printf("Payload size: %ld\n", payload_size);
		sprintf(payload_size_str, "%ld\n", payload_size);
		printf("Payload size str: %s\n", payload_size_str);
		strcat(payload, "\n");
		send_message(&fd_remote, payload_size_str);
		printf("Payload size sent\n");
		printf("Waiting on server to run simulation. This could take a while.\n");
		send_message(&fd_remote, payload);
	}
	if(!strcmp(buf, "CACHED\n")){
		printf("%s\n", buf);
		printf("Downloading cached Results from server.\n");
	}
	
	read_message(&rio_remote, buf, MAXLINE);
	if(!strcmp(buf, "ERROR\n")){
		printf("%s\n", buf);
		printf("Request failed: server error\n");
	}
	if(!strcmp(buf, "BEGINRESULTS\n")){
		printf("%s\n", buf);
		printf("Downloading Results...\n");
	}
	else{
		printf("Not recognized: %s\n", buf);
	}

	size_t payload_size;
	size_t response_bytes = 0;
	read_message(&rio_remote, buf, MAXLINE);
	printf("Payload size buf: %s\n", buf);
	payload_size = atoi(buf);
	printf("Payload size set to %ld\n", payload_size);
	strcpy(payload, "");
	while((response_bytes < payload_size)){
		read_message(&rio_remote, buf, MAXLINE);
		size_t bufsize = strlen(buf);
		response_bytes += bufsize;
		if(response_bytes < MAX_OBJECT_SIZE){
			strcat(payload, buf);
		}
	}
	printf("%s\n", payload);

	write_file(config->cache_path, "test_recording", payload, payload_size, 0);
	memset(payload, 0, sizeof(payload));

	printf("Download complete.\n");

	return;
}

void write_file(char* cache_path, char* model_md5, char* payload, size_t payload_size, int isModel){
	FILE* fp;
	char full_path[400];
	strcpy(full_path, cache_path);
	strcat(full_path, "/");
	if(isModel){
		strcat(full_path, "temp/");
		strcat(full_path, model_md5);
		strcat(full_path, "-model.pkl");
	}
	else{
		strcat(full_path, model_md5);
		strcat(full_path, "-result.pkl");
	}
	fp=fopen(full_path, "wb");
	fwrite(payload, 1, payload_size, fp);
	fclose(fp);
}

void get_data_from_file(char* cache_path, char* payload, char* model_md5, int isModel){
	FILE *fp;
	//char file_path[400];
	char full_path[400];
	strcpy(full_path, cache_path);
	strcat(full_path, "/");
	if(isModel){
		strcat(full_path, "temp/");
		strcat(full_path, model_md5);
		strcat(full_path, "-model.pkl");
	}
	else{
		strcat(full_path, model_md5);
		strcat(full_path, "-results.pkl");
	}
	printf("File path: %s\n", full_path);
	fp = fopen(full_path, "rb");
	if(NULL == fp){
		printf("File not found: %s\n", full_path);
		return;
	}
	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(payload, 1, fsize, fp);
	payload[fsize] = '\0';
	printf("Internal Payload: %s\n", payload);
	fclose(fp);
}

void send_model(){
	
}
