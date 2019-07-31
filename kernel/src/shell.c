#include <common.h>
#include <klib.h>
#include <devices.h>
#define Debug

char full_path[128];

void path_translation(const char* path){
    #ifdef Debug
    printf("before the trans the path is %s\n",path);
    #endif
    memset(full_path,'\0',sizeof(full_path));
    if(!strncmp(path,"..",2)){
        int path_len = strlen(current_path);
        int i;
        for(i=path_len-1;i>=0;i--){
            if(current_path[i]=='/')
                break;
        }
        char newpath[128];
        if(i==0){
            strncpy(newpath,"/",1);
        }else
            strncpy(newpath,current_path,i);
        strcat(full_path,newpath);
        path = path+2;
        strcat(full_path,path);
    } else if(!strncmp(path,".",1) || strncmp(path,"/",1)){
        strcat(full_path,current_path);
        path = path+1;
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
    //int tty_id = *(int*)ttyid;
    //printf("%s\n",ttyid);
    char buf[32];
    //printf("66666666666666666\n");
    //printf("this iddd is %s\n",ttyid);
    sprintf(buf, "/dev/%s",ttyid);
    //buf[8] = '0'+tty_id;
    buf[9] = '\0';
    //printf("%s\n",buf);
    int stdin = vfs->open(buf, O_RDONLY);
    int stdout = vfs->open(buf, O_WRONLY);
    char text[128];
    char line[128];
    char inter[3] = " $ ";
    ssize_t nread = 0;
    while (1) {
        if (nread!=0) {
            //printf("chrkk\n");
            if(line[nread-1]=='\n'){
                line[nread-1] = '\0';
                int cmd_cnt = 0; int flag = 0; 
                char cmd[32];char in_path[128];
                int path_cnt = 0;
                for(int i=0;i<nread-1;i++){
                    if(line[i]==' ' && !flag){
                        //cmd[i] = '\0';
                        flag = 1;
                    }else if(line[i]==' ' && flag){
                        vfs->write(stdout,"sorry we don't support it\n",6);
                    }else if(!flag){
                        cmd[i] = line[i];
                        cmd_cnt++;
                    }else{
                        in_path[path_cnt] = line[i];
                        path_cnt++;
                    }
                }
                //printf("cmd_int is %d and path_cnt is %d\n",cmd_cnt,path_cnt);
                cmd[cmd_cnt] = '\0';
                in_path[path_cnt] = '\0';
                if(!path_cnt){
                    strcpy(in_path,current_path);
                }
                path_translation(in_path);
                if(!strcmp(cmd,"ls")){
                    vfs->ls(full_path,stdout);
                }else if (!strcmp(cmd,"cd"))
                {   
                    if(!path_cnt)
                        vfs->cd("/",stdout);
                    else        
                        vfs->cd(full_path,stdout);
                    
                }else if(!strcmp(cmd,"mkdir")){
                    vfs->mkdir(full_path);
                }else if(!strcmp(cmd,"cat")){
                    printf("cattt\n");
                }else if(!strcmp(cmd,"rm")){
                    printf("rmmmmm\n");
                }else {
                    char err_info[128];
                    //memset(err_info,'\0',sizeof(err_info));
                    sprintf(err_info,"ksh : command not found %s\n",cmd);
                    vfs->write(stdout,err_info,strlen(err_info));
                }
                nread = 0;
            }
        } else {
            sprintf(text, "(%s) :", buf);
            vfs->write(stdout, text, strlen(text));
			vfs->write(stdout, current_path, strlen(current_path));
            //printf("shit\n");
			vfs->write(stdout, inter, strlen(inter));
            //printf("222\n");
            memset(line,'\0',sizeof(line));
            nread = vfs->read(stdin, line, sizeof(line));
            //printf("%d\n",nread);
            //printf("%s\n",stdin);
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