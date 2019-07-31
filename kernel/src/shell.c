#include <common.h>
#include <klib.h>
#include <devices.h>

char full_path[128];

void path_translation(const char* path){
    #ifdef Debug
    printf("before the trans the path is %s\n",path);
    #endif
    memset(full_path,'\0',sizeof(full_path));
    if(!strncmp(path,"./",2)){
        strcat(full_path,current_path);
        path = path+1;
        strcat(full_path,path);
    } else if(!strncmp(path,"../",3)){
        int path_len = strlen(current_path);
        int i;
        for(i=path_len-1;i>=0;i--){
            if(current_path[i]=='/')
                break;
        }
        char newpath[128];
        strncpy(newpath,current_path,i);
        strcat(full_path,newpath);
        path = path+2;
        strcat(full_path,path);
    } else{
        strcpy(&full_path,path);
    }
    #ifdef Debug
    printf("after translation the path is %s\n",full_path);
    #endif
}

void shell_thread(int tty_id) {
    char buf[128];
    //**
    sprintf(buf, "/dev/tty%d", tty_id);
    int stdin = vfs->open(buf, O_RDONLY);
    int stdout = vfs->open(buf, O_WRONLY);
    while (1) {
        if (got_a_line()) {
            //path_translation();
            //todo()
        } else {
            ssize_t nread = fs->read(stdin, buf, sizeof(buf));
            //todo
        }
    /* supported commands:
    * ls
    * cd /proc
    * cat filename
    * mkdir /bin
    * rm /bin/abc
    */
    }
}