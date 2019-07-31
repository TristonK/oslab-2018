#include <common.h>
#include <klib.h>
#include <devices.h>

struct filesystem blkfs[2];
extern inode_t* create_inode(struct filesystem* fs,inode_t *parent_node, const char* name,int mode);
extern int free_inode(inode_t* f_inode);
extern inodeops_t blkfs_inode_op;
extern fsops_t blkfs_op;

// FOR blkfs
void fs_init(struct filesystem *fs, const char *name, device_t *dev){
    fs->dev = dev;
    strcpy(fs->name,name);
    fs->ops = &blkfs_op;
}

static inode_t *fs_recursive_find(struct filesystem *fs, const char *path, int flags,inode_t *parent){
    //printf("try to find %s\n",path);
    inode_t* scan = parent->child;
    if(path[0]!='/'){
        printf("BAD PATH\n");
    }
    int path_len = strlen(path);
    char find_name[32];
    int i;int final_path = 1;
    for(i=1;i<path_len;i++){
        if(!strncmp(path+i,"/",1)){
            printf("hi\n");
            final_path = 0;
            find_name[i] = '\0';
            break;
        }else
        {
            find_name[i-1] = path[i];
        }
    }
    printf("gg %s\n",find_name);
    while(scan->next!=NULL){
        if(!strcmp(scan->name,find_name)){
            if(final_path){
                return scan;
            }else
            {
                return fs_recursive_find(scan->fs,path+i,flags,scan);
            }
        }
        scan = scan->next;
    }
    printf("Invalid path %s\n",path);
    return NULL;
}


inode_t *fs_lookup(struct filesystem *fs, const char *path, int flags, int from){
    //FUll path
    if(1){
        /*if(strcmp(fs->name,"blkfs")){
            printf("ERROR: NOT blkfs SYSTEM, please use the correct function\n");
            return NULL;
        }*/
        if(path[0]=='/' && path[1]=='\0'){
            return &root;
        }else
        {   printf("go to find %s\n",path);
            return fs_recursive_find(fs,path,flags,&root);
        }
        
       
    }
    //relative path
    else{
        //TODO();
        return NULL;
    }
}

int fs_close(inode_t *inode){
    pmm->free((void*)inode);
    return 0;
}

int ifs_open(file_t *file, int flags){
    if(file->inode->types == DIR_FILE){
        printf("ERROR: You Try To Open A Dir\n");
        return -1;
    }
    file->inode->mode = flags;
    file->offset = 0;
    return 0;
}
int ifs_close(file_t *file){
    pmm->free(file);
    return 0;
}
ssize_t ifs_read(file_t *file, char *buf, size_t size){
    kmt->spin_lock(&inode_rwlk);
    if(file->inode->types==DIR_FILE){
        printf("ERROR: %s is a directory\n",file->inode->name);
        return -1;
    }
    device_t *inode_dev = (device_t *)file->inode->ptr;
    ssize_t read_size = file->offset + size > file->inode->size?file->inode->size - file->offset:size;
    inode_dev->ops->read(inode_dev,file->offset,buf,read_size);
    file->offset += read_size;
    kmt->spin_unlock(&inode_rwlk);
    return size;
}
ssize_t ifs_write(file_t *file, const char *buf, size_t size){
    //MAY WRONG: PLEASE CHANGE THE SIZE AND POINTER
    kmt->spin_lock(&inode_rwlk);
    if(file->inode->types==DIR_FILE){
        printf("ERROR: %s is a directory\n",file->inode->name);
        return -1;
    }
    device_t *inode_dev = (device_t *)file->inode->ptr;
    inode_dev->ops->write(inode_dev,file->offset,buf,size);
    file->offset += size;
    kmt->spin_unlock(&inode_rwlk);
    return size;
}
off_t ifs_lseek(file_t *file, off_t offset, int whence){
    switch (whence)
    {
    case SEEK_SET:
        file->offset = offset;
        break;
    case SEEK_CUR:
        file->offset += offset;
        break;
    case SEEK_END:
        file->offset = file->inode->size + offset;
        break;
    default:
        printf("ERROR: UNAVLID WHENCE MODE\n");
        return -1;
        break;
    }
    return file->offset;
}
int ifs_mkdir(inode_t *My, const char *name){
    //My: parent_inode
    create_inode(My->fs,My,name,O_RDONLY);
    return 0;
}
int ifs_rmdir(inode_t *My, const char *name){
    //My: the inode need to be delete
    free_inode(My);
    return 0;
}
int ifs_link(const char *name, inode_t *inode,inode_t *new){
    return 0;
}
int ifs_unlink(const char *name,inode_t *inode){
    return 0;
}

fsops_t blkfs_op = {
    .init = fs_init,
    .lookup = fs_lookup,
    .close = fs_close, 
};

inodeops_t blkfs_inode_op = {
    .open   =   ifs_open,
    .close  =   ifs_close,
    .read   =   ifs_read,
    .write  =   ifs_write,
    .lseek  =   ifs_lseek,
    .mkdir  =   ifs_mkdir,
    .rmdir  =   ifs_rmdir,
    .link   =   ifs_link,
    .unlink =   ifs_unlink,
};