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
struct BPB{
  //.... have more but we dont need it
  unsigned short bytsPerSec; //0xb
  char SecPerClus;   //0xd
  unsigned short ReservedSec; //0xe
  char FatNum; //0x10
  unsigned short RootEnt;//should be 0
  unsigned short SmallSec;// be 0;
  char MediaDescri;
  unsigned short SecPerFat_0; //0x16 always be 0
  unsigned short SecPerTrack; //0x18
  unsigned short NumHeads;//0x1a
  unsigned int HiddenSec;
  unsigned int LargeSec;
  unsigned int SecPerFat;
  //.... have more but we dont need it
};

struct BPB bpb;

void get_info(){
  memcpy(&bpb,buf+0xb,sizeof(bpb));
  printf("each sec has byte %d\n",bpb.bytsPerSec);
  printf("each Cluster has %d sec\n",bpb.SecPerClus);
  printf("you reserve %d sec\n",bpb.ReservedSec);
  printf("you have %d Fat\n",bpb.FatNum);
}






int main(int argc, char *argv[]) {
  //*******read file img
  fd = open(argv[1],O_RDONLY);
  //printf("You will recover %s\n",argv[1]);
  if(fd<0){
    printf("ERROR: YOU entered an bad addr!\n");
    assert(0);
  }
  buf = (char*) mmap(NULL,1<<27,PROT_READ,MAP_SHARED,fd,0);
  //*******recovery
  get_info();
  
  //*******Stop work
  munmap((void*)buf,1<<27);
  close(fd);
  return 0;
}
