#include <common.h>
#include <klib.h>
#include <devices.h>

extern inodeops_t blkfs_inode_op;
extern fsops_t blkfs_op;
extern inode_t *fs_lookup(struct filesystem *fs, const char *path, int flags, int from);
extern inode_t* create_inode(struct filesystem* fs,inode_t *parent_node, const char* name,int mode,int types);
extern int free_inode(inode_t* f_inode);
extern int proc_cat(const char* path,int fd);

static void root_init(){
    root.parent = NULL;
    root.types = DIR_FILE;
    root.fs = &blkfs[0];
    root.mode = O_RDWR;
    root.refcnt = 0;
    root.ops = &blkfs_inode_op;
    root.size = 0;
    root.ptr = NULL;
}


int check_ls(const char* path){
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
    return 0;
}

char tab[2] = " ";
char next_line[2] = "\n";


int vfs_ls(const char* path,int fd){
   // printf("go ls\n");
    inode_t* ls_node = fs_lookup(&blkfs[0],path,O_RDWR,0);
    if(ls_node==NULL){
        char err_info[128];
        sprintf(err_info,"ls: cannot access '%s': No such file or directory",path);
        vfs->write(fd,err_info,sizeof(err_info));
        return -1;
    }
    if(ls_node->types == ORD_FILE){
        vfs->write(fd,ls_node->name,sizeof(ls_node->name));
    } else{
        inode_t *ls_child = ls_node->child;
        if(ls_child==NULL)
            return 0;
        while(ls_child!=NULL){
            vfs->write(fd,ls_child->name,sizeof(ls_child->name));
            vfs->write(fd,&tab,2);
            ls_child = ls_child->next;
        }
        vfs->write(fd,&next_line,1);
    }
    return 0;
}

int vfs_cd(const char* path,int fd){
    if(fs_lookup(&blkfs[0],path,O_RDONLY,0)==NULL){
        char err_info[128];
        sprintf(err_info,"cd: no such file or directory: %s\n",path);
        vfs->write(fd,err_info,strlen(err_info));
        return -1;
    }
    memset(current_path,'\0',sizeof(current_path));
    strcpy(current_path,path);
    return 0;
}

int vfs_cat(const char* path,int fd){
    char error_info[128];
    memset(error_info,'\0',sizeof(error_info));
    if(strncmp(path,"/dev",4)==0){
		vfs->write(fd,"Permission denied\n",18);
		return -1;
	}
	if(strncmp(path,"/proc",5)==0){
		proc_cat(path,fd);
		return 0;
//		vfs->write(sto,"Permission denied\n",18);
//		return -1;
	} else{
        inode_t* cat_node = fs_lookup(&blkfs[0],path,O_RDWR,0);
        if(cat_node==NULL){
            sprintf(error_info,"cat: cannot cat '%s': No such file or directory",path);
            vfs->write(fd,error_info,strlen(error_info));
            return -1;
        }else if(cat_node->types == DIR_FILE){
            sprintf(error_info,"cat: cannot cat '%s':Is a directory",path);
            vfs->write(fd,error_info,strlen(error_info));
            return -1;
        }else if(cat_node->content == NULL){
            return 0;
        }else{
            vfs->write(fd,cat_node->content,strlen(cat_node->content));
            return 0;
        }
    }
    return 0;
}

int vfs_rm(const char* path,int fd){
    inode_t* dele_node = fs_lookup(&blkfs[0],path,O_RDONLY,0);
    char error_info[128];
    memset(error_info,'\0',sizeof(error_info));
    if(dele_node == NULL){
        sprintf(error_info,"rm: cannot remove '%s': No such file or directory",path);
        vfs->write(fd,error_info,strlen(error_info));
        return -1;
    }
    if(dele_node->types == DIR_FILE){
        sprintf(error_info,"rm: cannot remove '%s': Is a directory,please use rmdir\n",path);
        vfs->write(fd,error_info,strlen(error_info));
    } else{
        free_inode(dele_node);
    }
    return 0;
}

int vfs_addcontent(const char*path,char* content){
    inode_t *added = fs_lookup(&blkfs[0],path,O_RDWR,0);
    if(added == NULL || added->types == DIR_FILE){
        return -1;
    }
    added->content = content;
    return 0;
}


void vfs_init (){
    kmt->spin_init(&inode_rwlk,"inode read_write lock");
    //printf("shit1\n");
    strcpy(current_path,"/");
    current_fs = &blkfs[0];
    root_init();
    //vfs->mount("/",&blkfs[0]);
	//vfs->mount("/mnt",&blkfs[1],"mnt");
    //printf("shit1\n");
    vfs->mkdir("mnt");
	vfs->mkdir("dev");
    vfs->mkdir("proc");
	//vfs->mount("/dev",&devfs,"dev");
	//vfs->mount("/proc",&procfs,"proc");
	vfs->newfile("/dev/tty1");
	vfs->newfile("/dev/tty2");
	vfs->newfile("/dev/tty3");
	vfs->newfile("/dev/tty4");
	vfs->newfile("/dev/ramdisk0");
	vfs->newfile("/dev/ramdisk1");
    vfs->newfile("/proc/cpuinfo");
    vfs->newfile("/proc/meminfo");
    vfs->newfile("/hello.c");
    char *added_con = pmm->alloc(128);
    strcpy(added_con,"#include<stdio.h>\nint main(){\n  printf(\"hello world\");\n  return 0;\n}\n\0");
    vfs_addcontent("/hello.c",added_con);
    //vfs_ls("/");
    //check_ls("/dev");
}


int vfs_access (const char *path, int mode){
    int path_len = strlen(path);
    if(path_len<=0)
        return -1;
    //full path
    return 0;
}

int vfs_mount (const char *path, filesystem_t *fs,char *name){
    inode_t* parent_node = root.fs->ops->lookup(root.fs,path,O_RDWR,0);
    create_inode(fs,parent_node, name,O_RDWR,DIR_FILE);
    return 0;
}

int vfs_unmount (const char *path){
    return 0;
}

int vfs_mkdir (const char *path){
    kmt->spin_lock(&inode_rwlk);
    int lens = strlen(path);
    int i=lens-1;
    //printf("is is %d and %d\n",i,lens-1);
    //printf("%s\n",(path+1));
    int flag=0;
    for(;i>=0;i=i-1){
        if(!strncmp((path+i),"/",1))
            break;
        else if(i==0){
            flag = 1;
            break;
        }
    }
    if(flag){
        //printf("%s pathsss\n",path);
        inode_t* parent_inode = fs_lookup(&blkfs[0],current_path,O_RDWR,0);
        parent_inode->ops->mkdir(parent_inode,path);
        kmt->spin_unlock(&inode_rwlk);
        return 0;
    } else if(i==0){
        //printf("%s path\n",path);
        root.ops->mkdir(&root,path+1);
        kmt->spin_unlock(&inode_rwlk);
        return 0;
    }else
    {
        char parent_name[32];
        strncpy(parent_name,path,i);
        parent_name[i]='\0';
        inode_t* parent_node = fs_lookup(&blkfs[0],parent_name,O_RDWR,0);
        parent_node->ops->mkdir(parent_node,path+i+1);
        kmt->spin_unlock(&inode_rwlk);
        return 0;
    }
}

int vfs_newfile(const char*path){
    kmt->spin_lock(&inode_rwlk);
    int lens = strlen(path);
    int i=lens-1;
    //printf("is is %d and %d\n",i,lens-1);
    //printf("%s\n",(path+1));
    int flag=0;
    for(;i>=0;i=i-1){
        if(!strncmp((path+i),"/",1))
            break;
        else if(i==0){
            flag = 1;
            break;
        }
    }
    if(flag){
        inode_t* parent_inode = fs_lookup(&blkfs[0],current_path,O_RDWR,0);
        create_inode(parent_inode->fs,parent_inode,path,O_RDONLY,ORD_FILE);
        kmt->spin_unlock(&inode_rwlk);
        return 0;
    } else if(i==0){
        create_inode(root.fs,&root,path+1,O_RDONLY,ORD_FILE);
        kmt->spin_unlock(&inode_rwlk);
        return 0;
    }else
    {  
        char parent_name[32];
        strncpy(parent_name,path,i);
        parent_name[i]='\0';
        inode_t* parent_node = fs_lookup(&blkfs[0],parent_name,O_RDWR,0);
        create_inode(parent_node->fs,parent_node,path+i+1,O_RDONLY,ORD_FILE);
        kmt->spin_unlock(&inode_rwlk);
        return 0;
    }
}

static void rmdir_recursive(inode_t* dele){
    while(dele->child!=NULL){
        if(dele->child->types == DIR_FILE){
            rmdir_recursive(dele->child);
        } else{
            free_inode(dele->child);
        }
    }
    free_inode(dele);
}

int vfs_rmdir (const char *path){
    inode_t* dele_node = fs_lookup(&blkfs[0],path,O_RDONLY,0);
    if(dele_node==NULL)
        return -1;
    rmdir_recursive(dele_node);
    return 0;
}

int vfs_link (const char *oldpath, const char *newpath){
    return 0;
}

int vfs_unlink (const char *path){
    return 0;
}

int vfs_open (const char *path, int flags){
    for(int i=0;i<NOFILE;i++){
        if(runtask[_cpu()]->fildes[i]==NULL){
            inode_t* file_node = fs_lookup(&blkfs[0],path,flags,0);
            file_t* newfile = pmm->alloc(sizeof(file_t));
            newfile->inode = file_node;
            newfile->refcnt = 0;
            newfile->offset = 0;
            runtask[_cpu()]->fildes[i] = newfile;
            char tp_name[16];
            sprintf(tp_name,"/proc/%d\0",i);
            printf("%s\n",tp_name);
            //vfs->newfile(temp_name);
            return i;
        }
    }
    printf("NO EXTRA FILE COULD BE ADDED\n");
    return -1;
}

ssize_t vfs_read (int fd, void *buf, size_t nbyte){
    file_t* node_file = runtask[_cpu()]->fildes[fd];
    return node_file->inode->ops->read(node_file,buf,nbyte);
}

ssize_t vfs_write (int fd, void *buf, size_t nbyte){
    file_t* node_file = runtask[_cpu()]->fildes[fd];
    return node_file->inode->ops->write(node_file,buf,nbyte);
}

off_t vfs_lseek (int fd, off_t offset, int whence){
    file_t* node_file = runtask[_cpu()]->fildes[fd];
    return node_file->inode->ops->lseek(node_file,offset,whence);
}

int vfs_close (int fd){
    file_t* f_file = runtask[_cpu()]->fildes[fd];
    runtask[_cpu()]->fildes[fd] = NULL;
    f_file->inode->ops->close(f_file);
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
    .ls = vfs_ls,
    .cd = vfs_cd,
    .cat = vfs_cat,
    .rm = vfs_rm,
    .newfile = vfs_newfile,
 };
