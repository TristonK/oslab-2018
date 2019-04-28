#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
int fildes[2];
struct proc{
    char proc_name[32];
    double proc_time;
    int proc_rate;
};
struct proc p[1024];
int proc_cnt=0;
extern char**environ;

void phase_st(char phase[1024]){
    char temp_name[32];
    char temp_time[32];
    int i=0;
    //printf("%s\n",phase);
    while (phase[i]!='('){
        temp_name[i]=phase[i];
        i++;
        if(i>=32)
            return;
    }
    temp_name[i]='\0';
    i=strlen(phase);
    while(phase[i]!='<'){
        i--;
        if(i<0)
            return;
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
    if(temp_timeval<1e-6)
        return;
    int have_it = 0;
    for(int j=0;j<proc_cnt;j++){
        if(strcmp(temp_name,p[j].proc_name)==0){
            have_it=1;
            p[j].proc_time+=temp_timeval;
            break;
        }
    }
    if(!have_it){
        strcpy(p[proc_cnt].proc_name,temp_name);
        //p[proc_cnt].proc_name = temp_name;
        p[proc_cnt].proc_time = temp_timeval;
        proc_cnt++;
    }
}

void paint_line(){
    double sum_time=0;
    for(int i=0;i<proc_cnt;i++){
        sum_time+=p[i].proc_time;
    }
    for(int i=0;i<proc_cnt;i++){
        p[i].proc_rate = (int)(p[i].proc_time*100/sum_time + 0.5);
    }
    for(int i=0;i<proc_cnt-1;i++){
        for(int j=i+1;j<proc_cnt;j++){
            if(p[i].proc_rate < p[j].proc_rate){
                struct proc temp;
                temp = p[i];
                p[i]=p[j];
                p[j]=temp;
            }
        }
    }
    int sum_rate=0;
   // printf("proc_cnt is %d\n",proc_cnt);
   int judge_color=0;
   for(int i=0;i<proc_cnt;i++){
        if(p[i].proc_rate){
            switch (judge_color){
                case 0:
                    printf("\x1b[42m%s:%10d%%",p[i].proc_name,p[i].proc_rate);
                    printf("\x1b[0m\n");
                    break;
                case 1:
                    printf("\x1b[43m%s:%10d%%",p[i].proc_name,p[i].proc_rate);
                    printf("\x1b[0m\n");
                    break;
                case 2:
                    printf("\x1b[44m%s:%10d%%",p[i].proc_name,p[i].proc_rate);
                    printf("\x1b[0m\n");
                    break;
                default:
                    printf("\x1b[46m%s:%10d%%",p[i].proc_name,p[i].proc_rate);
                    printf("\x1b[0m\n");
            }
            //printf("%s:  %d%%\n",p[i].proc_name,p[i].proc_rate);
            sum_rate+=p[i].proc_rate;
            judge_color=(judge_color+1)%4;
        }
    }
    if(sum_rate<100){
        printf("\x1b[45mothers:  %10d%%",100-sum_rate);
    }
    printf("\x1b[0m\n");
}

int main(int argc, char *argv[]) {
  char *input_op[50]={"strace","-T"};
  //char *environ[2] = {"PATH=/bin",NULL};
  for(int i=1;i<argc;i++){
    input_op[i+1]=argv[i];
  }
  if(pipe(fildes)!=0){
    printf("Fail to create pipe\n");
    assert(0);
  }
  /*for(int i=0;i<argc+2;i++){
      printf("%s  ",input_op[i]);
  }
  printf("\n");*/
  pid_t pid = fork();
  if(!pid){
        //printf("hi is son\n");
        close(fildes[0]);
        if(dup2(fildes[1],STDERR_FILENO)==-1){
            printf("SOMETHING WRONG ON CLOSE OUTPUT\n");
        }
        //printf("\x1b[44mA\n");
        //printf("\x1b[32mHello, World\\n")
        execve("/usr/bin/strace",input_op,environ);
        printf("You shouldn't go here\n");
        assert(0);
  } else{
        close(fildes[1]);
        dup2(fildes[0],STDIN_FILENO);
        //printf("father is here\n");
        char read_info[1024];
        while(fgets(read_info, sizeof(read_info),stdin)!=NULL){
            //printf("%s",read_info;
            phase_st(read_info);
        }
        //printf("end read\n");
        paint_line();
  }
  return 0;
}
