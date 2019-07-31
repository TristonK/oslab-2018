#include <common.h>
#include <klib.h>
#include <devices.h>

struct filesystem procfs;

int proc_cat(const char* path,int fd){
    if(!strncmp(path,"/proc/cpuinfo",13)){

    }else if(!strncmp(path,"/proc/meminfo",13)){

    }else{

    }
    return 0;
}
/* void procfs_init(struct filesystem *fs, const char *name, device_t *dev){
    fs->dev = NULL;
    strcpy(fs->name,name);
}  
inode_t *procfs_lookup(struct filesystem *fs, const char *path, int flags, int from){
}
int procfs_close(inode_t *inode){}
int procifs_open(file_t *file, int flags){}
int procifs_close(file_t *file){}
ssize_t procifs_read(file_t *file, char *buf, size_t size){}
ssize_t procifs_write(file_t *file, const char *buf, size_t size){}
off_t procifs_lseek(file_t *file, off_t offset, int whence){}
int procifs_mkdir(inode_t *My, const char *name){}
int procifs_rmdir(inode_t *My, const char *name){}
int procifs_link(const char *name, inode_t *inode,inode_t *new){}
int procifs_unlink(const char *name,inode_t *inode){}*/