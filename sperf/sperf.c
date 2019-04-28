#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
int fildes[2];

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
    printf("hi is son\n");
    close(fildes[0]);
    dup2(fildes[1],2);
    for(int i=0;i<argc+2;i++){
      printf("input_op %d is %s\n",i,input_op[i]);
    }
    execve("/use/bin/strace",input_op,environ);
    printf("we shouldnt go here]\n");
  } else{
    close(fildes[1]);
    dup2(fildes[0],0);
    char read_info[50000];
    fgets(read_info, sizeof(read_info),0);
    printf("%s\n",read_info);
    printf("father is here\n");
  }




  return 0;
}
