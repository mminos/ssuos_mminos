#ifndef _SSU_PS_
#define _SSU_PS_

// header

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/kdev_t.h>

//defines

#define true 1
#define false 0
#define PATHLEN 1024
#define NAMELEN 512

typedef struct proc_info {
	int pid;			// /proc/pid/stat
	int ppid;			// /porc/pid/stat
	int tty_nr;			// /proc/pid/stat
	char state;			// /proc/pid/stat
	char cmdline[NAMELEN];		// /porc/pid/cmdline
	char name[64];			// getpwuid..
	unsigned long utime, stime;	// /proc/pid/stat
	unsigned long vsize;		// /proc/pid/stat
	long rss;			// /proc/pid/stat
	unsigned long long starttime;	// /proc/pid/stat

	struct proc_info *next;
} proc_list;

//functions
int check_option(char **argv);
void scan_proc(void);
void get_cmdline(char *d_name, char *cmdline);

proc_list *create_node(proc_list stat);

time_t get_boot_time(void);
long get_total_memory(void);
int scandir_filter(const struct dirent *file);
int _isdigit(char *str);




#endif
