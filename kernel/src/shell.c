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
        strcpy(full_path,path);
    }
    #ifdef Debug
    printf("after translation the path is %s\n",full_path);
    #endif
}

void shell_thread(void* ttyid) {
    //output form: "buf: current_path$ stdin"  
    int tty_id = *(int*)ttyid;
    char buf[32];
    sprintf(buf, "/dev/tty%d", tty_id);
    int stdin = vfs->open(buf, O_RDONLY);
    int stdout = vfs->open(buf, O_WRONLY);
    char text[128];
    char line[128];
    char inter[4] = " $ ";
    ssize_t nread = 0;
    while (1) {
        if (nread!=0) {
            if(line[nread-1]=='\n'){
                int cmd_cnt = 0; int flag = 0; 
                char cmd[32];char in_path[128];
                int path_cnt = 0;
                for(int i=0;i<nread;i++){
                    if(line[i]==' '){
                        cmd[i] = '\0';
                        flag = 1;
                    }else if(!flag){
                        cmd[i] = line[i];
                        cmd_cnt++;
                    }else{
                        in_path[path_cnt] = line[i];
                        path_cnt++;
                    }
                }
                in_path[path_cnt] = '\0';
                if(!path_cnt){
                    strcpy(in_path,current_path);
                }
                path_translation(in_path);
                if(!strcmp(cmd,"ls")){
                    vfs->ls(&full_path,stdout);
                }else if (!strcmp(cmd,"cd"))
                {
                    printf("cdddd\n");
                }else if(!strcmp(cmd,"mkdir")){
                    vfs->mkdir(&full_path);
                }else if(!strcmp(cmd,"cat")){
                    printf("cattt\n");
                }eles if(!strcmp(cmd,"rm")){
                    printf("rmmmmm\n");
                }else {
                    char err_info[128];
                    sprintf(err_info,"ksh : command not found : %s\n",cmd);
                    vfs->write(stdout,err_info,sizeof(err_info));
                }
            }
        } else {
            sprintf(text, "(%s) : ", buf);
            vfs->write(stdout, text, strlen(buf)+5);
			vfs->write(stdout, current_path, strlen(current_path));
			vfs->write(stdout, inter, 3);
            nread = vfs->read(stdin, line, sizeof(line));
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