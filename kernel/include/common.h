#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
//#include <x86.h>

struct Cpu{
    int ncli;    //Depth of pushcli nesting
    int intena;
};

struct task {};
struct spinlock {
    intptr_t locked;
    const char *name;
    int cpu;
};
struct semaphore {};

#endif
