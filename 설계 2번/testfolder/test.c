#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
	int fd1[2], fd2[2];
	char buf[16];

	pipe(fd1);

	printf("%d %d\n", fd1[0], fd1[1]);

	pipe(fd2);

	printf("%d %d\n", fd2[0], fd2[1]);

	
	write(fd1[1], "oslab", 6);
	read(fd2[0], buf, 6);

	printf("%s\n", buf);
	
	return 0;
}
