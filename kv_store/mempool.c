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
    if (!m) return -1;
    if (size < 16) size =16;
    m -> block_size = size;
    m -> free_count = 0;
    m -> free_ptr = NULL;
    m -> pages = NULL;

    return mempool_expand(m);
}

int mempool_expand(mempool_t* m){
    if (!m) return -1;
    mempool_page_t* page = malloc(sizeof(mempool_page_t));
    if (!page) return -1;
    page -> mem = malloc(MEM_PAGE_SIZE);
    if (!page -> mem) {
        free(page);
        return -1;
    }

    page -> next = m -> pages;
    m -> pages = page;

    // 3. 将新页内的所有块链接到全局空闲链表
    char* ptr = page->mem;
    int blocks_in_page = MEM_PAGE_SIZE / m->block_size;

    for (int i = 0; i < blocks_in_page - 1; i++) {
        *(char**)ptr = ptr + m->block_size;
        ptr += m->block_size;
    }
    *(char**)ptr = m->free_ptr; // 新页的最后一个块指向原来的空闲链表头
    m->free_ptr = page->mem; // 更新全局空闲链表头为新页的第一个块
    m->free_count += blocks_in_page;

    return 0;
}

void mempool_destroy (mempool_t* m) {
    if (!m) return;
    mempool_page_t* page = m -> pages;
    while (page) {
        mempool_page_t* next = page -> next;
        free(page -> mem);
        free(page);
        page = next;
    }
    m -> pages = NULL;
    m -> free_ptr = NULL;
    m -> free_count = 0;
}

void* mempool_alloc (mempool_t* m) {
    if (!m || m -> free_count == 0) {
        if (mempool_expand(m) != 0) {
            return NULL; // 扩容失败，分配失败
        }
    }
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