#include <common.h>
#include <klib.h>
#include <devices.h>

fsops_t devfs_op{
    .init = devfs_init,
    .lookup = devfs_lookup,
    .close = devfs_close, 
}

struct filesystem devfs = {
    .name = "/dev",
    .ops = &devfs_op,
    .dev = NULL,
};

void devfs_init(struct filesystem *fs, const char *name, device_t *dev){
    return;
}  
inode_t *devfs_lookup(struct filesystem *fs, const char *path, int flags, int from){
    return NULL;
}
int devfs_close(inode_t *inode){
    return 0;
}


/* 
int devifs_open(file_t *file, int flags){}
int devifs_close(file_t *file){}
ssize_t devfs_read(file_t *file, char *buf, size_t size){}
ssize_t devifs_write(file_t *file, const char *buf, size_t size){}
off_t devifs_lseek(file_t *file, off_t offset, int whence){}
int devifs_mkdir(inode_t *My, const char *name){}
int devifs_rmdir(inode_t *My, const char *name){}
int devifs_link(const char *name, inode_t *inode,inode_t *new){}
int devifs_unlink(const char *name,inode_t *inode){}
*/
