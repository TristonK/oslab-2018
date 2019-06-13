#include "kvdb.h"

pthread_mutex_t mutex_lk = PTHREAD_MUTEX_INITIALIZER;
char right[2]="1\t";
char shit[2] ="0\t";
char tab[1] = {'\t'};
char newline[1] = {'\n'};
char nothing[128]={'\0'};

int file_lock(int fd){
    struct flock flok;
    flok.l_type = F_WRLCK;
    flok.l_whence = SEEK_SET;
    flok.l_start = 0;
    flok.l_len = 0;
    flok.l_pid = getpid();
    if(fcntl(fd,F_SETLKW,&flok)==-1)
        return -1;
    return 0;
}

int file_unlock(int fd){
    struct flock flok;
    flok.l_type = F_UNLCK;
    flok.l_whence = SEEK_SET;
    flok.l_start = 0;
    flok.l_len = 0;
    flok.l_pid = getpid();
    if(fcntl(fd,F_SETLKW,&flok)==-1)
        return -1;
    return 0;
}


int kvdb_open(kvdb_t *db, const char *filename){
    int len = strlen(filename);
    if(filename[len-3]!='.'||filename[len-2]!='d'||filename[len-1]!='b'||len>256){
        return -1;
    }
    strcpy(db->dataname,filename);
    //strncpy(db->logname,filename,len-3);
    //strcat(db->logname,".log");
    pthread_mutex_lock(&mutex_lk);
    if(db->opened == 1){
        pthread_mutex_unlock(&mutex_lk);
        return -1;
    }
    int datfd=-1;
    if(access(filename,F_OK)==0){
        datfd = open(db->dataname,O_RDWR|O_CREAT,0777);
        file_lock(datfd);
        pthread_mutex_init(&db->klock,PTHREAD_PROCESS_PRIVATE);
        db->dat_fd = datfd;
        lseek(datfd,0,SEEK_SET);
        char* judge = malloc(2);
        if(read(datfd,judge,2)==0||strcmp(judge,right)){
            //TODO: check the crash and recover it
            printf("shit!\n");
        }
        free(judge);
        
        db->opened = 1;
        file_unlock(datfd);
    }else{
        int datfd = open(db->dataname,O_RDWR|O_CREAT,0777);
        file_lock(datfd);
        pthread_mutex_init(&db->klock,PTHREAD_PROCESS_PRIVATE);
        db->dat_fd = datfd;
        db->opened = 1;
        write(datfd,(void*)right,2);
        file_unlock(datfd);
    }
    pthread_mutex_unlock(&mutex_lk);
    if(datfd<0) return datfd;
    return 0;
}

int kvdb_close(kvdb_t *db){
    pthread_mutex_lock(&mutex_lk);
    file_lock(db->dat_fd);
    if(db->opened != 1){
        pthread_mutex_unlock(&mutex_lk);
        return -1;
    }
    int datfd = close(db->dat_fd);
    db->dat_fd = 0;
    file_unlock(datfd);
    pthread_mutex_unlock(&mutex_lk);
    if(datfd<0) return datfd;
    return 0;
}
int kvdb_put(kvdb_t *db, const char *key, const char *value){
    pthread_mutex_lock(&db->klock);
    file_lock(db->dat_fd);
    int keylen = strlen(key);
    if(keylen>128)
        return -1;
    int valuelen = strlen(value);
    char lenn[16];
    itoa(valuelen,lenn,10);
    lseek(db->dat_fd,0,SEEK_SET);
    write(db->dat_fd,(void *)shit,2);
    write(db->dat_fd,(void*)key,keylen);
    write(db->dat_fd,(void*)nothing,128-keylen);
    //write(db->dat_fd,(void*)tab,1);
    write(db->dat_fd,(void*)(&lenn),16);
    write(db->dat_fd,(void*)value,valuelen);
    write(db->dat_fd,(void*)newline,1);
    sync();
    lseek(db->dat_fd,0,SEEK_SET);
    write(db->dat_fd,(void*)right,2);
    sync();
    file_unlock(db->dat_fd);
    pthread_mutex_unlock(&db->klock);
    return 0;
}
char *kvdb_get(kvdb_t *db, const char *key){
    pthread_mutex_lock(&db->klock);
    file_lock(db->dat_fd);
    int keylen = strlen(key);
    char* keyy = (char*)malloc(132);
    char* lenn = (char*)malloc(16);
    char* val;
    lseek(db->dat_fd,2,SEEK_SET);
    int flag=0;
    while(read(db->dat_fd,keyy,128)!=0){
        read(db->dat_fd,lenn,16);
        int len= atoi(lenn);
        if(strncmp(keyy,key,keylen)==0){
            val = malloc(len);
            flag = 1;
            read(db->dat_fd,val,len);
            lseek(db->dat_fd,1,SEEK_CUR);
        }   else
        {
            lseek(db->dat_fd,len+1,SEEK_CUR);
        }
    }
    free(keyy);
    free(lenn);
    file_unlock(db->dat_fd);
    pthread_mutex_unlock(&db->klock);
    if(!flag)
        return NULL;
    return val;
}
