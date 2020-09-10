#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
	int status;
	int res = 9099;

	if (fork() == 0) {
		res = execlp("cd", "cd", "~", (char *)0);
		printf("res = %d\n", res);
		exit(5);
	}
	wait(&status);

	printf("res : %d\n", res);
	return 0;

}
