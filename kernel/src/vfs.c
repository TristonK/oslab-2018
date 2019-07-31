#include <common.h>
#include <klib.h>
#include <devices.h>

extern inodeops_t blkfs_inode_op;
extern fsops_t blkfs_op;
extern inode_t *fs_lookup(struct filesystem *fs, const char *path, int flags, int from);


static void root_init(){
    root.parent = NULL;
    root.types = DIR_FILE;
    root.fs = &blkfs[0];
    root.mode = O_RDWR;
    root.refcnt = 0;
    root.ops = blkfs_op;
    root.size = 0;
    root.ptr = NULL;
}

void vfs_init (){
    kmt->spin_init(&inode_rwlk,"inode read_write lock");
    current_path = "/";
    root_init();
    //vfs->mount("/",&blkfs[0]);
	vfs->mount("/mnt",&blkfs[1]);
	vfs->mkdir("/dev");
	vfs->mount("/dev",&devfs);
	vfs->mkdir("/proc");
	vfs->mount("/proc",&procfs);
	vfs->mkdir("/dev/tty1");
	vfs->mkdir("/dev/tty2");
	vfs->mkdir("/dev/tty3");
	vfs->mkdir("/dev/tty4");
	vfs->mkdir("/dev/ramdisk0");
	vfs->mkdir("/dev/ramdisk1");
    vfs_ls("/");
}

int vfs_ls(const char* path){
    //!!!!!!!!!!!!!!!!!!!!!!!!change print
    inode_t* ls_node = fs_lookup(&blkfs[0],path,O_RDWR,0);
    if(ls_node==NULL){
        printf("ls: cannot access '%s': No such file or directory",path);
        return -1;
    }
    if(ls_node->types == ORD_FILE){
        printf("%s\n",ls_node->name);
    } else{
        inode_t *ls_child = ls_node->child;
        if(ls_child==NULL)
            return 0;
        while(ls_child!=NULL){
            printf("%s  ",ls_child->name);
            ls_child = ls_child->next;
        }
        printf("\n");
    }
}

int vfs_access (const char *path, int mode){
    int path_len = strlen(path);
    if(path_len<=0)
        return -1;
    //full path
    if()
    inode_t acess_inode;
}

int vfs_mount (const char *path, filesystem_t *fs){
    inode_t parent_node = root.fs->ops->lookup(root.fs,path,O_RDWR,0);
    create_inode(fs,parent_node, fs->name,O_RDWR);
}

int vfs_unmount (const char *path){

}

int vfs_mkdir (const char *path){
    int len = strlen(path);
    int i;
    for(i=len-1;i>=0;i--){
        if(path[i]=='/')
            break;
    }
    if(i==0){
        return root.ops->mkdir(root,path+1);
    }else
    {
        char parent_name[32];
        strcat(&parent_name,path,i);
        inode_t* parent_node = fs_lookup(blkfs[0],parent_name,O_RDWR,0);
        return parent_node->ops->mkdir(parent_node,path+i);
    }
}

int vfs_rmdir (const char *path){}

int vfs_link (const char *oldpath, const char *newpath){}

int vfs_unlink (const char *path){}

int vfs_open (const char *path, int flags){
    for(int i=0;i<NOFILE;i++){
        if(runtask[_cpu()]->fildes[i]->inode==NULL){
            inode_t* file_node = fs_lookup(blkfs[0],path,flags,0);
            runtask[_cpu()]->fildes[i]->inode = file_node;
            runtask[_cpu()]->fildes[i]->refcnt = 0;
            runtask[_cpu()]->fildes[i]->offset = 0;
            return i;
        }
    }
    printf("NO EXTRA FILE COULD BE ADDED\n");
    return -1;
}

ssize_t vfs_read (int fd, void *buf, size_t nbyte){
    file_t node_file = runtask[_cpu()]->fildes[fd];
    return node_file.inode->ops->read(node_file,buf,size);
}

ssize_t vfs_write (int fd, void *buf, size_t nbyte){
    file_t node_file = runtask[_cpu()]->fildes[fd];
    return node_file.inode->ops->write(node_file,buf,size);
}

off_t vfs_lseek (int fd, off_t offset, int whence){
    file_t node_file = runtask[_cpu()]->fildes[fd];
    return node_file.inode->ops->lseek(node_file,offset,whence);
}

int vfs_close (int fd){
    runtask[_cpu()]->fildes[fd]->inode = NULL;
    runtask[_cpu()]->fildes[fd]->offset = 0;
    return 0;
}

MODULE_DEF(vfs){
    .init = vfs_init,
    .access = vfs_access,
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .mkdir = vfs_mkdir,
    .rmdir = vfs_rmdir,
    .link = vfs_link,
    .unlink = vfs_unlink,
    .open = vfs_open,
    .read = vfs_read,
    .write = vfs_write,
    .lseek = vfs_lseek,
    .close = vfs_close,
 };
