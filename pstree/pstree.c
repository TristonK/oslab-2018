#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

//variables
char proc_path[100];
char buf[1024];

void read_proc(){
	DIR *dirptr = NULL;
	struct dirent *entry;
	dirptr = opendir("/proc");
	assert(dirptr != NULL);
	while((entry = readdir(dirptr))){
		if(entry->d_type == DT_DIR && entry->d_name[0] <=57 && entry->d_name[0] >= 48){
				strcpy(proc_path,"/proc/");
				strcat(proc_path,entry->d_name);
				strcat(proc_path,"/status");
				//printf("%s\n",proc_path);
				FILE *fp = fopen(proc_path,"r");
				if(fp){
					printf("**********************\n");
					while(!feof(fp)){
						char name[100];
						fgets(buf,1024,fp);
						if(!strncmp(buf,"Name",4)){
							bool flag=false;int k=0;
							for(int i=5;i<strlen(buf);++i){
								if(flag){
									name[k] = buf[i];
									k++;
									continue;
								}
								if(buf[i]!=' ')
									flag = true;
							}
							printf("%s\n",name);
							printf("%s",buf);
						}
						if(!strncmp(buf,"Pid",3)){
							printf("%s",buf);
						}
						if(!strncmp(buf,"PPid",4)){
							printf("%s",buf);
						}
					}
					fclose(fp);
				}
				else{
					printf("ERROR: Fail To Open %s\n",proc_path);
					assert(0);
				}	
			}	
	}
	closedir(dirptr);
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
