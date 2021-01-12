#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>

#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    int rio_cnt;               /* Unread bytes in internal buf */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

typedef struct sockaddr SA;

//constants
#define	MAXLINE	 8192
#define LISTENQ  1024

//error messages
void unix_error(char *msg);
void posix_error(int code, char *msg);

//memory allocation
void *Malloc(size_t size);
void Free(void *ptr);

//semaphores
void Sem_init(sem_t *sem, int pshared, unsigned int value);

//pthreads
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp, void * (*routine)(void *), void *argp);
void Pthread_join(pthread_t tid, void **thread_return);
void Pthread_detach(pthread_t tid);
pthread_t Pthread_self(void);
void Pthread_exit(void *retval);

//signals
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

//sockets
int Open_clientfd(char *hostname, char *port);
int open_clientfd(char *hostname, char *port);
int Open_listenfd(char *port);
int open_listenfd(char *port);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

//io
void Rio_writen(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_readinitb(rio_t *rp, int fd);
void rio_readinitb(rio_t *rp, int fd);
void Close(int fd);

#endif