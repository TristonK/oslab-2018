#include <common.h>
#include <klib.h>
//#include <src/x86/x86-qemu.h>
//#include <stdio.h>


#define LOCKDEF(name) \
  static volatile intptr_t name##_locked = 0; \
  static int name##_lock_flags[8]; \
  void name##_lock() { \
    name##_lock_flags[_cpu()] = _intr_read(); \
    _intr_write(0); \
    while (1) { \
      if (0 == _atomic_xchg(&name##_locked, 1)) break; \
    } \
  } \
  void name##_unlock() { \
    _atomic_xchg(&name##_locked, 0); \
    if (name##_lock_flags[_cpu()]) _intr_write(1); \
  }
//***************** Variables ******************
static uintptr_t pm_start, pm_end;
struct spinlock{
  intptr_t status;
  uintptr_t flags[8];
};
struct spinlock alloc_lk;
struct spinlock print_lk;
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
kblock freehead;
kblock runhead;
//****************** code ************************
/*static inline void cli(){
  asm volatile("cli");
}
static inline void sti(){
  asm volatile("sti");
}*/

/*void spin_lock(struct spinlock *lk){
  //printf("you locked it\n");
  lk->flags[_cpu()] = get_efl() & FL_IF;
  _intr_write(0);
  while(_atomic_xchg(&(lk->status), 1));
  __sync_synchronize();
  //printf("aaa\n");
}
void spin_unlock(struct spinlock *lk){
  //printf("unlock it\n");
  while(_atomic_xchg(&(lk->status), 0));
  __sync_synchronize();
  //asm volatile("movl $0, %0" : "+m" (lk) : );
  if(lk->flags[_cpu()])
  _intr_write(1); 
  //printf("ublock!\n");
} */

LOCKDEF(alloc);
LOCKDEF(print);


void show_alloc(){
  print_lock();
  printf("now i will show runlist with size %d\n",runlist.size);
  if(runlist.size){
    kblock *pr = runlist.head -> next;
    while(1){
      printf("begin at %d and end at %d and size is %x\n",pr->begin_addr, pr->end_addr, pr->size);
      if(pr->next==NULL)
        break;
      pr = pr->next;
    }
  }
  printf("now i will show you the freelist  with size %d\n",freelist.size);
  if(freelist.size){
    kblock *pr2 = freelist.head -> next ;
    while(1){
      printf("begin at %d and end at %d and size is %d\n",pr2->begin_addr,pr2->end_addr,pr2->size);
      printf("state is %d\n",pr2->state);
      if(pr2->next==NULL)
        break;
      pr2 = pr2->next;
    }
  }
  print_unlock();
}

kblock block;
static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  printf("heap start at %d\n",pm_start);
  printf("heap end at %d\n",pm_end);
  printf("you could use %d space\n",pm_end-pm_start);
  alloc_lk.status = 0;
  print_lk.status =0;
  runhead.begin_addr = 0;
  runhead.prev = NULL;
  runhead.end_addr = 0;
  freehead.end_addr = freehead.begin_addr = 0;
  freehead.prev = NULL;
  freelist.head = &freehead;
  runlist.head = &runhead; 
  block.begin_addr = pm_start;
  block.end_addr = pm_end;
  block.size = (pm_end-pm_start);
  block.state = 3;
  block.prev = freelist.head;
  block.next = NULL;
  freelist.head->prev=NULL;
  freelist.head->next = &block;
  printf("begin at %d and end at %d and size is %d\n",block.begin_addr,block.end_addr,block.size);
  runlist.head->next=NULL;
  runlist.size=0;
  freelist.size = 1;
}

static void block_cut(kblock *block,uintptr_t need_size){
    //spin_lock(&print_lk);
    print_lock();
    printf("you used memory from %x to %x from cpu %d\n",&block->begin_addr,&block->begin_addr+need_size,_cpu());
    //spin_unlock(&print_lk);
    print_unlock();
    if(block->size==need_size){
        block->prev->next=block->next;
        block->next=NULL;
        block->prev=NULL;
        freelist.size--;
        return;
    }
    uintptr_t rest_block_size=block->size-need_size;
    block->size = need_size;
    kblock *p_block = block->prev;
    kblock new_block;
    if(block->next==NULL){
        //printf("here3\n");
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
        //printf("here1\n");
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
      //spin_lock(&print_lk);
      print_lock();
      printf("you need %d but you dont have it\n",block_size);
      //spin_unlock(&print_lk);
      print_unlock();
      return NULL;
      //assert(0);
  }
  block->state=1;
  block_cut(block,block_size);
  add_runlist(block);
  //printf("here2\n");
  return (void*)block->begin_addr;
}

void free_unsafe(uintptr_t b_addr){
    //spin_lock(&print_lk);
    print_lock();
    printf("you want to free block from %I64u\n",b_addr);
    print_unlock();
    //spin_unlock(&print_lk);
    if(!runlist.size){
        //spin_lock(&print_lk);
        print_lock();
        printf("WRONG : WE DONT USE THE ADDR!\n");
        print_unlock();
        //spin_unlock(&print_lk);
        return;
    }
    kblock *used_block = runlist.head->next;
    while(used_block->begin_addr!=b_addr && used_block->next !=NULL){
        used_block=used_block->next;
        }
    if(used_block->begin_addr!=b_addr){
        //spin_lock(&print_lk);
        print_lock();
        printf("WRONG : WE DONT USE THE ADDR!\n");
        print_unlock();
        //spin_unlock(&print_lk);
        return;
    }
    used_block->state=0;
    runlist.size--;
    if(freelist.head->next==NULL){
        freelist.head->next=used_block;
        used_block->prev=freelist.head;
        freelist.size++;
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
  //spin_lock(&alloc_lk);
  alloc_lock();
  void *ret = alloc_unsafe(size);
  printf("finish\n");
  alloc_unlock();
  //spin_unlock(&alloc_lk);
  //printf("hi\n");
  return ret;
}

static void kfree(void *ptr) {
  //spin_lock(&alloc_lk);
  alloc_lock();
  free_unsafe((uintptr_t)(ptr));
  alloc_unlock();
  //spin_unlock(&alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
