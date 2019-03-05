#include <stdio.h>
#include <assert.h>
//test
int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // always true
		int op=(int)argv[i];
		switch(op){
			case '-p':
							printf("print pid\n");
							break;
			case '-n':
							printf("print num\n");
							break;
			default:
							printf("no such a op\n");
							break;
		}
    //printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // always true
  return 0;
}
