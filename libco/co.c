#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include "co.h"

#define MAX_CO 128
#if defined(__i386__)
  #define SP "%%esp"
#elif defined(__x86_64__)
  #define SP "%%rsp"
#endif

uint32_t thread_cnt;
func_t func_temp;
void* arg_temp;

struct co {
  uint32_t state;
  uint8_t stack[4096];
  jmp_buf buf;
  void* stack_backup;
};

struct co *current, coroutines[MAX_CO]; 

void co_init() {
  thread_cnt = 0;
  current = &coroutines[0];
  current->state=1;
}

struct co* co_start(const char *name, func_t func, void *arg) {
  thread_cnt++;
  func_temp = func;
  arg_temp = arg;
  int val = setjmp(coroutines[0].buf);
  if(val == 0){
    asm volatile("mov " SP ", %0; mov %1, " SP :
                 "=g"(coroutines[thread_cnt].stack_backup) :
                 "g"(coroutines[thread_cnt].stack + sizeof(coroutines[thread_cnt].stack)));
    coroutines[thread_cnt].state=1;
    current = &coroutines[thread_cnt];
    func_temp(arg_temp);
    current->state=0;
    longjmp(coroutines[0].buf,1);
    asm volatile("mov %0," SP : : "g"(coroutines[thread_cnt].stack_backup));
  }else{
    return &coroutines[thread_cnt];
  }
  //func(arg); // Test #2 hangs
  //return NULL;
}

void co_yield() {
  int val = setjmp(current->buf);
  if (val == 0) {
    int pick_num = rand()%(thread_cnt+1);
    while(!coroutines[pick_num].state){
      pick_num = rand()%(thread_cnt+1);
    }
    current = &coroutines[pick_num];
    longjmp(current->buf,1); 
  } else {
    // ???
  }
}

void co_wait(struct co *thd) {
  setjmp(current->buf);
  if(thd->state){
     int pick_num = rand()%(thread_cnt+1);
    while(!coroutines[pick_num].state){
      pick_num = rand()%(thread_cnt+1);
    }
    current = &coroutines[pick_num];
    longjmp(current->buf,1); 
  }
}

