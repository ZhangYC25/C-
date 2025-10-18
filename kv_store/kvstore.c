#include "kvstore.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define KVSTORE_MAX_TOKENS 128

const char* commands[]={
    "SET", "GET", "DEL", "MOD",
    "RSET", "RGET", "RDEL", "RMOD"
};
enum {
    KYS_CMD_START = 0,
    KYS_CMD_SET = KYS_CMD_START,
    KYS_CMD_GET,
    KYS_CMD_DEL,
    KYS_CMD_MOD ,
    KYS_CMD_COUNT
};

void* kvstore_malloc(size_t size){
    return malloc(size);
}
void kvstore_free(void* ptr){
    return free(ptr);
}


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

int kvstore_parser_protocol(struct conn_item* item, char** tokens, int count){
    if (item == NULL || tokens[0] == NULL || count == 0) return -1;
    int cmd = KYS_CMD_START;
    for (cmd = KYS_CMD_START;cmd < KYS_CMD_COUNT; cmd++) {
        if (strcmp(commands[cmd], tokens[0]) == 0) {
            break;
        }
    }

    char* msg = item -> wbuffer;
    memset(msg, 0, BUFFER_LENGTH);
    switch (cmd) {
        case KYS_CMD_SET:
            LOG("set\n");
            int ref_set = kvstore_array_set(tokens[1], tokens[2]);
            if (!ref_set) {
                snprintf(msg, BUFFER_LENGTH, "SUCESS");
            } else {
                snprintf(msg, BUFFER_LENGTH, "FAILED");
            }
            break;
        case KYS_CMD_GET:
            LOG("get\n");
            char* value = kvstore_array_get(tokens[1]);
            if (value) {
                snprintf(msg, BUFFER_LENGTH, "%s", value);
            } else {
                snprintf(msg, BUFFER_LENGTH, "NO EXITS");
            }
            break;
        case KYS_CMD_DEL:
            int ref_del = kvstore_array_del(tokens[1]);
            if (ref_del < 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
            } else if(ref_del == 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "SUCESS");
            } else {
                snprintf(msg, BUFFER_LENGTH, "NO EXITS");
            }
            LOG("del\n");
            break;
        case KYS_CMD_MOD:
            int ref_mod = kvstore_array_mod(tokens[1],tokens[2]);
            if (ref_mod < 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
            } else if(ref_mod == 0) {
                snprintf(msg, BUFFER_LENGTH, "%s", "SUCESS");
            } else {
                snprintf(msg, BUFFER_LENGTH, "NO EXITS");
            }
            LOG("mod\n");
            break;
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

    kvstore_parser_protocol(item, tokens, count);
    return 0;
}

#if 0
int main(){
#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL) 
    epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
    ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif
}
#endif