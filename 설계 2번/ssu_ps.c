#include "ssu_ps.h"

struct winsize term; //터미널 크기
proc_list *head;

long mem_tot;	//총 메모리 kb
time_t boot_time;		//오에스 부팅 타임
int a_flag, u_flag, x_flag;

int main(int argc, char *argv[]) 
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &term);

	printf("lines : %d\n", term.ws_row);
	printf("columns %d\n", term.ws_col);

	if (argc > 2) {
		fprintf(stderr, "usage : ./pps <option>\n");
		exit(1);
	}

	if (argc == 2 && check_option(argv) < 0) {
		fprintf(stderr, "option error in pps\n");
		exit(1);
	}
	
	boot_time = get_boot_time();
	mem_tot = get_total_memory();

	scan_proc();
	
}

int check_option(char **argv)
{
	int len = strlen(argv[1]);
	int i;

	for (i = 0; i < len; i++) {
		switch (argv[1][i]) {
		
		case 'a' :
			a_flag = true;
			break;

		case 'u' :
			u_flag = true;
			break;

		case 'x' :
			x_flag = true;
			break;

		default : 
			return -1;

		}
	}

	return 0;
}

void scan_proc(void) 
{
	int count;
	struct dirent **dirlist;
	struct passwd userpwd;
	struct stat status;
	char path[PATHLEN];
	char line[NAMELEN];
	char tmp[NAMELEN];
	proc_list pstat;

	if ((count = scandir("/proc", &dirlist, scandir_filter, NULL)) < 0) {
		fprintf(stderr, "scandir error in /proc\n");
		exit(1);
	}
	
	for (int i = 0; i < count && _isdigit(dirlist[i]->d_name); i++) {
		memset(path, 0, PATHLEN);
		sprintf(path, "%s/%s/%s", "/proc", dirlist[i]->d_name, "stat");
		
		FILE *fp = fopen(path, "r");

		memset(&pstat, 0, sizeof(proc_list));

		//stat파일에서 정보 읽어오기
		fscanf(fp, "%d %s %c %d %*d %*d %d %*d %*u %*u %*u %*u %*u %lu %lu"
				"%*d %*d %*d %*d %*d %*d %llu %lu %ld",
				&pstat.pid, pstat.cmdline, &pstat.state, &pstat.ppid, 
				&pstat.tty_nr, &pstat.utime, &pstat.stime, &pstat.starttime, 
				&pstat.vsize, &pstat.rss);
		fclose(fp);
		
		//uid에서 사용자 이름 구하기
		stat(path, &status);
		userpwd = *getpwuid(status.st_uid);
		strcpy(pstat.name, userpwd.pw_name);
		
		//cmdline 구하기

		memset(path, 0, PATHLEN);
		memset(tmp, 0, PATHLEN);
		memset(line, 0, NAMELEN);

		sprintf(path, "%s/%s/%s", "/proc", dirlist[i]->d_name, "cmdline");
		fp = fopen(path, "r");

		fread(line, NAMELEN, 1, fp);
		for (int i = 0; i < NAMELEN; i++) {
			if (line[i] == 0) {
				if (i == NAMELEN - 1)
					break;
				else if (line[i + 1] == 0)
					break;
				else
					line[i] = ' ';
			}
		}

		if (strlen(line) == 0) {
			pstat.cmdline[0] = '[';
			pstat.cmdline[strlen(pstat.cmdline) - 1] = ']';
		}
		else {
			memset(pstat.cmdline, 0, NAMELEN);
			strcpy(pstat.cmdline, line);
		}
		

	}
	
	for (int i = 0; i < count; i++)
		free(dirlist[i]);
	free(dirlist);

}


proc_list *create_node(proc_list stat)
{

}

time_t get_boot_time() 
{
	FILE *fp = fopen("/proc/stat", "r");
	time_t res;

	for (int i = 0; i < 5; i++) {
		fscanf(fp, "%*[^\n]\n");
	}
	fscanf(fp, "%*s %lu", &res);

	fclose(fp);
	return res;
}

long get_total_memory()
{
	FILE *fp = fopen("/proc/meminfo", "r");
	long res;

	fscanf(fp, "%*s %ld", &res);
	fclose(fp);
	return res;
}

int scandir_filter(const struct dirent *file) 
{
	char name[NAMELEN] = {0, };
	
	strcpy(name, file->d_name);

	if (_isdigit(name)) return true;
	else return false;
}


int _isdigit(char *str)
{
	int len = strlen(str);
	int i;

	for (i = 0; i < len; i++)
		if (!isdigit(str[i]))
			return false;

	return true;
}
