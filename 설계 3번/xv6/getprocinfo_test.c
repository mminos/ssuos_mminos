#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

struct processInfo
{
	int pid;
	int ppid;
    	uint psize;
    	int nswitch;		//number of context switches
};


int main(void)
{
	int max, i;
	struct processInfo procinfo;
	set_prio(1000);
	printf(1, "PID \t PPID \t SIZE \t Number of Context Switch\n");

	max = get_max_pid();
	for (i = 1; i <= max; i++) {
		if (get_proc_info(i, &procinfo) < 0)
			continue;
		else {
			printf(1, "%d \t %d \t %d \t %d\n",
					procinfo.pid, procinfo.ppid, procinfo.psize, procinfo.nswitch);
		}
	}
	exit();
}

