#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

/*TODO:
* 2. nemuric sort is wrong
* 3. modify the line in the tree
*/

//variables
int proc_num = 0;
int print_pid = 0;
int numeric_sort = 0;
int line[1024];
struct Proc
{
	char name[100];
	int pid, ppid;
	int print, generation;
	int type;
	struct Proc *child;
	struct Proc *sibling;
} p[1024];

void get_str(char *ans, int start_pos, char buf[1024])
{
	char str[100];
	int flag = 0;
	int k = 0;
	for (int i = start_pos; i < strlen(buf); ++i)
	{
		if (flag)
		{
			str[k] = buf[i];
			k++;
			continue;
		}
		if (buf[i] != '\t' && buf[i] != ' ')
		{
			flag = 1;
			str[k] = buf[i];
			k++;
		}
	}
	str[k - 1] = '\0';
	strcpy(ans, str);
}

void read_proc()
{
	char proc_path[100];
	char thread_path[100];
	char buf[1024];
	DIR *dirptr = NULL;
	struct dirent *entry;
	dirptr = opendir("/proc");
	assert(dirptr != NULL);
	while ((entry = readdir(dirptr)))
	{
		if (entry->d_type == DT_DIR && entry->d_name[0] <= 57 && entry->d_name[0] >= 48)
		{
			strcpy(proc_path, "/proc/");
			strcat(proc_path, entry->d_name);
			strcat(proc_path, "/task");
			DIR *dir_thread = NULL;
			struct dirent *entry_thread;
			dir_thread = opendir(proc_path);
			if(dir_thread==NULL){
				printf("fail to open %s\n",proc_path);
				assert(dir_thread != NULL);
			}
			while ((entry_thread = readdir(dir_thread)))
			{
				if (entry_thread->d_type == DT_DIR && entry_thread->d_name[0] <= 57 && entry_thread->d_name[0] >= 48)
				{
					strcpy(thread_path, proc_path);
					strcat(thread_path, "/");
					strcat(thread_path, entry_thread->d_name);
					strcat(thread_path, "/status");
					FILE *fp = fopen(thread_path, "r");
					if (fp)
					{
						char name[100], pid_str[100], ppid_str[100], tgid_str[100];
						int pid = 0, ppid = 0, tgid = 0;
						while (!feof(fp))
						{
							fgets(buf, 1024, fp);
							if (!strncmp(buf, "Name", 4))
							{
								get_str(name, 5, buf);
								//strcpy(p[proc_num].name,name);
							}
							if (!strncmp(buf, "Pid", 3))
							{
								get_str(pid_str, 4, buf);
								pid = atoi(pid_str);
								//p[proc_num].pid=pid;
								//printf("%d\n",pid);
							}
							if (!strncmp(buf, "PPid", 4))
							{
								get_str(ppid_str, 5, buf);
								ppid = atoi(ppid_str);
								//p[proc_num].ppid=ppid;
								//printf("%d\n",ppid);
							}
							if (!strncmp(buf, "Tgid", 4))
							{
								get_str(tgid_str, 5, buf);
								tgid = atoi(tgid_str);
							}
						}
						if (ppid != 2 && pid != 2)
						{
							strcpy(p[proc_num].name, name);
							p[proc_num].pid = pid;
							p[proc_num].type = 0;
							if (tgid == pid)
							{ //不是子线程
								p[proc_num].ppid = ppid;
								p[proc_num].type = 1;
							}
							else
							{ //子线程
								p[proc_num].ppid = tgid;
								for (int i = 0; i <= proc_num; i++)
								{
									if (p[i].pid == tgid)
									{
										strcpy(p[proc_num].name, p[i].name);
										break;
									}
								}
							}
							p[proc_num].print = p[proc_num].generation = 0;
							proc_num++;
						}
						fclose(fp);
					}
					else
					{
						printf("ERROR: Fail To Open %s\n", thread_path);
						assert(0);
					}
				}
			}
		}
	}
	closedir(dirptr);
	//printf("%d\n",proc_num);
}

void sort_tree()
{
	for (int i = 0; i < proc_num; ++i)
	{
		for (int j = i + 1; j < proc_num; ++j)
		{
			if (numeric_sort)
			{
				/*NEED CHANGE*/
				if (p[i].pid > p[j].pid)
				{
					struct Proc temp;
					temp = p[i];
					p[i] = p[j];
					p[j] = temp;
				}
			}
			else
			{
				if (strcmp(p[i].name, p[j].name) > 0)
				{
					struct Proc temp;
					temp = p[i];
					p[i] = p[j];
					p[j] = temp;
					//temp.name = p[i].name;temp.pid = a.pid;temp.ppid=a.ppid;
					//temp.print=p[i].print;temp.generation=p[i].generation;
					//temp.type=p[i].type;temp.child=p[i].child;temp.sibling=p[i].sibling;
				}
				else if(strcmp(p[i].name,p[j].name)==0 && p[i].pid>p[j].pid){
					struct Proc temp;
					temp = p[i];
					p[i] = p[j];
					p[j] = temp;
				}
			}
		}
	}
}

void print_tree(int ppid, int father_num)
{
	for (int i = 0; i < proc_num; ++i)
	{
		if (!p[i].print && p[i].ppid == ppid)
		{ 
			for (int j = 0;j<proc_num;j++){
				if(p[j].pid == p[i].ppid){
					for(int k=j;k<proc_num;k++){
						if(!p[k].print&&k!=i&&p[k].ppid==p[j].pid){
							line[father_num]=1;
							break;
						}
						else{
							line[father_num]=0;
						}
					}
					break;
				}
			}
			for (int j = 0; j < father_num; j++)
			{
				if(line[j])
					printf("|    ");
				else
				{
					printf("     ");
				}
				
			}
			if(line[father_num])
				printf("|——");
			else
				printf("└─—")
			if (!p[i].type)
			{
				if (print_pid == 1)
				{
					printf("{%s}(%d)\n", p[i].name, p[i].pid);
				}
				else
					printf("{%s}\n", p[i].name);
			}
			else
			{
				if (print_pid == 1)
				{
					printf("%s(%d)\n", p[i].name, p[i].pid);
				}
				else
					printf("%s\n", p[i].name);
			}
			p[i].print = 1;
			p[i].generation = father_num + 1;
			print_tree(p[i].pid, p[i].generation);
		}
	}
}

int main(int argc, char *argv[])
{
	int i;
	read_proc();
	int print_version = 0;
	for (i = 0; i < argc; i++)
	{
		assert(argv[i]); // always true
		if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--show-pids"))
			print_pid = 1;
		else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--numeric-sort"))
			numeric_sort = 1;
		else if (!strcmp(argv[i], "-V") || !strcmp(argv[i], "--version"))
			print_version=1;
	}
	if(print_version){
		printf("pstree (kuangsl) 1.0\nCopyright (C) 2019-2019 what a sad lab\n");}
	else{
		sort_tree();
		print_tree(0, 0);
	}
	assert(!argv[argc]); // always true
	return 0;
}
