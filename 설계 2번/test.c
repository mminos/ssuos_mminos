#include <stdio.h>
#include <time.h>

int main(void) 
{
	time_t t = 1600334909;
	struct tm _t;

	printf("time : %s\n", ctime(&t));
}
