#include <common.h>
#include <klib.h>
#include <devices.h>

//for blkfs

int ifs_open(file_t *file, int flags){
    
}
int ifs_close(file_t *file){}
ssize_t ifs_read(file_t *file, char *buf, size_t size){}
ssize_t ifs_write(file_t *file, const char *buf, size_t size){}
off_t ifs_lseek(file_t *file, off_t offset, int whence){}
int ifs_mkdir(inode_t *My, const char *name){}
int ifs_rmdir(inode_t *My, const char *name){}
int ifs_link(const char *name, inode_t *inode,inode_t *new){}
int ifs_unlink(const char *name,inode_t *inode){}