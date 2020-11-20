#include "cache.h"
#include "server.h"
#include "csapp.h"

server_resources* server_resources_create(){
	server_resources* myself = (server_resources*)Malloc(sizeof(server_resources));
	if(NULL != myself){
		myself->config = NULL;
		myself->connfd = NULL;
	}
	return myself;
}
void run_solver_server(config_data* config){
	int listenfd;
	int *connfd;
	struct sockaddr_in client_addr;

	printf("Solver server running!\n");
	socklen_t client_len = sizeof(struct sockaddr_in);
	pthread_t tid;
	pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

	Signal(SIGPIPE, sigpipe_handler);

	listenfd = Open_listenfd(config->listen_port);

	while(1){
	connfd = Malloc(sizeof(int));
	*connfd = Accept(listenfd, (SA *) &client_addr, &client_len);
	printf("Connfd accepted, value %d\n", *connfd);
	server_resources* server_resources = server_resources_create();
	server_resources->config = config;
	server_resources->connfd = connfd;
	server_resources->cache_mutex = &cache_mutex;
	server_resources->file_mutex = &file_mutex;
	printf("Server resources assigned\n");
	Pthread_create(&tid, NULL, server_thread, server_resources);
	Pthread_detach(tid);
	}
}

void* server_thread(void* server_rsc){
	server_resources* this_server_resources = (server_resources*)server_rsc;
	int this_connfd = *this_server_resources->connfd;
	unsigned this_tid = Pthread_self();

	Signal(SIGPIPE, sigpipe_handler);

	if(DEBUG){
		printf("Thread created. fd = %d\t tid = %u\n", this_connfd, this_tid);
		fflush(stdout);
	}

	serve(this_server_resources);

	Free(this_server_resources);
	Close(this_connfd);
	printf("Server thread closing. fd = %d\t tid = %u\n", this_connfd, this_tid);
	Pthread_exit(NULL);
	return NULL;
}

void serve(server_resources* server_resources){
	char buf[MAXLINE];
	char this_model_md5[33];
	char payload[MAX_OBJECT_SIZE];
	rio_t rio_client;

	Rio_readinitb(&rio_client, *server_resources->connfd);
	Rio_readlineb(&rio_client, buf, MAXLINE);
	printf("%s", buf);
	if(!strcmp(buf, "CACHEQUERY\n")){
		printf("Cache query detected!\n");
		send_message(server_resources->connfd, "CACHEQUERY detected\n");
		printf("Server read %s", buf);
		fflush(stdout);
		node* node = cache_search(server_resources->config->cache, buf);
		if(node==NULL){
			strcpy(buf, "NOTFOUND\n");
		}
		else{
			strcpy(buf, "FOUND\n");
		}
		send_message(server_resources->connfd, buf);
		printf("sent: %s", buf);
	}
	if(!strcmp(buf, "REQUEST\n")){
		printf("Request detected!\n");
		send_message(server_resources->connfd, "REQUEST detected\n");
		read_message(&rio_client, buf, MAXLINE);
		printf("Received model md5 %s", buf);
		strncpy(this_model_md5, buf, 32);
		node* node=NULL;// = cache_search(server_resources->config->cache, this_model_md5);
		if(NULL==node){
			printf("md5 %s not cached\n", this_model_md5);
			send_message(server_resources->connfd, "NOTCACHED\n");
			printf("sent: NOTCACHED\n");

			printf("Downloading model pickle from client\n");
			size_t payload_size;
			size_t response_bytes = 0;
			read_message(&rio_client, buf, MAXLINE);
			payload_size = atoi(buf);
			printf("Payload size set to %ld\n", payload_size);
			strcpy(payload, "");
			while((response_bytes < payload_size)){
				read_message(&rio_client, buf, MAXLINE);
				size_t bufsize = strlen(buf);
				response_bytes+=bufsize;
				if(response_bytes<MAX_OBJECT_SIZE){
					strcat(payload, buf);
				}
			}
			printf("Model pickle downloaded from client\n");
			printf("%s\n", payload);

			pthread_mutex_lock(server_resources->file_mutex);
			write_file(server_resources->config->cache_path, this_model_md5, payload, payload_size, 1);
			pthread_mutex_unlock(server_resources->file_mutex);

			fork_exec_gillespy2(server_resources->config->cache_path, this_model_md5);
			//payload = get_data_from_file(server_resources->config->cache_path, payload, this_model_md5, 0);
			//pthread_mutex_lock(server_resources->cache_mutex);
			//cache_add(server_resources->config->cache, payload, this_model_md5, sizeof(payload);
			//pthread_mutex_unlock(server_resources->cache_mutex);
		}
		else{
			printf("md5 %s found in cache\n", this_model_md5);
			send_message(server_resources->connfd, "CACHED\n");
			read_message(&rio_client, buf, MAXLINE);
		}
		printf("Sending Results object\n");
	        //send Results object
		node = cache_search(server_resources->config->cache, this_model_md5);
		if(NULL == node){
			send_message(server_resources->connfd, "ERROR\n");
		}
		else{
			send_message(server_resources->connfd,"BEGINRESULTS\n");
		}
		printf("If/else over\n");
		fflush(stdout);
		pthread_mutex_lock(server_resources->file_mutex);
		get_data_from_file(server_resources->config->cache_path, payload, this_model_md5, 0);
		pthread_mutex_unlock(server_resources->file_mutex);
		printf("Results loaded from file: %s\n", payload);
		fflush(stdout);
		printf("Payload prepared: %s\n", payload);
		size_t payload_size = strlen(payload);
		char payload_size_str[20];
		sprintf(payload_size_str, "%ld\n", payload_size);
		printf("Payload size string written: %s\n", payload_size_str);
		send_message(server_resources->connfd, payload_size_str);
		printf("Payload size sent\n");
		strcat(payload, "\n");
		send_message(server_resources->connfd, payload);
		printf("Payload sent\n");
	}
}

void fork_exec_gillespy2(char* cache_path, char* model_md5){
	printf("Fork-exec to gillespy2 goes here\n");
	fflush(stdout);
	//fork
	//child exec gillespy2 script to run model md5
	//waitpid
	return;
}
