#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>    
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>

int fd;
char *buf;
struct BPB{
  //.... have more but we dont need it
  unsigned char unused[11];
  unsigned short bytsPerSec; //0xb
  unsigned char SecPerClus;   //0xd
  unsigned short ReservedSec; //0xe
  unsigned char FatNum; //0x10
  unsigned short RootEnt;//0x11  should be 0
  unsigned short SmallSec;//0x13 be 0;
  unsigned char MediaDescri;
  unsigned short SecPerFat_0; //0x16 always be 0
  unsigned short SecPerTrack; //0x18
  unsigned short NumHeads;//0x1a
  unsigned int HiddenSec; //0x1c
  unsigned int LargeSec; //0x20
  unsigned int SecPerFat; //0x24
  //.... have more but we dont need it
}__attribute__((packed));
struct SmallDir{
  unsigned char Name[11];
  unsigned char Attr;
  unsigned char Reserved;
  unsigned char CrTimeLen;
  unsigned short CrTime;
  unsigned short CrDate;
  unsigned short LastVisTime;
  unsigned short HAddr;//21 22
  unsigned short LastTime;
  unsigned short LastDate;
  unsigned short LAddr;
  unsigned int Length; 
}__attribute__((packed));
struct LargeDir{
  unsigned char Attr;
  unsigned char Name[10];
  unsigned char flag;
  unsigned char reserved;
  unsigned char hash;
  unsigned char Name2[10];
  unsigned short BeginClus;
  unsigned char Name3[4];
}__attribute__((packed));

struct BPB bpb;
uintptr_t RanfFByte;
uintptr_t BytsPerClus;
uintptr_t DirPerClus;
int NumClus;

void get_info(){
  memcpy(&bpb,buf,sizeof(bpb));
  /*printf("each sec has byte %d\n",bpb.bytsPerSec);
  printf("each Cluster has %d sec\n",bpb.SecPerClus);
  printf("you reserve %d sec\n",bpb.ReservedSec);
  printf("you have %u Fat\n",bpb.FatNum);
  printf("0x10 is %d\n",buf[0x10]);*/
  RanfFByte = (bpb.bytsPerSec)*(bpb.SecPerFat*bpb.FatNum+ bpb.ReservedSec);
  BytsPerClus = bpb.SecPerClus * bpb.bytsPerSec;
  NumClus = (bpb.LargeSec- RanfFByte/bpb.bytsPerSec)/bpb.SecPerClus;
  DirPerClus = BytsPerClus/32;
}

void find_bmp(){
  struct SmallDir dir;
  char fname[256];
  //printf("jj\n");
  //printf("dir size is %d and bpb size is %d\n",(int)sizeof(dir),(int)sizeof(bpb));
  printf("%x %x %x %x\n",(int)RanfFByte,(int)NumClus,(int)DirPerClus,(int)BytsPerClus);
  for(int i=0;i<NumClus;i++){
    for(int j=0;j<DirPerClus;j++){
      uintptr_t ofset = RanfFByte+i*BytsPerClus+j*32;
      if(buf[ofset+0xa]==0x0f){
        //printf("long file\n");
      }else{
          memcpy(&dir,buf+ofset,sizeof(dir));
          if(dir.Name[8]=='B'&&dir.Name[9]=='M'&&dir.Name[10]=='P'){
            //***********check
            unsigned int beginclus = (dir.HAddr<<16)|(dir.LAddr);
            if(beginclus<2)
              continue;
            beginclus-=2;
            uintptr_t pic_data = beginclus*BytsPerClus + RanfFByte;
            if(buf[pic_data]!='B'||buf[pic_data+1]!='M')
              continue;
            int picsize = *(int *)&buf[pic_data+2];
            if(dir.Length!=picsize)
              continue;
            //*********** get-name
            if(dir.Name[6]=='~'){
              struct LargeDir ldir;
              uintptr_t newofset = ofset-32;
              memset(fname,'\0',sizeof(fname));
              int namecnt = 0;
              int flag = 1;
              do{
                memcpy(&ldir,buf+newofset,sizeof(ldir));
                if(ldir.flag!=0x0f)
                  break;
                for(int i=0;i<5;i++){
                  if(ldir.Name[2*i]!=0xffff&&flag){
                    if(ldir.Name[2*i]==' '||ldir.Name[2*i]==0x2e){
                      break;
                    }
                    fname[namecnt] = ldir.Name[2*i];
                    namecnt++;
                  }
                }
                for(int i=0;i<5;i++){
                  if(ldir.Name2[2*i]!=0xffff&&flag){
                    if(ldir.Name2[2*i]==' '||ldir.Name2[2*i]==0x2e){
                      break;
                    }
                    fname[namecnt] = ldir.Name2[2*i];
                    namecnt++;
                  }
                }
                for(int i=0;i<2;i++){
                  if(ldir.Name3[2*i]!=0xff&&flag){
                    if(ldir.Name3[2*i]==' '||ldir.Name3[2*i]==0x2e){
                      break;
                    }
                    fname[namecnt] = ldir.Name3[2*i];
                    namecnt++;
                  }
                }
                newofset-=32;
              }while (ldir.Attr<0x40);
              strcat(fname,".bmp");
              //printf("%s\n",fname);
            } else{
              memcpy(&fname,dir.Name,11);
              for(int i=0;i<8;i++){
              if(fname[i]==' ')
                fname[i] = '\0';
              }
              strcat(fname,".bmp");
              fname[11] = '\0';
              //printf("%s\n",fname);
            }
            if(fname[0]=='\0'||fname[0]=='.')
              continue;
            printf("%s 's addr is %x and data in %x\n",fname,(int)ofset,(int)pic_data);        
            //*****************
            char *picname = malloc(100);
            strcat(picname,"/home/Documents/");
            strcat(picname,fname);
            printf("%s\n",picname);
            int fdpic = open(picname,O_RDWR|O_CREAT|O_TRUNC,0777);
            pwrite(fdpic,buf+pic_data,picsize,0);
            close(fdpic);
          } 
      }
    }
  }
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
  find_bmp();
  //*******Stop work
  munmap((void*)buf,1<<27);
  close(fd);
  return 0;
}
