#include "ssu_ps.h"

struct winsize term; 	//터미널 크기
proc_list *head;

long mem_tot;		//총 메모리 kb
time_t boot_time;	//오에스 부팅 타임
double uptime;
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
	head = create_node(NULL);
	
	scan_proc();
	print_ps();

	free_list(head);
	exit(0);
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
	char line[LINEMAX];
	char tmp[NAMELEN];
	proc_list pstat;
	proc_list *node;

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
		fscanf(fp, "%d %s %c %d %d %d %d %d %*u %*u %*u %*u %*u %lu %lu"
				"%*d %*d %*d %ld %ld %*d %llu %lu %ld",
				&pstat.pid, pstat.cmdline, &pstat.state, &pstat.ppid, &pstat.pgrp,
				&pstat.sid, &pstat.tty_nr, &pstat.tpgid, &pstat.utime, &pstat.stime, 
				&pstat.nice, &pstat.num_threads, &pstat.starttime, 
				&pstat.vsize, &pstat.rss);
		fclose(fp);
		
		//uid에서 사용자 이름 구하기
		memset(path, 0, PATHLEN);
		sprintf(path, "%s/%s", "/proc", dirlist[i]->d_name);
		stat(path, &status);
		userpwd = *getpwuid(status.st_uid);
		strcpy(pstat.name, userpwd.pw_name);
		
		//cmdline 구하기
		memset(path, 0, PATHLEN);
		memset(tmp, 0, PATHLEN);
		memset(line, 0, LINEMAX);

		sprintf(path, "%s/%s/%s", "/proc", dirlist[i]->d_name, "cmdline");
		fp = fopen(path, "r");

		fread(line, LINEMAX, 1, fp);
		for (int i = 0; i < NAMELEN; i++) {
			if (line[i] == 0) {
				if (i == LINEMAX - 1)
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
		
//		printf("pid : %d ppid : %d tty_nr : %d state : %c cmdline : %s\n"
//				"name : %s utime : %lu stime : %lu vsize : %lu\n"
//				"rss : %ld starttime : %llu\n", pstat.pid, pstat.ppid,
//				pstat.tty_nr, pstat.state, pstat.cmdline, pstat.name,
//				pstat.utime, pstat.stime, pstat.vsize, pstat.rss, pstat.starttime);
		//리스트에 저장하기

		node = create_node(&pstat);
		add_node(node);
		
	}
	
	for (int i = 0; i < count; i++)
		free(dirlist[i]);
	free(dirlist);

}

void print_ps(void)
{
	if (!a_flag && !u_flag && !x_flag)
		print_default();		//no option
	else if (!x_flag)
		print_without_x();		//option a, (a)u
	else if (!a_flag)
		print_without_a();		//option x, ux
	else
		print_all();			//option ax, aux
}

void print_default(void)
{
	proc_list *tmp = head->next;
	int bash_pid = getppid();
	int bash_tty = -1;
	char bash_name[64];
	char headline[LINEMAX];
	char tmp_line[LINEMAX];
	char line[LINEMAX];
	char tty[8];
	struct tm t;
	time_t runtime;

	memset(headline, 0, LINEMAX);
	memset(line, 0, LINEMAX);
	sprintf(headline, "%6s %-7s %8s %s", "PID", "TTY", "TIME", "CMD");
	strncpy(line, headline, term.ws_col);
	printf("%s\n", line);

	while (tmp != NULL) {
		if (tmp->pid == bash_pid) {
			memset(bash_name, 0, 64);
			strcpy(bash_name, tmp->name);
			bash_tty = tmp->tty_nr;
		}
		
		tmp = tmp->next;
	}

	get_tty(bash_tty, tty);
	
	tmp = head->next;
	while (tmp != NULL) {
		if (tmp->tty_nr == bash_tty && !strcmp(tmp->name, bash_name)) {
			runtime = tmp->stime + tmp->utime;
			runtime /= sysconf(_SC_CLK_TCK);
			t = *gmtime(&runtime);
			
			memset(line, 0, LINEMAX);
			memset(tmp_line, 0, LINEMAX);
			sprintf(tmp_line, "%6d %-7s %02d:%02d:%02d ", tmp->pid, tty,
					t.tm_hour, t.tm_min, t.tm_sec);
			strcat(tmp_line, tmp->cmdline);
			strncpy(line, tmp_line, term.ws_col);
			printf("%s\n", line);
		}

		tmp = tmp->next;
	}
}

void print_without_x(void)
{
	proc_list *tmp = head->next;
	int bash_pid = getppid();
	char bash_name[64];
	char headline[LINEMAX];
	char tmp_line[LINEMAX];
	char line[LINEMAX];
	char tty[8];
	char state[6];
	double cpu_per, mem_per;
	struct tm t, t2;
	time_t runtime;
	time_t starttime;

	while (tmp != NULL) {
		if (tmp->pid == bash_pid) {
			memset(bash_name, 0, 64);
			strcpy(bash_name, tmp->name);
		}
		
		tmp = tmp->next;
	}

	tmp = head->next;

	if (u_flag) {
		memset(line, 0, LINEMAX);
		memset(headline, 0, LINEMAX);
		sprintf(headline, "%-9s %6s %4s %4s %7s %7s %-7s %-5s %5s %5s %s",
				"USER", "PID", "%CPU", "%MEM", "VSZ", "RSS", "TTY", 
				"STAT", "START", "TIME", "COMMAND");
		strncpy(line, headline, term.ws_col);
		printf("%s\n", line);

		while (tmp != NULL) {
			get_tty(tmp->tty_nr, tty);

			if (strcmp(tty, "?") && !strcmp(tmp->name, bash_name)) {
				uptime = get_uptime();
				runtime = tmp->stime + tmp->utime;
				runtime /= sysconf(_SC_CLK_TCK);
				
				t = *gmtime(&runtime);
				cpu_per = runtime / uptime * 100;
				mem_per = (double)tmp->rss * 4 / mem_tot * 100;
				starttime = tmp->starttime / sysconf(_SC_CLK_TCK) + boot_time;
				t2 = *localtime(&starttime);
				get_state(tmp, state);

				memset(line, 0, LINEMAX);
				memset(tmp_line, 0, LINEMAX);
				sprintf(tmp_line, "%-9s %6d %4.1lf %4.1lf %7lu %7ld "
						"%-7s %-5s %02d:%02d %02d:%02d ",
						tmp->name, tmp->pid, cpu_per, mem_per, tmp->vsize / 1024,
						tmp->rss * 4, tty, state, t2.tm_hour, t2.tm_min,
						t.tm_min, t.tm_sec);
				strcat(tmp_line, tmp->cmdline);
				strncpy(line, tmp_line, term.ws_col);
				printf("%s\n", line);
			}
			
			tmp = tmp->next;
		}		
	}
	else {
		memset(line, 0, LINEMAX);
		memset(headline, 0, LINEMAX);
		sprintf(headline, "%6s %-7s %-5s %5s %s", "PID", "TTY", "STAT", "TIME", "CMD");
		strncpy(line, headline, term.ws_col);
		printf("%s\n", line);

		while (tmp != NULL) {
			get_tty(tmp->tty_nr, tty);

			if (strcmp(tty, "?") && !strcmp(tmp->name, bash_name)) {
				runtime = tmp->stime + tmp->utime;
				runtime /= sysconf(_SC_CLK_TCK);
				t = *gmtime(&runtime);
				get_state(tmp, state);

				memset(line, 0, LINEMAX);
				memset(tmp_line, 0, LINEMAX);
				sprintf(tmp_line, "%6d %-7s %-5s %02d:%02d ", tmp->pid, tty,
						state, t.tm_min, t.tm_sec);
				strcat(tmp_line, tmp->cmdline);
				strncpy(line, tmp_line, term.ws_col);
				printf("%s\n", line);
			}

			tmp = tmp->next;
		}
	}
}

void print_without_a(void)
{
	proc_list *tmp = head->next;
	int bash_pid = getppid();
	char bash_name[64];
	char headline[LINEMAX];
	char tmp_line[LINEMAX];
	char line[LINEMAX];
	char tty[8];
	char state[6];
	double cpu_per, mem_per;
	struct tm t, t2;
	time_t runtime, starttime;

	while (tmp != NULL) {
		if (tmp->pid == bash_pid) {
			memset(bash_name, 0, 64);
			strcpy(bash_name, tmp->name);
		}
		
		tmp = tmp->next;
	}
	
	tmp = head->next;

	if (u_flag) {
		memset(line, 0, LINEMAX);
		memset(headline, 0, LINEMAX);
		sprintf(headline, "%-9s %6s %4s %4s %7s %7s %-7s %-5s %5s %5s %s",
				"USER", "PID", "%CPU", "%MEM", "VSZ", "RSS", "TTY", 
				"STAT", "START", "TIME", "COMMAND");
		strncpy(line, headline, term.ws_col);
		printf("%s\n", line);

		while (tmp != NULL) {
			if (!strcmp(tmp->name, bash_name)) {
				get_tty(tmp->tty_nr, tty);
				uptime = get_uptime();
				runtime = tmp->stime + tmp->utime;
				runtime /= sysconf(_SC_CLK_TCK);

				t = *gmtime(&runtime);
				cpu_per = runtime / uptime * 100;
				mem_per = (double)tmp->rss * 4 / mem_tot * 100;
				starttime = tmp->starttime / sysconf(_SC_CLK_TCK) + boot_time;
				t2 = *localtime(&starttime);
				get_state(tmp, state);

				memset(line, 0, LINEMAX);
				memset(tmp_line, 0, LINEMAX);
				sprintf(tmp_line, "%-9s %6d %4.1lf %4.1lf %7lu %7ld "
						"%-7s %-5s %02d:%02d %02d:%02d ",
						tmp->name, tmp->pid, cpu_per, mem_per, tmp->vsize / 1024,
						tmp->rss * 4, tty, state, t2.tm_hour, t2.tm_min,
						t.tm_min, t.tm_sec);
				strcat(tmp_line, tmp->cmdline);
				strncpy(line, tmp_line, term.ws_col);
				printf("%s\n", line);
			}
			
			tmp = tmp->next;
		}
	}
	else {
		memset(line, 0, LINEMAX);
		memset(headline, 0, LINEMAX);
		sprintf(headline, "%6s %-7s %-5s %5s %s", "PID", "TTY", "STAT", "TIME", "CMD");
		strncpy(line, headline, term.ws_col);
		printf("%s\n", line);

		while (tmp != NULL) {
			if (!strcmp(tmp->name, bash_name)) {
				get_tty(tmp->tty_nr, tty);
				runtime = tmp->stime + tmp->utime;
				runtime /= sysconf(_SC_CLK_TCK);
				t = *gmtime(&runtime);
				get_state(tmp, state);

				memset(line, 0, LINEMAX);
				memset(tmp_line, 0, LINEMAX);
				sprintf(tmp_line, "%6d %-7s %-5s %02d:%02d ", tmp->pid, tty,
						state, t.tm_min, t.tm_sec);
				strcat(tmp_line, tmp->cmdline);
				strncpy(line, tmp_line, term.ws_col);
				printf("%s\n", line);
			}

			tmp = tmp->next;
		}
	}


}

void print_all(void)
{
	proc_list *tmp = head->next;
	char headline[LINEMAX];
	char tmp_line[LINEMAX];
	char line[LINEMAX];
	char tty[8];
	char state[6];
	double cpu_per, mem_per;
	struct tm t, t2;
	time_t runtime, starttime;
	
	if (u_flag) {
		memset(line, 0, LINEMAX);
		memset(headline, 0, LINEMAX);
		sprintf(headline, "%-9s %6s %4s %4s %7s %7s %-7s %-5s %5s %5s %s",
				"USER", "PID", "%CPU", "%MEM", "VSZ", "RSS", "TTY",
				"STAT", "START", "TIME", "COMMAND");
		strncpy(line, headline, term.ws_col);
		printf("%s\n", line);

		while (tmp != NULL) {
			get_tty(tmp->tty_nr, tty);
			uptime = get_uptime();
			runtime = tmp->stime + tmp->utime;
			runtime /= sysconf(_SC_CLK_TCK);

			t = *gmtime(&runtime);
			cpu_per = runtime / uptime * 100;
			mem_per = (double)tmp->rss * 4 / mem_tot * 100;
			starttime = tmp->starttime / sysconf(_SC_CLK_TCK) + boot_time;
			t2 = *localtime(&starttime);
			get_state(tmp, state);

			memset(line, 0, LINEMAX);
			memset(tmp_line, 0, LINEMAX);
			sprintf(tmp_line, "%-9s %6d %4.1lf %4.1lf %7lu %7ld "
					"%-7s %-5s %02d:%02d %02d:%02d ",
					tmp->name, tmp->pid, cpu_per, mem_per, tmp->vsize / 1024,
					tmp->rss * 4, tty, state, t2.tm_hour, t2.tm_min,
					t.tm_min, t.tm_sec);
			strcat(tmp_line, tmp->cmdline);
			strncpy(line, tmp_line, term.ws_col);
			printf("%s\n", line);

			tmp = tmp->next;
		}
	}
	else {
		memset(line, 0, LINEMAX);
		memset(headline, 0, LINEMAX);
		sprintf(headline, "%6s %-7s %-5s %5s %s", "PID", "TTY", "STAT", "TIME", "CMD");
		strncpy(line, headline, term.ws_col);
		printf("%s\n", line);

		while (tmp != NULL) {
			get_tty(tmp->tty_nr, tty);

			runtime = tmp->stime + tmp->utime;
			runtime /= sysconf(_SC_CLK_TCK);
			t = *gmtime(&runtime);
			get_state(tmp, state);

			memset(line, 0, LINEMAX);
			memset(tmp_line, 0, LINEMAX);
			sprintf(tmp_line, "%6d %-7s %-5s %02d:%02d ", tmp->pid, tty,
					state, t.tm_min, t.tm_sec);
			strcat(tmp_line, tmp->cmdline);
			strncpy(line, tmp_line, term.ws_col);
			printf("%s\n", line);

			tmp = tmp->next;
		}
	}
}


proc_list *create_node(proc_list* stat)
{
	proc_list *new;

	new = (proc_list *)malloc(sizeof(proc_list));

	if (stat == NULL)
		memset(new, 0, sizeof(proc_list));
	else
		memcpy(new, stat, sizeof(proc_list));
	
	new->next = NULL;

	return new;
}

void add_node(proc_list *node)
{
	proc_list *tmp = head;

	while (tmp->next != NULL) 
		tmp = tmp->next;

	tmp->next = node;
}

void print_list(void)		//리스트 확인
{
	proc_list *pstat = head->next;

	while (pstat != NULL) {
		printf("pid : %d ppid : %d tty_nr : %d state : %c cmdline : %s "
				"name : %s utime : %lu stime : %lu vsize : %lu "
				"rss : %ld starttime : %llu\n", pstat->pid, pstat->ppid,
				pstat->tty_nr, pstat->state, pstat->cmdline, pstat->name,
				pstat->utime, pstat->stime, pstat->vsize, pstat->rss, pstat->starttime);
		pstat = pstat->next;
	}
}

void free_list(proc_list *tmp)
{
	if (tmp == NULL) return;
	else free_list(tmp->next);

	free(tmp);
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

long get_total_memory(void)
{
	FILE *fp = fopen("/proc/meminfo", "r");
	long res;

	fscanf(fp, "%*s %ld", &res);
	fclose(fp);
	return res;
}

double get_uptime(void)
{
	FILE *fp = fopen("/proc/uptime", "r");
	double res;

	fscanf(fp, "%lf", &res);
	fclose(fp);
	return res;
}

void get_tty(int tty_nr, char *tty)
{
	int major, minor;

	major = MAJOR(tty_nr);
	minor = MINOR(tty_nr);

	memset(tty, 0, 8);

	if (major == 4) {
		if (minor < 64)
			sprintf(tty, "%s%d", "tty", minor);
		else
			sprintf(tty, "%sS%d", "tty", minor - 64);
	}
	else if (major >= 136 && major <= 143) {
		sprintf(tty, "%s/%d", "pts", minor);
	}
	else
		sprintf(tty, "%s", "?");
}

void get_state(proc_list *tmp, char *state)	// L state(check locked page)의 경우 smaps에서 확인할 수 있다는
{						// 것 까지는 확인하였으나 확인하려면 CONFIG_PROC_PAGE_MONITOR를
	int idx = 0;				// enable 시켜야 된다는데, 방법을 모르겠어서 구현을 못했습니다..
	memset(state, 0, 6);
	state[idx++] = tmp->state;
	
	if (tmp->nice < -4)		//priority low, high 기준을 정확히 못찾겠어서 임의로 정했습니다.
		state[idx++] = '<';	//5 이상 low, -5 이하 high로 나타나는걸로 보였습니다 (제 pc 기준)
	else if (tmp->nice > 4)
		state[idx++] = 'N';

	if (tmp->pid == tmp->sid)
		state[idx++] = 's';

	if (tmp->num_threads > 1)
		state[idx++] = 'l';

	if (tmp->pgrp == tmp->tpgid)
		state[idx++] = '+';
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
