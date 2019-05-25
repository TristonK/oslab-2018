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
#define block_num 4096
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
kblock block[block_num];
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
LOCKDEF(test);

void show_alloc(){
  print_lock();
  printf("|----------------------------|\n");
  printf("now i will show runlist with size %d\n",runlist.size);
  if(runlist.size){
    kblock *pr = runlist.head -> next;
    while(1){
      printf("begin at %d and end at %d and size is %d\n",pr->begin_addr, pr->end_addr, pr->size);
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
      //printf("state is %d\n",pr2->state);
      if(pr2->next==NULL)
        break;
      pr2 = pr2->next;
    }
  }
  printf("|----------------------------|\n");
  print_unlock();
}

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
  for(int i=0;i<block_num;i++){
    block[i].state = 0;  // 0: unused 1:in freelist 2:in runlist
  } 
  block[0].begin_addr = pm_start;
  block[0].end_addr = pm_end;
  block[0].size = (pm_end-pm_start);
  block[0].state = 1;
  block[0].prev = freelist.head;
  block[0].next = NULL;
  freelist.head->prev=NULL;
  freelist.head->next = &block[0];
  //printf("begin at %d and end at %d and size is %d\n",block.begin_addr,block.end_addr,block.size);
  runlist.head->next=NULL;
  runlist.size=0;
  freelist.size = 1;
}

static int find_free_block(){
  for(int i=0;i<block_num;i++){
    if(block[i].state == 0){
      return i;
    }
  }
  print_lock();
  printf("NO FREE BLOCK TO USE");
  print_unlock();
  assert(0);
}

static void block_cut(kblock *blockc,uintptr_t need_size){
    //spin_lock(&print_lk);
    print_lock();
    printf("*** you used memory from %d to %d from cpu %d\n",blockc->begin_addr,blockc->begin_addr+need_size,(_cpu()+1));
    //spin_unlock(&print_lk);
    print_unlock();
    if(blockc->size == need_size){
        blockc->prev->next = blockc->next;
        blockc->next->prev = blockc ->prev;
        blockc->next=NULL;
        blockc->prev=NULL;
        freelist.size--;
        return;
    }
    uintptr_t rest_block_size=blockc->size-need_size;
    blockc->size = need_size;
    kblock *p_block = blockc->prev;
    kblock *new_block = &block[find_free_block()];
    if(blockc->next==NULL){
        //printf("!\n");
        new_block->state= 1;
        new_block->size=rest_block_size;
        new_block->end_addr=blockc->end_addr;
        blockc->end_addr=blockc->begin_addr+need_size;
        new_block->begin_addr=blockc->end_addr;
        new_block->next=NULL;
        new_block->prev=p_block;
        p_block->next= new_block;
        printf(" hi %d\n",p_block->end_addr);
        blockc->next=NULL;
        blockc->prev=NULL;
        return;
    }
    kblock *n_block = blockc->next;
    if(n_block->begin_addr==blockc->end_addr){
        //printf("here1\n");
        new_block ->state = 0;
        n_block->size+=rest_block_size;
        n_block->begin_addr-=rest_block_size;
        blockc->end_addr=blockc->begin_addr+need_size;
        p_block->next=n_block;
        n_block->prev = p_block;
        blockc->next=NULL;
        blockc->prev=NULL;
        return;
    }
    new_block->state=1;
    new_block->size=rest_block_size;
    new_block->end_addr=blockc->end_addr;
    blockc->end_addr=blockc->begin_addr+need_size;
    new_block->begin_addr=blockc->end_addr;
    new_block->next=n_block;
    new_block->prev=p_block;
    n_block->prev= new_block;
    p_block->next= new_block; 
    blockc->next=NULL;
    blockc->prev=NULL;
}

static void add_runlist(kblock *blockadd){
    if(!runlist.size){
        runlist.head->next=blockadd;
        blockadd ->prev = runlist.head;
        blockadd ->next = NULL;
    }
    else{
        kblock *tail_block=runlist.head->next;
        while (tail_block->next!=NULL){
            tail_block=tail_block->next;
        }
        tail_block->next= blockadd;
        blockadd ->prev = tail_block;
        blockadd -> next = NULL;
    }
    runlist.size++;
}

static void *alloc_unsafe(size_t size){
  if(size == 0)
    return NULL;
  uintptr_t block_size = (size/1024+(size%1024!=0))*1024;
  kblock *block1 = freelist.head->next;
  while(block1->size<block_size&&block1->next!=NULL){
      block1 = block1->next;
  }
  if(block1->size<block_size){
      //spin_lock(&print_lk);
      print_lock();
      printf("you need %d but you dont have it\n",block_size);
      //spin_unlock(&print_lk);
      print_unlock();
      return NULL;
      //assert(0);
  }
  block1->state=2;
  block_cut(block1,block_size);
  add_runlist(block1);
  //printf("here2\n");
  return (void*)block1->begin_addr;
}

static void check_block(){
    kblock *chek = freelist.head->next;
    while(chek -> next != NULL){
      if(chek->end_addr == chek->next->begin_addr){
        kblock *nblock = chek->next;
        chek->end_addr = nblock->end_addr;
        chek->size += nblock->size;
        nblock ->state = 0;
        chek ->next = nblock ->next;
        nblock ->next = nblock->prev = NULL;
        freelist.size--;
      }
      chek = chek ->next;
    }
}

void free_unsafe(uintptr_t b_addr){
    //spin_lock(&print_lk);
    print_lock();
    printf("### you want to free block from %d\n",b_addr);
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
    used_block->state=1;
    runlist.size--;
    freelist.size++;
    kblock *ppblock = used_block->prev;
    ppblock->next = used_block->next;
    used_block->next->prev = used_block->prev;
    //freelist is null 
    if(freelist.head->next==NULL){
        freelist.head->next=used_block;
        used_block->prev=freelist.head;
        used_block->next =NULL;
        freelist.size++;
        return;
    }
    kblock *used_prev=freelist.head->next;
    if(used_block->end_addr < used_prev->begin_addr){
        freelist.head->next=used_block;
        used_block->next = used_prev;
        used_block->prev=freelist.head;
        used_prev->prev=used_block;
        return;
    }
    if(used_block ->end_addr == used_prev -> begin_addr){
      used_prev->begin_addr = used_block ->begin_addr;
      used_prev->size += used_block->size;
      used_block->state = 0;
      used_block -> prev = used_block ->next =NULL;
      freelist.size--;
      return;
    }
    while(used_prev->next!=NULL && used_prev->next->end_addr < used_block->begin_addr){
        used_prev=used_prev->next;
    }
    if(used_prev->next==NULL){
        used_prev->next=used_block;
        used_block->prev=used_prev;
    }
    else if(used_prev->end_addr == used_block->begin_addr){
      used_prev->end_addr = used_block ->end_addr;
      used_prev->size += used_block->size;
      used_block->state = 0;
      used_block ->prev =NULL;
      //ppblock->next = used_block->next;
      used_block -> next =NULL;
      freelist.size--;
    }
    else{
        used_block->next=used_prev->next;
        used_prev->next->prev=used_block;
        used_block->prev=used_prev;
        used_prev->next=used_block;
    }
    check_block();
}




static void *kalloc(size_t size) {
  //spin_lock(&alloc_lk);
  alloc_lock();
  void *ret = alloc_unsafe(size);
  //printf("finish\n");
  alloc_unlock();
  //spin_unlock(&alloc_lk);
  //printf("hi\n");
  return ret;
}

static void kfree(void *ptr) {
  //spin_lock(&alloc_lk);
  alloc_lock();
  if(ptr == NULL){
    print_lock();
    printf("WRONG: YOU WANT TO FREE A NULL SPACE\n");
    print_unlock();
  }
  else{
    free_unsafe((uintptr_t)(ptr));
  }
  alloc_unlock();
  //spin_unlock(&alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
