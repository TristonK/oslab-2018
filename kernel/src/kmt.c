#include <common.h>
#include <klib.h>
#include <devices.h>
#include "x86.h"
#include <limits.h>


//*******ariables
struct Cpu mycpu[16];
spinlock_t task_lk;
//***************

//lmitated xv6
void pushcli(){
    int eflags;
    eflags = get_efl();
    cli();
    if(mycpu[_cpu()].ncli == 0){
        mycpu[_cpu()].intena = eflags & FL_IF;
    }
    mycpu[_cpu()].ncli +=1;
}

void popcli(){
    if(get_efl() & FL_IF)
		panic("popcli - interruputible");
	if(--mycpu[_cpu()].ncli < 0)
		panic("popcli");
	if(mycpu[_cpu()].ncli == 0 && mycpu[_cpu()].intena)
        sti();
}

int holding(spinlock_t *lock){
    int ret;
    pushcli();
    ret = (lock->locked && lock->cpu ==_cpu());
    popcli();
    return ret;
}

_Context *kmt_context_save(_Event ev, _Context context){return NULL;}

_Context *kmt_context_switch(_Event ev, _Context context){return NULL;}

static void kmt_init(){
    kmt->spin_init(&task_lk,"task_lock");
    os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save); 
    os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch); 
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    return 0;
}
static void kmt_teardown(task_t *task){

}
static void kmt_spin_init(spinlock_t *lk, const char *name){
    lk -> name = name;
    lk -> locked = 0;
    lk -> cpu = -1;
}
static void kmt_spin_lock(spinlock_t *lk){
    pushcli();
    if(holding(lk)){
        panic("Has benn aquired");
    }
    while (_atomic_xchg(&lk->locked,1)){;}
    __sync_synchronize();
    lk -> cpu = _cpu();
}
static void kmt_spin_unlock(spinlock_t *lk){
    if(!holding(lk)){
        panic("why unlock unexist lock");
    }
    lk->cpu = -1;
    __sync_synchronize();
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
    popcli();
}
static void kmt_sem_init(sem_t *sem, const char *name, int value){

}
static void kmt_sem_wait(sem_t *sem){

}
static void kmt_sem_signal(sem_t *sem){

}



MODULE_DEF(kmt){
    .init = kmt_init,
    .create = kmt_create,
    .teardown = kmt_teardown,
    .spin_init = kmt_spin_init,
	.spin_lock = kmt_spin_lock,
	.spin_unlock = kmt_spin_unlock,
	.sem_init = kmt_sem_init,
	.sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal,
};