#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>    
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>

int fd;
char *buf;

int main(int argc, char *argv[]) {
  fd = open(argv[1],O_RDONLY);
  printf("You will recover %s\n",argv[1]);
  if(fd<0){
    printf("ERROR: YOU entered an bad addr!\n");
    assert(0);
  }
  buf = (char*) mmap(NULL,1<<27,PROT_READ,MAP_SHARED,fd,0);
  printf("in 0x0b is %x\n",buf[0x0b]);
  printf("in 0x0b+1 is %x\n",buf[0x0c]);
  printf("in 0xd is %x\n",buf[0x0d]);
  printf("in 0xe is %x\n",buf[0x0e]);
  printf("in 0xe+1 is %x\n",buf[0x0f]);
  printf("in 0x10 is %x\n",buf[0x10]);
  munmap((void*)buf,1<<27);
  close(fd);
  return 0;
}
