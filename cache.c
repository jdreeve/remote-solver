#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "cache.h"
#include "config.h"

void cache_populate(cache* cache, char* cache_path){
	DIR *d;
	struct dirent *dir;
	char file_path[400];
	char md5_buf[50];
	FILE *fp;
	char data[MAX_OBJECT_SIZE];
	size_t file_size;

	d = opendir(cache_path);

	if(d){
		while((dir = readdir(d)) != NULL){
			//create file path
			strcpy(file_path, cache_path);
			strncat(file_path, "/", strlen("/"));
			strncat(file_path, dir->d_name, 200);
			if(DEBUG){
				printf("File path created:%s\n", file_path);
			}

			//get md5 from file name (format: md5-filetype.pkl, filetype should always be results here)
			if(strstr(file_path, "results")){
				strncpy(md5_buf, dir->d_name, MD5_SIZE);
			}
			else{
				continue;
			}
			fp = fopen(file_path, "rb");
			fseek(fp, 0, SEEK_END);
			size_t fsize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			file_size = fread(data, 1, fsize, fp);
			cache_add(cache, data, md5_buf, file_size);
			fclose(fp);
		}
	}

	closedir(d);
	return;
}

cache* cache_create(){
    cache* list = (cache*)Malloc(sizeof(cache));
    if (list != NULL){
        list->head = NULL;
        list->tail = NULL;
        list->count = 0;
	list->read_count = 0;
        list->cache_bytes = 0;
        Sem_init(&list->mutex, 0, 1);
        Sem_init(&list->writelock, 0, 1);
    }//if
    return list;
}//cache_create

void cache_compute_results(cache* list, char* model_md5){
	node* node = NULL;
	node = cache_search(list, model_md5);
	if (NULL==node){
		printf("Error: node for %s not found\n", model_md5);
		return;
	}
	
	//confirm model pickle exists
	//call python component on model pickle
	//confirm results pickle exists
}

void cache_destroy(cache* list){
	fflush(stdout);
	if (list != NULL && list->head != NULL){
            node* current_node = list->head;
            while(current_node->next != NULL){
                node* next_node = current_node->next;
                free_node(current_node);
                current_node=next_node;
        }//while
        free_node(current_node);
    }//if
    /* Free listueue structure */
    Free(list);
}//cache_destroy

bool cache_add(cache* list, char* results_pickle, char* model_md5, size_t bytes){
    node *new_node;

    if(list==NULL){
        return false;
    }//if

    new_node = (node*)Malloc(sizeof(node));
    if(new_node==NULL){
	    printf("Malloc error\n");
    return false;
    }//if

    new_node->model_md5 = (char*)Malloc((strlen(model_md5)+1) * sizeof(char));
    new_node->results_pickle = (char*)Malloc((strlen(results_pickle)+1) * sizeof(char));
	new_node->model_pickle = NULL;
    strcpy(new_node->model_md5, model_md5);
    strcpy(new_node->results_pickle, results_pickle);
    new_node->node_bytes = bytes;
    new_node->timestamp = time(NULL);

    if(NULL == new_node->model_md5|| NULL == new_node->results_pickle){
        return false;
    }

    if (list->count==0){//insert node at head, empty list
        list->tail = new_node;
    }//if
    new_node->next = list->head;
    new_node->previous = NULL;
    list->head = new_node;
    list->count++;
    printf("New list count: %d\n", list->count);
    return true;
}

bool cache_delete(cache* list){
    if (list == NULL){//bad pointer
        return false;
    }//if
    node* doomed_node;
    doomed_node = get_least_recently_used_node(list);
    if (list->head==doomed_node){//if node is head
        list->head = list->head->next;
	if (doomed_node->next != NULL){//if there are any other nodes
	    doomed_node->next->previous = NULL;
	}//if
    }//if
    else if (list->tail==doomed_node){//if node is tail
        list->tail = list->tail->previous;
	if (doomed_node->previous != NULL){//if there are any other nodes
	    doomed_node->previous->next = NULL;
	}//if
    }//if
    else {//if node is mid-list
        doomed_node->previous->next = doomed_node->next;
	doomed_node->next->previous = doomed_node->previous;
    }//else

    /* TODO reduce list data use by size of doomed node data */

    free_node(doomed_node);
    list->count--;
    return true;
}//cache_delete

node* get_least_recently_used_node(cache* list){
    node* iterator = list->head;
    node* least_node = iterator;

    while(iterator->next != NULL){
        iterator = iterator->next;
        if(iterator->timestamp < least_node->timestamp){
	    least_node = iterator;
	}
    }
    return iterator;
}

void free_node(node* node){
    Free(node->model_pickle);
    Free(node->results_pickle);
    Free(node->model_md5);
    Free(node);
}

void cache_print(cache *list){
    if (list==NULL || list->head==NULL){
        return;
    }
    node *iterator = list->head;
    while (iterator != NULL){
        printf("%s\n",iterator->model_md5);
	iterator = iterator->next;
    }
    printf("\n");
    return;
}

node* cache_search(cache* list, char* model_md5){
    if (list==NULL || list->count == 0){
        return NULL;
    }//if
    node* iterator = list->head;
    if(iterator != NULL){
    }
    while( (iterator != NULL) && (strcmp(iterator->model_md5, model_md5)) ){
        iterator = iterator->next;
    }//while

	if(iterator == NULL) return NULL;
        if(!strcmp(iterator->model_md5, model_md5)){
	    return iterator;
	}
    return NULL;
}
