#include "alloc.h"


char *make_ptr();

int main(void) {
	char *ptr;

	ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0 );
	ptr[2048] = '9';
	printf("%d %d\n", ptr[1], ptr[2048]);
	printf("%p\n%p\n", ptr, make_ptr());
	return 0;
}

char *make_ptr(void) {
	char *ptr2 = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0 );
	return ptr2;
}
