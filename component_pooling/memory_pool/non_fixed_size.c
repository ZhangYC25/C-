# include <stdio.h>
# include <stdlib.h>

#define MEM_PAGE_SIZE 152

typedef struct mem_node_s{
    //用char*是因为 char* 占一字节，可以用于计算；同样uint_8也是，而且更规范
    //而void* 不能用于计算，因为不知道它到底占用多大的内存，无法进行 +- 操作
    char* free_ptr;
    char* end;
    struct mem_node_s* next;
} mem_node_t;

typedef struct mem_pool_s{
    struct mem_node_s* first;
    //struct mem_node_s* current;
    int max; //page size
} mem_pool_t;

//初始化的是 内存池
int memory_init(mem_pool_t* m, int size){
    if (!m) return -1;
    
    void* addr = malloc(size);
    mem_node_t* node = (mem_node_t*)addr;
    node -> free_ptr = (char*)addr + sizeof(mem_node_t);
    node -> end = (char*)addr + size;
    node -> next = NULL;

    m -> first = node;
    //m-> current = node;
    m -> max = size;
    return 0;
}

void* memory_alloc(mem_pool_t* m, int size){
    void* addr = m -> first;
    mem_node_t* node = (mem_node_t*)addr;

    while(node){
        if (size <= (node -> end  - node -> free_ptr)){
            char* ptr = node -> free_ptr;
            node -> free_ptr += size;
            return ptr;
        }
        node = node -> next;
    }
    addr = malloc(m -> max);
    node = (mem_node_t*)addr;

    node -> free_ptr = (char*)addr + sizeof(mem_node_t);
    node -> end = (char*)addr + size;
    node -> next = m -> first;

    //m -> current = node;
    m -> first = node;

    char* ptr = node -> free_ptr;
    node -> free_ptr += size;
    return ptr;
}

void memory_destory(mem_pool_t* m){
    if (!m) return;
    while(m->first){
        mem_node_t* node = m -> first;
        m -> first = node -> next;
        //free(void*) 但是可以隐式转换
        //也可以用void*来接收m->first
        free(node);
    }
}

int main() {

	
	mem_pool_t m;

	memory_init(&m, MEM_PAGE_SIZE);


	void *p1 = memory_alloc(&m, 16);
	printf("1: memory_alloc_alloc: %p\n", p1);

	void *p2 = memory_alloc(&m, 32);
	printf("2: memory_alloc_alloc: %p\n", p2);

	void *p3 = memory_alloc(&m, 64);
	printf("3: memory_alloc_alloc: %p\n", p3);

	void *p4 = memory_alloc(&m, 128);
	printf("4: memory_alloc_alloc: %p\n", p4);

	memory_destory(&m);
}