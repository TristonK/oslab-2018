#include "kvdb.h"

pthread_mutex_t mutex_lk = PTHREAD_MUTEX_INITIALIZER;


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
    strncpy(db->logname,filename,len-3);
    strcat(db->logname,".log");
    pthread_mutex_lock(&mutex_lk);
    if(db->opened == 1){
        pthread_mutex_unlock(&mutex_lk);
        return -1;
    }
    int datfd = open(db->dataname,O_RDWR|O_CREAT,0777);
    int logfd = open(db->logname,O_RDWR|O_CREAT,0777);
    pthread_mutex_init(&db->klock,PTHREAD_PROCESS_PRIVATE);
    db->dat_fd = datfd;
    db->log_fd = logfd;
    db->opened = 1;
    pthread_mutex_unlock(&mutex_lk);
    if(datfd<0||logfd<0) return min(datfd,logfd);
    return 0;
}

int kvdb_close(kvdb_t *db){
    pthread_mutex_lock(&mutex_lk);
    if(db->opened != 1){
        pthread_mutex_unlock(&mutex_lk);
        return -1;
    }
    int datfd = close(db->dat_fd);
    int logfd = close(db->log_fd);
    pthread_mutex_unlock(&mutex_lk);
    if(datfd<0||logfd<0) return min(datfd,logfd);
    return 0;
}
int kvdb_put(kvdb_t *db, const char *key, const char *value){

}
char *kvdb_get(kvdb_t *db, const char *key){}
