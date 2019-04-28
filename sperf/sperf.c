#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
int fildes[2];
char proc_name[128][32];
double proc_time[128];
double proc_rate[128];
int proc_cnt=0;


void phase_st(char phase[1024]){
    char temp_name[32];
    char temp_time[32];
    int i=0;
    while (phase[i]!='('){
        temp_name[i]=phase[i];
        i++;
    }
    temp_name[i]='\0';
    i=strlen(phase);
    while(phase[i]!='<'){
        i--;
    }
    i++;
    int time_cnt=0;
    while(phase[i]!='>'){
        temp_time[time_cnt]=phase[i];
        i++;
        time_cnt++;
    }
    temp_time[time_cnt]='\0';
    double temp_timeval = atof(temp_time);
    int have_it = 0;
    for(int j=0;j<proc_cnt;j++){
        if(strcmp(temp_name,proc_name[j])==0){
            have_it=1;
            proc_name[j]+=temp_timeval;
            break;
        }
    }
    if(!have_it){
        
    }

}

int main(int argc, char *argv[]) {
  char *input_op[50]={"strace","-T"};
  char *environ[2] = {"PATH=/bin",NULL};
  for(int i=1;i<argc;i++){
    input_op[i+1]=argv[i];
  }
  if(pipe(fildes)!=0){
    printf("Fail to create pipe\n");
    assert(0);
  }
  pid_t pid = fork();
  if(!pid){
    //printf("hi is son\n");
    close(fildes[0]);
    dup2(fildes[1],2);
    //printf("\x1b[44mA\n");
    //printf("\x1b[32mHello, World\\n")
    execve("/usr/bin/strace",input_op,environ);
    printf("You shouldn't go here\n");
    assert(0);
  } else{
    close(fildes[1]);
    dup2(fildes[0],0);
    char read_info[1024];
    while(fgets(read_info, sizeof(read_info),stdin)!=NULL){

      printf("%s",read_info);
    }
  }




  return 0;
}
