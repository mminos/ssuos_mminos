#include <pthread.h>

typedef struct wait_q {
	int *status;
	struct wait_q *next;
} wait_q;

typedef struct SSU_Sem {
	int key;
	wait_q *head;
	pthread_mutex_t lock;
	int wait_lock;
} SSU_Sem;

void SSU_Sem_init(SSU_Sem *, int);
void SSU_Sem_up(SSU_Sem *);
void SSU_Sem_down(SSU_Sem *);
