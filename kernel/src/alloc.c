#include <common.h>
#include <klib.h>

//***************** Variables ******************
static uintptr_t pm_start, pm_end;
uintptr_t alloc_lk;
struct run{
  struct run *next;
};


//****************** code ************************
static inline void cli(){
  asm volatile("cli");
}
static inline void sti(){
  asm volatile("sti");
}

void spin_lock(uintptr_t lk){
  cli();
  while(atomic_xchg(lk, 1)){}
}
void spin_unlock(){
   while(atomic_xchg(lk, 0)){}
   sti();
} 

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  alloc_lk = 0;
}

void *alloc_unsafe(size_t size){
  return NULL;
}
void free_unsafe(){}


static void *kalloc(size_t size) {
  spin_lock(&alloc_lk);
  void *ret = alloc_unsafe(size);
  spin_unlock(&alloc_lk);
  return ret;
}

static void kfree(void *ptr) {
  spin_lock(&alloc_lk);
  free_unsafe();
  spin_unlock(&alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
