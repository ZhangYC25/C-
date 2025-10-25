# include <stdio.h>
# include <stdlib.h>
# include "kvstore.h"

/*
typedef struct mempool_s {
    int block_size;     //每个的大小
    int free_count;     //可分配的数量
    char* free_ptr;     //下一块在哪里
    char* mem;          //整块的指针
} mempool_t;
*/

int mempool_init (mempool_t *m, int size) {
    if(!m) return -1;

    if (size < 16) size = 16;
    m -> block_size = size;

    m -> mem = malloc(MEM_PAGE_SIZE);
    if (!m -> mem) return -1;
    m -> free_ptr = m -> mem;
    m -> free_count = MEM_PAGE_SIZE / size;

    int i = 0;
    char* ptr = m -> free_ptr;
    for (i = 0;i < m -> free_count;i++) {
        *(char**)ptr = ptr + size;
        ptr += size;
    }
    *(char**)ptr = NULL;
    return 0;
}

void mempool_destroy (mempool_t* m) {
    if (!m || m -> mem) return;
    free(m -> mem);
}

void* mempool_alloc (mempool_t* m) {
    if (!m || m -> free_count == 0) return NULL;
    //开辟一个地址即freeptr指向的地址
    void* ptr = m -> free_ptr;
    //把 该地址存储 的下一个 内存地址 重新赋值给 freeptr
    m -> free_ptr = *(char**)ptr;
    m -> free_count --;
    return ptr;
}

void* mempool_free (mempool_t* m, void *ptr) {
    //把 该地址 保存的 下一个内存地址 指向 freeptr
    *(char**)ptr = m -> free_ptr;
    //把 freeptr 重置成当前释放节点
    m -> free_ptr = ptr;
    m -> free_count ++;
}

/*
int main () {
    mempool_t m;
    mempool_init(&m, 32);

    void* p1 = mempool_alloc(&m);
    printf("alloc  p1: %p\n",p1);

    void* p2 = mempool_alloc(&m);
    printf("alloc  p2: %p\n",p2);

    void* p3 = mempool_alloc(&m);
    printf("alloc  p3: %p\n",p3);

    void* p4 = mempool_alloc(&m);
    printf("alloc  p4: %p\n",p4);

    mempool_free(&m, p2);

    void* p5 = mempool_alloc(&m);
    printf("alloc  p5: %p\n",p5);

    return 0;
}
*/