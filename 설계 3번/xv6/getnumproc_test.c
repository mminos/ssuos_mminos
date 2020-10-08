#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{
	int num = get_num_proc();
	printf(1, "Total Number of Active Processes : %d\n", num);
	exit();
}

