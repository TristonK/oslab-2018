#include <common.h>
#include <klib.h>

extern void spin_lock(intptr_t *lk);
extern void spin_unlock(intptr_t *lk);
intptr_t lk;
extern intptr_t print_lk;
uintptr_t my_test[500];
int used_cnt;

static void os_init() {
  pmm->init();
  lk=0;
  used_cnt=0;
  for(int i=0;i<500;i++)
    my_test[i]=0;
  srand(uptime());
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]); _putc('\n');
}

static void os_run() {
  hello();
  _intr_write(1);
  void *t1,*t2,*t3,*t4,*t5,*t6,*t7;
  t1 = pmm ->alloc(13*1024);
  printf("hi siri\n");
  t2 = pmm -> alloc(17*1024);
  pmm->free(t2);
  t3 = pmm -> alloc(15*1024);
  t4 = pmm -> alloc (17244);
  t5 = pmm -> alloc (15222);
  pmm -> free (t1);
  pmm -> free (t4);
  t6 = pmm -> alloc (122222);
  pmm -> free(t6);
  t7 = pmm -> alloc(123);
  t1 = pmm -> alloc(32*1024);
  t2 = pmm -> alloc(16*1024);
  pmm -> free(t7);
  pmm -> free(t2);
  pmm -> free(t1);
  pmm -> free(t3);
  pmm -> free(t5);
  /*while (1) {
    spin_lock(&lk);
    int op = rand()%2;*/
    /*0: randomly free 
    * 1: randomly alloc
    * maxrandom num = 32768*/
    /*if(op){
      if(used_cnt==500){
        spin_lock(&print_lk);
        printf("you have alloc 500 blocks\n");
        spin_unlock(&print_lk);
        for(int i=0;i<500;i++){
          pmm->free((void*)my_test[i]);
          my_test[i]=0;
        }
        spin_lock(&print_lk);
        printf("you now free all the blocks\n");
        spin_unlock(&print_lk);
        //_yield();
      }
      int size_mode = rand()%10;*/
      /*
      * 90%: small size need to be alloc
      * 10%: big size need to be alloc
      */
      /*if(size_mode<=8){
        int small_order = rand()%11+2;
        int small_add = rand()%(1<<small_order);
        size_t small_size= (1<<small_order)+small_add;
        uintptr_t alloc_addr = (uintptr_t)pmm->alloc(small_size);
        if(!alloc_addr){
          for(int i=0;i<500;i++){
            if(my_test[i]){
              pmm->free((void*)my_test[i]);
              my_test[i]=0;
            } 
          } 
          used_cnt=0;
          spin_lock(&print_lk);
          printf("you now free all the blocks\n");
          spin_unlock(&print_lk);
        }
        alloc_addr = (uintptr_t)pmm->alloc(small_size);
        for(int i = 0;i<=499;i++){
          if(!my_test[i]){
            my_test[i]=alloc_addr;
            used_cnt++;
            break;
          }
        }
      }
      else{
        int big_order = rand()%5+10;
        int big_add = rand()%(1<<big_order);
        size_t big_size= (1<<big_order)+big_add;
        uintptr_t big_alloc_addr = (uintptr_t)pmm->alloc(big_size);
        if(!big_alloc_addr){
          for(int i=0;i<500;i++){
            if(my_test[i]){
              pmm->free((void *)my_test[i]);
              my_test[i]=0;
            }
          }
          used_cnt=0;
          spin_lock(&print_lk);
          printf("you now free all the blocks\n");
          spin_unlock(&print_lk);
        }
        big_alloc_addr = (uintptr_t)pmm->alloc(big_size);
        for(int i = 0;i<=499;i++){
          if(!my_test[i]){
            my_test[i]=big_alloc_addr;
            used_cnt++;
            break;
          }
        }
      }
    }
    else{
      if(used_cnt == 0){
        my_test[0] = (uintptr_t)pmm->alloc(2048);
        used_cnt++;
      }
      int free_index = rand()%500;
      while(!my_test[free_index]){
        free_index++;
        if(free_index==500){
          free_index = 0;
        }
      }
      pmm->free((void*)my_test[free_index]);
      my_test[free_index]=0;
      used_cnt--;
    }
    spin_unlock(&lk);
    //_yield();
  }*/
}

static _Context *os_trap(_Event ev, _Context *context) {
  return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
