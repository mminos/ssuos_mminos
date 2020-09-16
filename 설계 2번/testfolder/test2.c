#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
	int a,b,c,d;
	scanf("%d %*d"
			"%d %d", &a, &b, &c);
	printf("%d %d %d\n", a, b, c);
}
