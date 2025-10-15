#ifndef __KVSTORE_H__
#define __KVSTORE_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_LENGTH	1024
typedef int (*RCALLBACK)(int fd);

//fd, buffer, callback
struct conn_item{
	int fd;

	char rbuffer[BUFFER_LENGTH];
	int rlen;
	char wbuffer[BUFFER_LENGTH];
	int wlen;

	union {
		RCALLBACK accept_callback;
		RCALLBACK recv_callback;
	} recv_t;
	RCALLBACK send_callback;
};

int epoll_entry(void);
int ntyco_entry(void);

int kvstore_request(struct conn_item* item);

#define NETWORK_EPOLL 0
#define NETWORK_NTYCO 1
#define NETWORK_IOURING 2

#define ENABLE_ARRAY_KVENGINE 1
#define ENABLE_NETWORK_SELECT NETWORK_NTYCO

#if ENABLE_ARRAY_KVENGINE
void* kvstore_malloc(size_t size);
void kvstore_free(void* ptr);
struct kvs_array_item{
    char* key;
    char* value;
};
int kvstore_array_set(char* key, char* value);
char* kvstore_array_get(char* key);
#endif

#endif