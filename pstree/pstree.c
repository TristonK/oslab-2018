#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

//variables
int proc_num=0;
struct Proc{
	char name[100];
	int pid,ppid;
	int print,generation;
}p[1024];

void get_str(char* ans,int start_pos,char buf[1024]){
	char str[100];
	int flag = 0;int k=0;
	for(int i=start_pos;i<strlen(buf);++i){
		if(flag){
			str[k] = buf[i];
			k++;
			continue;
		}
		if(buf[i]!='\t' && buf[i]!=' '){
			flag = 1;
			str[k] = buf[i];
			k++;
		}
	}
	str[k]='\0';
	strcpy(ans,str);	
}

void read_proc(){
	char proc_path[100];
  char buf[1024];
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
					char name[100],pid_str[100],ppid_str[100];
					int pid,ppid;
					while(!feof(fp)){
						fgets(buf,1024,fp);
						if(!strncmp(buf,"Name",4)){
							get_str(name,5,buf);
							strcpy(p[proc_num].name,name);
						}
						if(!strncmp(buf,"Pid",3)){
							get_str(pid_str,4,buf);
							pid = atoi(pid_str);
							p[proc_num].pid=pid;
							//printf("%d\n",pid);
						}
						if(!strncmp(buf,"PPid",4)){
							get_str(ppid_str,5,buf);
							ppid = atoi(ppid_str);
							p[proc_num].ppid=ppid;
							//printf("%d\n",ppid);
						}
					}
					p[proc_num].print = p[proc_num].generation = 0;
					fclose(fp);
				}
				else{
					printf("ERROR: Fail To Open %s\n",proc_path);
					assert(0);
				}
				proc_num++;	
			}	
	}
	closedir(dirptr);
	//printf("%d\n",proc_num);
}

void print_tree(int ppid,int father_num){
		for(int i=0; i < proc_num; ++i){
			if(!p[i].print && p[i].ppid==ppid){
				for(int j=0;j<father_num;j++){
					printf("|    ");
				}
				printf("|——");
				printf("%s",p[i].name);
				p[i].print = 1;
				p[i].generation = father_num+1;
				print_tree(p[i].pid,p[i].generation);
			}
		}
}





int main(int argc, char *argv[]) {
  int i;
	read_proc();
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // always true
		if(!strcmp(argv[i],"-p") || !strcmp(argv[i],"--show-pids"))
			printf("print pids\n");
		else if(!strcmp(argv[i],"-n") || !strcmp(argv[i],"--numeric-sort"))
			printf("sort\n");
		else if(!strcmp(argv[i],"-V") || !strcmp(argv[i],"--version"))
		  printf("pstree (kuangsl) 1.0\nCopyright (C) 2019-2019 what a sad lab\n");
		else
		 	print_tree(0,0);
  }
	
  assert(!argv[argc]); // always true
  return 0;
}
