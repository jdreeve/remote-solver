/* Implements web cache as dually linked list sorted by timestamp.
 *
 * 
 */
#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdbool.h>
#include "csapp.h"
#include <semaphore.h>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MD5_SIZE 32

typedef struct ELE {
    time_t timestamp;
    char* model_pickle;
    char* results_pickle;
    char* model_md5;
    size_t node_bytes;
    struct ELE *next;
    struct ELE *previous;
} node;

/* Queue structure */
typedef struct {
    node *head;  /* Linked cache of elements */
    node *tail;
    int count;
    int read_count;
    size_t cache_bytes;
    sem_t mutex;
    sem_t writelock;
} cache;

/************** Operations on cache ************************/

/*
  Create empty cache.
  Return NULL if could not allocate space.
*/
cache *cache_create();


void cache_populate(cache* cache, char* cache_path);
/*
 * Free all dynamically allocated data in a node, then free the node itself
*/
void free_node(node* node);
/*
  Free all bytes used by cache.
  No effect if list is NULL
*/
void cache_destroy(cache *list);

/*
 * Returns pointer to LRU node.
*/
node* get_least_recently_used_node(cache* cache);

/*
 * Create a node and insert it into the cache in alphanumeric sort order using strcmp
 * Return true if successful.
 * Return false if cache is NULL or could not allocate space.
 */
bool cache_add(cache *list, char* results_pickle, char* model_md5, size_t bytes);

void cache_compute_results(cache* list, char* model_md5);


/*
 * Deletes the least recently accessed node (by timestamp) from the list.
 * Return true if successful.
 * Return false if list is NULL or empty.
*/
bool cache_delete(cache *list);

/*
 * Prints each element in the list on its own line.
 */
void cache_print(cache *list);

/*
 * Return pointer to node with matching host and path.
 * Returns NULL if list is NULL or empty.
 */
node* cache_search(cache *list, char* model_md5);

#endif
