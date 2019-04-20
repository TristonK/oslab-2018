#include <common.h>
#include <klib.h>

//***************** Variables ******************
static uintptr_t pm_start, pm_end;
intptr_t alloc_lk;
typedef struct run{
  int state;
  uintptr_t begin_addr,end_addr;
  uintptr_t size;
  struct run *next;
  struct run *prev;
}kblock;
typedef struct kmem{
  kblock *head;
  int size;
}kmem;
kmem freelist;
kmem runlist;

//****************** code ************************
static inline void cli(){
  asm volatile("cli");
}
static inline void sti(){
  asm volatile("sti");
}

void spin_lock(intptr_t *lk){
  cli();
  while(_atomic_xchg(lk, 1)){}
}
void spin_unlock(intptr_t *lk){
   while(_atomic_xchg(lk, 0)){}
   sti();
} 

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  printf("heap start at 0x%x\n",pm_start);
  printf("heap end at 0x%x\n",pm_end);
  printf("you could use %d space\n",pm_end-pm_start);
  alloc_lk = 0;
  kblock block;
  block.begin_addr = pm_start;
  block.end_addr = pm_end;
  block.size = (pm_end-pm_start);
  block.state = 0;
  block.prev = freelist.head;
  block.next = NULL;
  freelist.head->prev=NULL;
  freelist.head->next=&block;
  runlist.head->next=NULL;
  runlist.size=0;
}

static void block_cut(kblock *block,uintptr_t need_size){
    printf("you used memory from %d to %d\n",block->begin_addr,block->begin_addr+need_size);
    if(block->size==need_size){
        block->prev->next=block->next;
        block->next=NULL;
        block->prev=NULL;
        return;
    }
    uintptr_t rest_block_size=block->size-need_size;
    block->size = need_size;
    kblock *p_block = block->prev;
    kblock new_block;
    if(block->next==NULL){
        new_block.state=0;
        new_block.size=rest_block_size;
        new_block.end_addr=block->end_addr;
        block->end_addr=block->begin_addr+need_size;
        new_block.begin_addr=block->end_addr;
        block->next=NULL;
        block->prev=NULL;
        new_block.next=NULL;
        new_block.prev=p_block;
        p_block->next=&new_block;
        return;
    }
    kblock *n_block = block->next;
    if(n_block->begin_addr==block->end_addr){
        n_block->size+=rest_block_size;
        n_block->begin_addr-=rest_block_size;
        block->end_addr=block->begin_addr+need_size;
        p_block->next=n_block;
        block->next=NULL;
        block->prev=NULL;
        return;
    }
    new_block.state=0;
    new_block.size=rest_block_size;
    new_block.end_addr=block->end_addr;
    block->end_addr=block->begin_addr+need_size;
    new_block.begin_addr=block->end_addr;
    block->next=NULL;
    block->prev=NULL;
    new_block.next=n_block;
    new_block.prev=p_block;
    n_block->prev=&new_block;
    p_block->next=&new_block;
}

static void add_runlist(kblock *block){
    if(!runlist.size){
        runlist.head->next=block;
    }
    else{
        kblock *tail_block=runlist.head->next;
        while (tail_block->next!=NULL){
            tail_block=tail_block->next;
        }
        tail_block->next=block;
    }
    runlist.size++;
}

static void *alloc_unsafe(size_t size){
  if(size == 0)
    return NULL;
  uintptr_t block_size = (size/1024+(size%1024!=0))*1024;
  kblock *block = freelist.head->next;
  while(block->size<block_size&&block->next!=NULL){
      block = block->next;
  }
  if(block->size<block_size){
      printf("you need %d but you dont have it\n",block_size);
      return NULL;
      //assert(0);
  }
  block->state=1;
  block_cut(block,block_size);
  add_runlist(block);
  return (void*)block->begin_addr;
}

void free_unsafe(uintptr_t b_addr){
    printf("you want to free block from %d\n",b_addr);
    if(!runlist.size){
        printf("WRONG : WE DONT USE THE ADDR!\n");
        return;
    }
    kblock *used_block = runlist.head->next;
    while(used_block->begin_addr!=b_addr)
        used_block=used_block->next;
    if(used_block->begin_addr!=b_addr){
        printf("WRONG : WE DONT USE THE ADDR!\n");
        return;
    }
    used_block->state=0;
    runlist.size--;
    if(freelist.head->next==NULL){
        freelist.head->next=used_block;
        used_block->prev=freelist.head;
        return;
    }
    kblock *used_prev=freelist.head->next;
    if(used_block->end_addr < used_prev->begin_addr){
        freelist.head->next=used_block;
        used_block->next = used_prev;
        used_block->prev=freelist.head;
        used_prev->prev=used_block;
    }
    while(used_prev->next!=NULL && used_prev->next->end_addr < used_block->begin_addr){
        used_prev=used_prev->next;
    }
    if(used_prev->next==NULL){
        used_prev->next=used_block;
        used_block->prev=used_prev;
    }
    else{
        used_block->next=used_prev->next;
        used_prev->next->prev=used_block;
        used_block->prev=used_prev;
        used_prev->next=used_block;
    }
}


static void *kalloc(size_t size) {
  spin_lock(&alloc_lk);
  void *ret = alloc_unsafe(size);
  spin_unlock(&alloc_lk);
  return ret;
}

static void kfree(void *ptr) {
  spin_lock(&alloc_lk);
  free_unsafe((uintptr_t)ptr);
  spin_unlock(&alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
