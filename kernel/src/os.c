#include <common.h>
#include <klib.h>

extern void spin_lock(intptr_t *lk);
extern void spin_unlock(intptr_t *lk);
intptr_t lk;
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
  while (1) {
    spin_lock(&lk);
    int op = rand()%2;
    /*0: randomly free 
    * 1: randomly alloc
    * maxrandom num = 32768*/
    if(op){
      if(used_cnt==500){
        printf("you have alloc 500 blocks\n");
        for(int i=0;i<500;i++){
          pmm->free(my_test[i]);
          my_test[i]=0;
        }
        printf("you now free all the blocks\n");
      }
      int size_mode = rand()%10;
      /*
      * 90%: small size need to be alloc
      * 10%: big size need to be alloc
      */
      if(size_mode<=8){
        int small_order = rand()%11+2;
        int small_add = rand()%(1<<small_order);
        size_t small_size= (1<<small_order)+small_add;
        uintptr_t alloc_addr = pmm->alloc(small_size);
        if(!alloc_addr){
          for(int i=0;i<500;i++){
            pmm->free(my_test[i]);
            my_test[i]=0;
          }
          printf("you now free all the blocks\n");
        }
        else{
          for(int i = 0;i<=499;i++){
            if(!my_test[i]){
              my_test[i]=alloc_addr;
              used_cnt++;
              break;
            }
          }
        }
      }
      else{
        int big_order = rand()%5+10;
        int big_add = rand()%(1<<big_order);
        size_t big_size= (1<<big_order)+big_add;
        uintptr_t big_alloc_addr = pmm->alloc(big_size);
        if(!big_alloc_addr){
          for(int i=0;i<500;i++){
            pmm->free(my_test[i]);
            my_test[i]=0;
          }
          printf("you now free all the blocks\n");
        }
        else{
          for(int i = 0;i<=499;i++){
            if(!my_test[i]){
              my_test[i]=big_lloc_addr;
              used_cnt++;
              break;
            }
          }
        }
      }
    }
    else{
      if(used_cnt == 0){
        my_test[0] = pmm->alloc(2048);
        used_cnt++;
      }
      int free_index = rand()%500;
      while(!my_test[free_index]){
        free_index++;
        if(free_index==500){
          free_index = 0;
        }
      }
      pmm->free(my_test[free_index]);
      my_test[free_index]=0;
      used_cnt--;
    }
    spin_unlock(&lk);
    _yield();
  }
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
