#include <stdio.h>
#include <unistd.h>
#include <linux/kdev_t.h>
int main(void) {
	int dev = 1026;
	printf("%d %d\n", MAJOR(dev), MINOR(dev));
	printf("%d\n", getppid());
	int pid = fork();
	if (pid == 0) while(1);
}
