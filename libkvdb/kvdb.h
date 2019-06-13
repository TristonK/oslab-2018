#ifndef __KVDB_H__
#define __KVDB_H__

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<fcntl.h>

struct kvdb {
    int dat_fd;
    //int log_fd;
    int opened;
    char dataname[256];
    //char logname[256];
    struct flock datlock;
    pthread_mutex_t klock;
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif
