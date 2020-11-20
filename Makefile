CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: remote-solver

remote-solver: remote-solver.o csapp.o cache.o config.o fetch.o server.o
	$(CC) $(CFLAGS) remote-solver.o csapp.o cache.o config.o fetch.o server.o -o remote-solver $(LDFLAGS)
	$(CC) $(CFLAGS) -c csapp.c

remote-solver.o: remote-solver.c csapp.h
	$(CC) $(CFLAGS) -c remote-solver.c

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

fetch.o: fetch.c fetch.h
	$(CC) $(CFLAGS) -c fetch.c

server.o: server.c server.h
	$(CC) $(CFLAGS) -c server.c

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

cache: cache.c cache.h
	$(CC) $(CFLAGS) -c cache.c

clean:
	rm -f *~ *.o remote-solver core *.tar *.zip *.gzip *.bzip *.gz

