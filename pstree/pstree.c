#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

//variables



void read_proc(){
	DIR *dirptr = NULL;
	struct dirent *entry;
	dirptr = opendir("/proc");
	//assert(dirptr == NULL);
	while((entry = readdir(dirptr))){
		if(entry->d_type == DT_DIR && entry->d_name[0] <=57 && entry->d_name >= 48)
			printf("%s\n",entry->d_name);		
	}
}


int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // always true
		if(!strcmp(argv[i],"-p") || !strcmp(argv[i],"--show-pids"))
			printf("print pids\n");
		else if(!strcmp(argv[i],"-n") || !strcmp(argv[i],"--numeric-sort"))
			printf("sort\n");
		else if(!strcmp(argv[i],"-V") || !strcmp(argv[i],"--version"))
		  printf("pstree (kuangsl) 1.0\nCopyright (C) 2019-2019 what a sad lab\n");
		else
		 	printf("pstree\n");
  }
	read_proc();
  assert(!argv[argc]); // always true
  return 0;
}
