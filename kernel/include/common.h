#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
//#include <x86.h>

task_t *runtask[16];

/*
 */ 
spinlock_t inode_rwlk;

#define maxCpu _ncpu()

enum {
    NONE=0,RUNABLE,RUNNING,YIELD
};

/* 
 * file's right
 * R_OK : 4
 * W_OK : 2
 * X_OK : 1
 * F_OK : 0 
 */
#define R_OK  4
#define W_OK  2
#define X_OK  1
#define F_OK  0
#define O_RDONLY 4
#define O_WRONLY 2
#define O_RDWR 6
/*
*file's type
* directory : 2
* ordinary file: 1
 */
#define DIR_FILE 2
#define ORD_FILE 1

#define NOFILE 16

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct file file_t;
typedef struct inode inode_t;
typedef struct fsops fsops_t;
typedef struct filesystem filesystem_t;

struct file {
    int refcnt; // 引用计数
    inode_t *inode;
    uint64_t offset;
    //int flags;
//...
};


struct Cpu{
    int ncli;    //Depth of pushcli nesting
    int intena; // interrupt enabled before pushcli?
};

struct task {
    int id;
    int state;
    int cpu_index;
    const char* name;
    _Context context;
    _Area stk;
    char stack[4096];
    file_t *fildes[NOFILE];
};
struct spinlock {
    intptr_t locked;
    const char *name;
    int cpu;
};
struct semaphore {
    const char* name;
    int value;
    spinlock_t lock;
    int task_id[32];
    task_t* wtask[32];
    int wait_pos;
};

typedef struct Handle{
    int seq;
    int event;
    handler_t handler;
    struct Handle *pre;
    struct Handle *next;
}handle;

/*struct handles{
    int size;
    struct handle *head;
};*/




struct filesystem{
  // ... 
  char name[32];
  fsops_t *ops;
  device_t *dev;
};

struct fsops {
  void (*init)(struct filesystem *fs, const char *name, device_t *dev);
  inode_t *(*lookup)(struct filesystem *fs, const char *path, int flags, int from);
  int (*close)(inode_t *inode);
};

typedef struct inodeops {
  int (*open)(file_t *file, int flags);
  int (*close)(file_t *file);
  ssize_t (*read)(file_t *file, char *buf, size_t size);
  ssize_t (*write)(file_t *file, const char *buf, size_t size);
  off_t (*lseek)(file_t *file, off_t offset, int whence);
  int (*mkdir)(inode_t *My, const char *name);
  int (*rmdir)(inode_t *My, const char *name);
  int (*link)(const char *name, inode_t *inode,inode_t *new);
  int (*unlink)(const char *name,inode_t *inode);
  // 你可以自己设计readdir的功能
} inodeops_t;

struct inode {
	//...
    char name[32];
    int mode;
    int refcnt;
	void *ptr;
	struct filesystem *fs;
	inodeops_t *ops;
    int types;
    int size;
    inode_t *parent;
    inode_t *child;
    inode_t *next;
    void *content;
    // ...
};

typedef struct {
  void (*init)();
  int (*access)(const char *path, int mode);
  int (*mount)(const char *path, filesystem_t *fs, char *name);
  int (*unmount)(const char *path);
  int (*mkdir)(const char *path);
  int (*rmdir)(const char *path);
  int (*link)(const char *oldpath, const char *newpath);
  int (*unlink)(const char *path);
  int (*open)(const char *path, int flags);
  ssize_t (*read)(int fd, void *buf, size_t nbyte);
  ssize_t (*write)(int fd, void *buf, size_t nbyte);
  off_t (*lseek)(int fd, off_t offset, int whence);
  int (*close)(int fd);
  int (*ls)(const char *path,int fd);
  int (*cd)(const char *path,int fd);
  int (*cat)(const char *path,int fd);
  int (*rm)(const char* path,int fd)
} MODULE(vfs);

inode_t root;
extern struct filesystem blkfs[2],procfs,devfs;
char current_path[128];
struct filesystem *current_fs;

#endif
