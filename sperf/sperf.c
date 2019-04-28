#include <stdio.h>
#include <string.h>
//int pipe(int fildes[2]);

int main(int argc, char *argv[]) {
  printf("argc is %d\n",argc);
  printf("%s\n",argv[0]);
  /*pid_t pid = fork();
  if(!pid){
    printf("hi is son\n");
    execve("/use/bin/strace",["strace","-T"]);
  } else{
    printf("father is here\n");
  }*/




  return 0;
}
