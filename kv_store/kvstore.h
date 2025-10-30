#ifndef __KVSTORE_H__
#define __KVSTORE_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_LENGTH	1024
#define MEM_PAGE_SIZE 4096
#define PARTSIZE 16

typedef int (*RCALLBACK)(int fd);

//#define ENABLE_LOG 0

#ifdef ENABLE_LOG
#define LOG(_fmt, ...) fprintf(stdout, "[%s: %d]: %s"_fmt,__FILE__, __LINE__, __VAR_ARGS__)
#else
#define LOG(_fmt, ...)
#endif

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
//内存池
typedef struct mempool_page_s{
	char* mem;          //整块的指针
	struct mempool_page_s* next; //下一个页
} mempool_page_t;
typedef struct mempool_s {
    int block_size;     //每个的大小
    int free_count;     //可分配的数量
    char* free_ptr;     //下一块在哪里
	mempool_page_t* pages; //内存池中的第一个页
} mempool_t;

int mempool_init(mempool_t *m, int size);
int mempool_expand(mempool_t* m);
void mempool_destroy(mempool_t* m);
void* mempool_alloc(mempool_t* m);
void* mempool_free(mempool_t* m, void *ptr);

int epoll_entry(void);
int ntyco_entry(void);

int kvstore_request(struct conn_item* item);

#define NETWORK_EPOLL 0
#define NETWORK_NTYCO 1
#define NETWORK_IOURING 2

#define ENABLE_ARRAY_KVENGINE 1
#define ENABLE_RBTREE_KVENGINE 1

#define ENABLE_NETWORK_SELECT NETWORK_NTYCO

#if ENABLE_ARRAY_KVENGINE
void* kvstore_malloc(size_t size);
void kvstore_free(void* ptr);
struct kvs_array_item{
    char* key;
    char* value;
};
int kvstore_array_set(char* key, char* value, mempool_t* pool);
char* kvstore_array_get(char* key);
int kvstore_array_del(char* key, mempool_t* pool);
int kvstore_array_mod(char* key, char* value, mempool_t* pool);
int kvstore_array_count(void);
#endif

#if ENABLE_RBTREE_KVENGINE
//rbtree
typedef struct _rbtree rbtree_t;

extern rbtree_t Tree;

int kvstore_rbtree_create(rbtree_t *tree);
void kvstore_rbtree_destory(rbtree_t *tree);
int kvstore_rbtree_set(rbtree_t *tree, char *key, char *value, mempool_t*);
char* kvstore_rbtree_get(rbtree_t *tree, char *key);
int kvstore_rbtree_del(rbtree_t *tree, char *key, mempool_t* pool);
int kvstore_rbtree_mod(rbtree_t *tree, char *key, char *value, mempool_t* pool);
int kvstore_rbtree_count(rbtree_t *tree);

#endif

#endif