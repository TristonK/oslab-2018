#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>

struct Cpu{
    int ncli;    //Depth of pushcli nesting
    int intena;
};

struct task {};
struct spinlock {
    unsigned int locked;
    char *name;
    int cpu;
};
struct semaphore {};

#endif
