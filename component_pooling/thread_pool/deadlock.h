#ifndef _DEADLICK_H_
#define _DEADLOCK_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
//有向图
typedef unsigned long int uint64;


#define MAX		100

enum Type {PROCESS, RESOURCE};

struct source_type {

	uint64 id;
	enum Type type;

	uint64 lock_id;
	int degress;
};

struct vertex {

	struct source_type s;
	struct vertex *next;

};

struct task_graph {

	struct vertex list[MAX];
	int num;

	struct source_type locklist[MAX];
	int lockidx; //

	pthread_mutex_t mutex;
};

struct vertex *create_vertex(struct source_type type);
int search_vertex(struct source_type type);
void add_vertex(struct source_type type);
int add_edge(struct source_type from, struct source_type to);
int verify_edge(struct source_type i, struct source_type j);
int remove_edge(struct source_type from, struct source_type to);
void print_deadlock(void);
int DFS(int idx);
int search_for_cycle(int idx);


#endif