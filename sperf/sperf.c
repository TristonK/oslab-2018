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
    //printf("hi is son\n");
    close(fildes[0]);
    dup2(fildes[1],2);
    printf("\x1b[44mA\n");
    //printf("\x1b[32mHello, World\\n")
    execve("/usr/bin/strace",input_op,environ);
    printf("You shouldn't go here\n");
    assert(0);
  } else{
    close(fildes[1]);
    dup2(fildes[0],0);
    char read_info[10240];
    while(fgets(read_info, sizeof(read_info),stdin)!=NULL){
      printf("%s",read_info);
    }
  }




  return 0;
}
