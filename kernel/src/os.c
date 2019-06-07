#include <common.h>
#include <klib.h>
#include <devices.h>

spinlock_t ostrap;
handle* hde; 

extern ssize_t tty_write(device_t *dev, off_t offset, const void *buf, size_t count);


void echo_task(void *name) {
  device_t *tty = dev_lookup(name);
  while (1) {
    char line[128], text[128];
    sprintf(text, "(%s) $ ", name); tty_write(tty,0, text,strlen(text));
    int nread = tty->ops->read(tty, 0, line, sizeof(line));
    line[nread - 1] = '\0';
    sprintf(text, "Echo: %s.\n", line); tty_write(tty,0, text,strlen(text));
  }
}

static void os_init() {
  pmm->init();
  kmt->init();
  dev->init();
  kmt->spin_init(&ostrap,"trap_lock");
  hde = NULL;
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty1");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty2");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty3");
  kmt->create(pmm->alloc(sizeof(task_t)), "print", echo_task, "tty4");
  //handl.head = NULL;
  //handl.size = 0;
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
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  kmt->spin_lock(&ostrap);
  _Context *ret = NULL;
  handle *h = hde;
  if(hde==NULL)
    printf("shit\n");
  while (h!=NULL)
  {
    printf("hh\n");
    if(h->event == _EVENT_NULL || h->event == ev.event){
      //printf("hi\n");
      _Context *next = h ->handler(ev,context);
      if(next!=NULL) ret = next;
    }
    h = h->next;
  }
  //if(ret==NULL)
  //  printf("nullllll\n");
  //  panic("bull");
  kmt->spin_unlock(&ostrap);
  return ret;
}


static void os_on_irq(int seq, int event, handler_t handler) {
  if(hde == NULL){
    printf("hde is not null\n");
    hde = (handle*) pmm->alloc(sizeof(handle));
    hde -> pre =NULL;
    hde -> next =NULL;
    hde -> event = event;
    hde -> handler = handler;
    hde -> seq =seq;
    if(hde == NULL)
      printf("cao\n");
  }
  else{
    handle *insert_p = hde;
    handle *new_handle = (handle*)pmm->alloc(sizeof(handle));
    new_handle -> seq = seq;
    new_handle -> event =event;
    new_handle -> handler = handler;
    new_handle -> next =NULL;
    new_handle -> pre =NULL;
    if(insert_p->seq > seq){
      new_handle->next = insert_p;
      insert_p ->pre = new_handle;
      hde = new_handle;
      return;
    }
    while(insert_p!=NULL){
      if(insert_p->next==NULL || insert_p->next->seq > seq)
        break; 
      insert_p = insert_p->next;
    }
    if(insert_p->next==NULL){
      insert_p->next = new_handle;
      new_handle ->pre = insert_p;
    } else
    {
      new_handle-> next = insert_p->next;
      new_handle -> pre = insert_p;
      insert_p -> next -> pre = new_handle;
      insert_p ->next = new_handle;
    }
  }
  
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
