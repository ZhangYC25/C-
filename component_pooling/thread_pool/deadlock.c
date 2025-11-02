#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>

#include "deadlock.h"


typedef int (*pthread_mutex_look_t)(pthread_mutex_t*);
pthread_mutex_look_t pthread_mutex_lock_f = NULL;

typedef int (*pthread_mutex_unlook_t)(pthread_mutex_t*);
pthread_mutex_unlook_t pthread_mutex_unlock_f = NULL;


typedef int (*pthread_create_t)(pthread_t *thread, 
const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);
pthread_create_t pthread_create_f = NULL;

int pthread_create(pthread_t *thread, 
const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg){

    pthread_create_f(thread, attr, (*start_routine), arg);
}


int pthread_mutex_lock(pthread_mutex_t* mutex){
    pthread_mutex_lock_f(mutex);
    printf("lock success: %ld\n",pthread_self());
}

int pthread_mutex_unlock(pthread_mutex_t* mutex){
    pthread_mutex_unlock_f(mutex);
    printf("unlock success: %ld\n",pthread_self());
}



void init_hook(){
    if (!pthread_mutex_lock_f) 
        pthread_mutex_lock_f = dlsym(RTLD_NEXT,"pthread_mutex_lock");

    if (!pthread_mutex_unlock_f) 
        pthread_mutex_unlock_f = dlsym(RTLD_NEXT,"pthread_mutex_unlock");

    if (!pthread_create_f) 
        pthread_create_f = dlsym(RTLD_NEXT, "pthread_create");

}

pthread_mutex_t t1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t t2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t t3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t t4 = PTHREAD_MUTEX_INITIALIZER;

void* t1_cb(void*){
    pthread_mutex_lock(&t1);
    sleep(1);
    pthread_mutex_lock(&t2);

    pthread_mutex_unlock(&t2);

    pthread_mutex_unlock(&t1);
}

void* t2_cb(void*){
    pthread_mutex_lock(&t2);
    sleep(1);
    pthread_mutex_lock(&t3);

    pthread_mutex_unlock(&t3);
    pthread_mutex_unlock(&t2);
}

void* t3_cb(void*){
    pthread_mutex_lock(&t3);
    sleep(1);
    pthread_mutex_lock(&t4);
    pthread_mutex_unlock(&t4);
    pthread_mutex_unlock(&t3);
}

void* t4_cb(void*){
    pthread_mutex_lock(&t4);
    sleep(1);
    pthread_mutex_lock(&t1);

    pthread_mutex_unlock(&t1);
    pthread_mutex_unlock(&t4);
}

int main(){
#if 0
    init_hook();
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;
    pthread_t t4;

    pthread_create(&t1, NULL, t1_cb, NULL);
    pthread_create(&t2, NULL, t2_cb, NULL);
    pthread_create(&t3, NULL, t3_cb, NULL);
    pthread_create(&t4, NULL, t4_cb, NULL);


    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    return 0;
#endif
    tg = (struct task_graph*)malloc(sizeof(struct task_graph));
    tg -> num = 0;

    struct source_type v1;
    v1.id = 1;
    v1.type = PROCESS;
    add_vertex(v1);

    struct source_type v2;
    v2.id = 2;
    v2.type = PROCESS;
    add_vertex(v2);

    struct source_type v3;
    v3.id = 3;
    v3.type = PROCESS;
    add_vertex(v3);

    struct source_type v4;
    v4.id = 4;
    v4.type = PROCESS;
    add_vertex(v4);

    struct source_type v5;
    v5.id = 5;
    v5.type = PROCESS;
    add_vertex(v5);

    add_edge(v1,v2);
    add_edge(v2,v3);
    add_edge(v3,v4);
    add_edge(v4,v5);
    add_edge(v4,v2);

    search_for_cycle(search_vertex(v1));
}