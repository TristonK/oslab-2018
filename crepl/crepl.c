#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>

char s_input[100000];
char exec_func[100000];
char func_name[1000];
int (*EXEC_FUNC)();

int main(int argc, char *argv[]) {
  int func_id = 1;
  char int_string[4];
  FILE *fp;
  if(fopen("/tmp/lab4.c","w")==NULL){
    printf("FAIL TO CREATE LAB4.C\n");
    assert(0);
  }
  strcpy(int_string,"int");
  while(1){
    if(fgets(s_input, sizeof(s_input),stdin)!=NULL){
      if(!strncmp(s_input,int_string,3)){
        /*Add a new function*/
        fp = fopen("/tmp/lab4.c","a+");
        if(fp==NULL){
          printf("FAIL to open file to add func\n");
          assert(0);
        }
        fprintf(fp,"%s",s_input);
        fclose(fp);
      } else{
        /*Expression*/
        memset(exec_func,'\0', sizeof(exec_func));
        memset(func_name,'\0', sizeof(func_name));
        sprintf(func_name,"__expr_wrap_%d()",func_id);
        sprintf(exec_func,"int %s{return %s;}",func_name,s_input);
        FILE* ex;
        ex = fopen("lab4exec.c","w");
        fclose(ex);
        ex = fopen("lab4exec.c","a+");
        fp = fopen("/tmp/lab4.c","r");
        char c;
        while((c = fgetc(fp)) != EOF)
        {
          fputc(c,ex);
        }
        fclose(ex);
        fclose(fp);
        ex = fopen("lab4exec.c","a+");
        fprintf(ex,"%s",exec_func);
        fclose(ex);
        system("gcc -shared -fPIC lab4exec.c -o /tmp/lab4exec.so -ldl");
        void *handle;
        char *error;
        handle = dlopen("/tmp/lab4exec.so", RTLD_LAZY);
        if(!handle){
          printf("ERROR:%s\n", dlerror());
          assert(0);
        }
        EXEC_FUNC = dlsym(handle, func_name);
        if((error=dlerror())!=NULL){
          printf("ERROR: %s\n",error);
        }
        printf(">> %s = %d.\n",s_input, (*EXEC_FUNC)());
        dlclose(handle);
      }
    }
  }
  return 0;
}
