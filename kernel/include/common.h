#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
//#include <x86.h>

struct Cpu{
    int ncli;    //Depth of pushcli nesting
    int intena; // interrupt enabled before pushcli?
};

struct task {
    const char* name;
    _Context context;
    char stack[4096];
};
struct spinlock {
    intptr_t locked;
    const char *name;
    int cpu;
};
struct semaphore {
    const char* name;
    int count;
    spinlock_t lock;
    task_t tasks[32];
};

typedef struct Handle{
    int seq;
    int event;
    handler_t handler;
    struct handle *pre;
    struct handle *next;
}handle;

/*struct handles{
    int size;
    struct handle *head;
};*/

#endif
