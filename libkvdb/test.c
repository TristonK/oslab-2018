#include "kvdb.h"
#include <stdlib.h>

char *key;
char *mima;
char *name;
int keylen;

void getkey(){
	key = malloc(128);
	mima = malloc(100000000);
	keylen = rand()%128+1;
	int milen = rand()%10000+keylen;
	for(int i=0;i<keylen;i++){
		int k = rand()%26;
		*(key+i) = 'a'+k;
		*(mima+i) = 'a'+k;
	}
	for(int i= keylen;i<milen;i++){
		int k = rand()%26;
		*(mima+i) = 'a'+k;
	}
}


int main() {
	for(int i=0;i<4;i++)
		fork();
	kvdb_t db;
	//const char *key = "operating-systems";
	char *value;
	name = malloc(4);
	*(name) = 'a'+getpid()%26;
	strcat(name,".db");
	kvdb_open(&db, name); // BUG: should check for errors
	getkey();
	kvdb_put(&db, key, mima);
	value = kvdb_get(&db, key);
	kvdb_close(&db);
	if(strncmp(key,value,keylen)!=0)
		printf("bbbbad\n");
	//printf("[%s]: [%s]\n", key, value);
	free(value);
	return 0;
}
