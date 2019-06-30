#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
//#include <x86.h>

#define maxCpu _ncpu()

enum {
    NONE=0,RUNABLE,RUNNING,YIELD
};

struct Cpu{
    int ncli;    //Depth of pushcli nesting
    int intena; // interrupt enabled before pushcli?
};

struct task {
    int id;
    int state;
    int cpu_index;
    const char* name;
    _Context context;
    _Area stk;
    char stack[4096];
};
struct spinlock {
    intptr_t locked;
    const char *name;
    int cpu;
};
struct semaphore {
    const char* name;
    int value;
    spinlock_t lock;
    int task_id[32];
    task_t* wtask[32];
    int wait_pos;
};

typedef struct Handle{
    int seq;
    int event;
    handler_t handler;
    struct Handle *pre;
    struct Handle *next;
}handle;

/*struct handles{
    int size;
    struct handle *head;
};*/

#endif
