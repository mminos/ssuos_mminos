#include <stdio.h>

struct str{
	int a,b,c;
};

int main(void) {
	struct str st = {1,2,3};
	printf("%d %d %d\n", st.a,st.b,st.c);
	return 0;
}
