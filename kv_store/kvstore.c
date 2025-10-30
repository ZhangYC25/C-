#include "kvstore.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define KVSTORE_MAX_TOKENS 128

const char* commands[]={
    "SET", "GET", "DEL", "MOD","COUNT",
    "RSET", "RGET", "RDEL", "RMOD","RCOUNT"
};
enum {
    KVS_CMD_START = 0,
    KVS_CMD_SET = KVS_CMD_START,
    KVS_CMD_GET,
    KVS_CMD_DEL,
    KVS_CMD_MOD ,
    KVS_CMD_COUNT,

    KVS_CMD_RSET,
	KVS_CMD_RGET,
	KVS_CMD_RDEL,
	KVS_CMD_RMOD,
	KVS_CMD_RCOUNT
};

void* kvstore_malloc(size_t size){
    return malloc(size);
}
void kvstore_free(void* ptr){
    return free(ptr);
}

#if ENABLE_RBTREE_KVENGINE
int rbtree_set(char *key, char *value, mempool_t* pool){
    return kvstore_rbtree_set(&Tree, key, value, pool);
}
char* rbtree_get(char *key){
    return kvstore_rbtree_get(&Tree, key);
}
int rbtree_del(char *key, mempool_t* pool){
    return kvstore_rbtree_del(&Tree, key, pool);
}
int rbtree_mod(char *key, char *value, mempool_t* pool){
    return kvstore_rbtree_mod(&Tree, key, value, pool);
}

int rbtree_count(void) {
	return kvstore_rbtree_count(&Tree);
}

#endif

int kvstore_split_token(char* msg, char** tokens) {
    if (msg == NULL || tokens == NULL) return -1;

    int idx = 0;
    char* token = strtok(msg, " ");
    while (token != NULL) {
        tokens[idx ++] = token;
        token = strtok(NULL, " ");
    }
    return idx;
}

int kvstore_parser_protocol(struct conn_item* item, char** tokens, int count, mempool_t* pool){
    if (item == NULL || tokens[0] == NULL || count == 0) return -1;
    int cmd = KVS_CMD_START;
    for (cmd = KVS_CMD_START;cmd < KVS_CMD_RCOUNT; cmd++) {
        if (strcmp(commands[cmd], tokens[0]) == 0) {
            break;
        }
    }

    char* msg = item -> wbuffer;
    char* key = tokens[1];
    char* value = tokens[2];

    memset(msg, 0, BUFFER_LENGTH);
    switch (cmd) {
        case KVS_CMD_SET:
            LOG("set\n");
            //int ref_set = kvstore_array_set(key, value);
            int ref_set = kvstore_array_set(tokens[1], tokens[2], pool);
            if (!ref_set) {
                snprintf(msg, BUFFER_LENGTH, "SUCESS");
            } else {
                snprintf(msg, BUFFER_LENGTH, "FAILED");
            }
            break;
        case KVS_CMD_GET:
            LOG("get\n");
            char* value = kvstore_array_get(tokens[1]);
            if (value) {
                snprintf(msg, BUFFER_LENGTH, "%s", value);
            } else {
                snprintf(msg, BUFFER_LENGTH, "NO EXITS");
            }
            break;
        case KVS_CMD_DEL:
            int ref_del = kvstore_array_del(tokens[1], pool);
            if (ref_del < 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
            } else if(ref_del == 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "SUCESS");
            } else {
                snprintf(msg, BUFFER_LENGTH, "NO EXITS");
            }
            LOG("del\n");
            break;
        case KVS_CMD_MOD:
            int ref_mod = kvstore_array_mod(tokens[1],tokens[2], pool);
            if (ref_mod < 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
            } else if(ref_mod == 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "SUCESS");
            } else {
                snprintf(msg, BUFFER_LENGTH, "NO EXITS");
            }
            LOG("mod\n");
            break;

        case KVS_CMD_COUNT: {
			int count = kvstore_array_count();
			if (count < 0) {  // server
				snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
			} else {
				snprintf(msg, BUFFER_LENGTH, "%d", count);
			}
			break;
		}

        // rbtree
		case KVS_CMD_RSET: {
            printf("here\n");
            //int res = rbtree_set(key, value, pool);
			int res = rbtree_set(tokens[1], tokens[2], pool);
			if (!res) {
				snprintf(msg, BUFFER_LENGTH, "SUCCESS");
			} else {
				snprintf(msg, BUFFER_LENGTH, "FAILED");
			}
			break;
		}
		case KVS_CMD_RGET: {
			char *val = rbtree_get(key);
			if (val) {
				snprintf(msg, BUFFER_LENGTH, "%s", val);
			} else {
				snprintf(msg, BUFFER_LENGTH, "NO EXIST");
			}
			break;
		}
		case KVS_CMD_RDEL: {
			int res = rbtree_del(key, pool);
			if (res < 0) {  // server
				snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
			} else if (res == 0) {
				snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
			} else {
				snprintf(msg, BUFFER_LENGTH, "NO EXIST");
			}
			break;
		}
		case KVS_CMD_RMOD: {
			int res = rbtree_mod(key, value, pool);
			if (res < 0) {  // server
				snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
			} else if (res == 0) {
				snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
			} else {
				snprintf(msg, BUFFER_LENGTH, "NO EXIST");
			}
			break;
		}

		case KVS_CMD_RCOUNT: {
			int count = rbtree_count();
			if (count < 0) {  // server
				snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
			} else {
				snprintf(msg, BUFFER_LENGTH, "%d", count);
			}
			break;
		}

        default:
            assert(0);
    }
}

//OP Key Value
int kvstore_request(struct conn_item* item){
    LOG("recv: %s\n", item->rbuffer);
    char* msg = item -> rbuffer;
    char* tokens[KVSTORE_MAX_TOKENS];

    int count = kvstore_split_token(msg, tokens);

    int idx = 0;

    for (idx = 0;idx < count;idx ++) {
        LOG("idx: %s\n", tokens[idx]);
    }

    mempool_t m;
    mempool_init(&m, PARTSIZE);
    
    kvstore_parser_protocol(item, tokens, count, &m);
    return 0;
}


int init_kvengine(void){
#if ENABLE_ARRAY_KVENGINE

#endif

#if ENABLE_RBTREE_KVENGINE
    return kvstore_rbtree_create(&Tree);

#endif
}

#if 1
int main(){
    init_kvengine();
#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL) 
    epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
    ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif
}
#endif