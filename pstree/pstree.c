#include <stdio.h>
#include <assert.h>
#include <string.h>
//test
int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // always true
		if(!strcmp(argv[i],"-p") || !strcmp(argv[i],"--show-pids"))
			printf("print pids\n");
		else if(!strcmp(argv[i],"-n") || !strcmp(argv[i],"--numeric-sort"))
			printf("sort\n");
		else if(!strcmp(argv[i],"-V") || !strcmp(argv[i],"--version"))
		  printf("pstree (kuangsl) 1.0\n Copyright (C) 2019-2019 what a sad lab\n");
    //printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // always true
  return 0;
}
