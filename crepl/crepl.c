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
  printf(">> ");
  while(1){
    if(fgets(s_input, sizeof(s_input),stdin)!=NULL){
      if(!strncmp(s_input,int_string,3)){
        /*Add a new function*/
        fp = fopen("/tmp/m4check_func.c","w");
        fprintf(fp,"%s",s_input);
        fclose(fp);
      #ifdef __x86_64__
        // printf("__x86_64__");
          if(system("gcc -shared -fPIC /tmp/m4check_func.c -o /tmp/m4check_func.so -ldl")!=0){
            printf("\x1b[31mCompile Error\n");
            printf("\x1b[0m");
            continue;
        }
          #elif __i386__
        //printf("__i386__");
          if(system("gcc -m32 -shared -fPIC /tmp/m4check_func.c -o /tmp/m4check_func.so -ldl")!=0){
            printf("\x1b[31mCompile Error\n");
            printf("\x1b[0m");
            continue;
          }
        #endif
        fp = fopen("/tmp/lab4.c","a+");
        if(fp==NULL){
          printf("FAIL to open file to add func\n");
          assert(0);
        }
        fprintf(fp,"%s",s_input);
        fclose(fp);
        printf("\x1b[32mAdded: ");
        printf("\x1b[0m%s",s_input);
        printf(">> ");
      } else{
        /*Expression*/
        memset(exec_func,'\0', sizeof(exec_func));
        memset(func_name,'\0', sizeof(func_name));
        if(s_input[strlen(s_input)-1]=='\n')
          s_input[strlen(s_input)-1]='\0';
        sprintf(func_name,"__expr_wrap_%d",func_id);
        sprintf(exec_func,"int %s(){return %s;}",func_name,s_input);
        FILE* ex;
        ex = fopen("/tmp/lab4exec.c","w");
        fclose(ex);
        ex = fopen("/tmp/lab4exec.c","a+");
        fp = fopen("/tmp/lab4.c","r");
        char c;
        while((c = fgetc(fp)) != EOF)
        {
          fputc(c,ex);
        }
        fclose(ex);
        fclose(fp);
        ex = fopen("/tmp/lab4exec.c","a+");
        fprintf(ex,"%s",exec_func);
        fclose(ex);
        #ifdef __x86_64__
         // printf("__x86_64__");
          if(system("gcc -shared -fPIC /tmp/lab4exec.c -o /tmp/lab4exec.so -ldl")!=0){
            printf("\x1b[31mCompile Error Or No such Function\n");
            printf("\x1b[0m");
            continue;
        }
        #elif __i386__
          //printf("__i386__");
          if(system("gcc -m32 -shared -fPIC /tmp/lab4exec.c -o /tmp/lab4exec.so -ldl")!=0){
            printf("\x1b[31mCompile Error Or No such Function\n");
            printf("\x1b[0m");
            continue;
          }
          #endif

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
        printf("%s = %d.\n",s_input, (*EXEC_FUNC)());
        printf(">> ");
        dlclose(handle);
      }
    }
  }
  return 0;
}
