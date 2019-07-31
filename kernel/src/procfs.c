#include <common.h>
#include <klib.h>
#include <devices.h>

struct filesystem procfs;
char cpuino[428];
char memi1[64];
char memi2[64];
char memi3[64];
char in_ref[32];
char inname[16];
char out_ref[32];
char outname[16];

int cpuflag = 0;

int proc_cat(const char* path,int fd){
    //printf("shit\n");
    if(!strncmp(path,"/proc/cpuinfo",13)){
        if(cpuflag){
            vfs->write(fd,cpuino,strlen(cpuino));
        }else{
            cpuflag = 1;
            //printf("cpu is %d\n",_ncpu());
            for(int i=0;i<_ncpu();i++){
                char tempss[24];
                sprintf(tempss,"processor : %d\n",i);
                strcat(cpuino,tempss);
                strcat(cpuino,"vendor_id : OSintel\nmodel:17\nmodel name:OS(R) Core(TM) i0-0001HQ CPU @ 0.00GHz\n\n");
            }
            vfs->write(fd,cpuino,strlen(cpuino));
        }
    }else if(!strncmp(path,"/proc/meminfo",13)){
        sprintf(memi1,"MemTotal    : %d\n",all_size);
        sprintf(memi2,"MemAvailable: %d\n",all_size-used_size);
        sprintf(memi3,"MemUsed     : %d\n",used_size);
        vfs->write(fd,memi1,strlen(memi1));
        vfs->write(fd,memi2,strlen(memi2));
        vfs->write(fd,memi3,strlen(memi3));
    }else{
        if(!strcmp(path,"/proc/0")){
            sprintf(in_ref,"refcnt : %d\n\0",runtask[_cpu()]->fildes[0]->refcnt);
            sprintf(inname,"name: stdin\n");
            vfs->write(fd,inname,strlen(inname));
            vfs->write(fd,in_ref,strlen(in_ref));
        }
        else if (!strcmp(path,"/proc/1"))
        {
            sprintf(outname,"name: stdout\n");
            sprintf(out_ref,"refcnt : %d\n\0",runtask[_cpu()]->fildes[1]->refcnt);
            vfs->write(fd,outname,strlen(outname));
            vfs->write(fd,out_ref,strlen(out_ref));
        }
        
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