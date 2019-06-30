#include <common.h>
#include <klib.h>
#include <devices.h>
#include "x86.h"
#include <limits.h>


//*******ariables
struct Cpu mycpu[16];
spinlock_t task_lk;
task_t *c_task[32];
task_t *runtask[16];
_Context store_cond[16];
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

_Context *kmt_context_save(_Event ev, _Context *context){
    //printf("save\n");
    if(runtask[_cpu()]==NULL){
        store_cond[_cpu()] = *context;
    }    else
    {
        runtask[_cpu()]->context = *context;
        if(runtask[_cpu()]->state==RUNNING)
            runtask[_cpu()]->state = RUNABLE;
    }
    //return context;
    return NULL;
}

_Context *kmt_context_switch(_Event ev, _Context *context){
    //printf("switch\n");
    int idx = (runtask[_cpu()]==NULL)?0:runtask[_cpu()]->id;
    //int idx_bak2= idx;
    if(runtask[_cpu()]==NULL){
        int flag = 0;
        for(;idx<32;idx++){
            if(c_task[idx]!=NULL &&c_task[idx]->cpu_index == _cpu() &&c_task[idx]->state== RUNABLE){
                flag = 1;
                break;
            }       
        }
        if(flag){
            runtask[_cpu()] = c_task[idx];
            runtask[_cpu()]->state = RUNNING;
            return &runtask[_cpu()]->context;
        } else{
            return &store_cond[_cpu()];
        }
        
    } else{
        int idx_bak = idx; 
        while (1){
            idx = (idx+1)%32;
            if(c_task[idx]!=NULL &&c_task[idx]->cpu_index == _cpu() && c_task[idx]->state== RUNABLE)
                break;
            if(idx == idx_bak)
                break;
        }
        if(idx != idx_bak){
            runtask[_cpu()] = c_task[idx];
            runtask[_cpu()]->state = RUNNING;
            return &runtask[_cpu()]->context;
        }else{
            //printf("no changeeeee\n");
            return &runtask[_cpu()]->context;
        }
        
    }
    //return NULL;
}

static void kmt_init(){
    kmt->spin_init(&task_lk,"task_lock");
    os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save); 
    os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
    //printf("min is %d and max is %d\n",-1,INT_MAX); 
    for(int i=0;i<32;i++){
        c_task[i] = NULL;
        //c_task[i]->id = -1;
        //c_task[i]->state = NONE;
    }
    for(int i=0;i<16;i++){
        mycpu[i].ncli = 0;
        mycpu[i].intena = 1;
    }
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    kmt->spin_lock(&task_lk);
    //printf("creating begin\n");
    int ret=-1;
    for(int i=0;i<32;i++){
        if(c_task[i]==NULL){
            task->id = i;
            ret = i;
            break;
        }
    }
    if(ret==-1)
        panic("The num of thread is too much");
    //printf("creating\n");
    task->state = RUNABLE;
    task->stk.start = task->stack;
    task->stk.end = task->stk.start+4096;
    task->cpu_index = ret % maxCpu;
    task->context = *(_kcontext(task->stk,entry,arg));
    c_task[ret] = task;
    //printf("end create\n");
    kmt->spin_unlock(&task_lk);
    return ret;
}
static void kmt_teardown(task_t *task){
    kmt->spin_lock(&task_lk);
    c_task[task->id]->state = NONE;
    c_task[task->id] = NULL;
    task->id = -1;
    pmm->free((void*)task);
    kmt->spin_unlock(&task_lk);
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
        printf("lk name is %s\n",lk->name);
        panic("why unlock unexist lock");
    }
    lk->cpu = -1;
    __sync_synchronize();
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
    popcli();
}


static void kmt_sem_init(sem_t *sem, const char *name, int value){
    sem -> name =name;
    sem -> value = value;
    kmt ->spin_init(&sem->lock,name);
    for(int i=0;i<32;i++)
        sem->task_id[i] = -1;
        sem->wait_pos = 0;
}

static void kmt_sem_wait(sem_t *sem){
    kmt->spin_lock(&sem->lock);
    //printf("waitt\n");
    sem -> value--;
    if(sem->value<0){
        runtask[_cpu()]->state = YIELD;
        int poss = sem->wait_pos;
        for(int i=0;i<32;i++){
            if(sem->task_id[poss]==-1){
                sem->task_id[poss] = runtask[_cpu()]->id;
                printf("wait %d\n",runtask[_cpu()]->id);
                break;
            }else{
                poss = (poss+1)%32;
            }
        }
        kmt->spin_unlock(&sem->lock);
        _yield();
    }
    else
        kmt->spin_unlock(&sem->lock);
}

static void kmt_sem_signal(sem_t *sem){
    kmt->spin_lock(&sem->lock);
    //printf("signal %s\n",sem->name);
    sem->value++;
    if(sem->value<=0){
        int poss = sem->wait_pos;
        sem->wait_pos = (poss+1)%32;
        int idd = sem->task_id[poss];
        sem -> task_id[poss] = -1;
        if(idd == -1){
            panic("no thread need");
        }
        printf("signal %d\n",idd);
        c_task[idd]->state = RUNABLE;
        //runtask[idd]->state = RUNABLE;
    }
    //printf("sig end\n");
    kmt->spin_unlock(&sem->lock);
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