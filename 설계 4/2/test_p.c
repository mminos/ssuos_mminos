#include "rw_lock.h"

struct rw_lock rwlock;
pthread_spinlock_t spinlock;

void *Read(void *arg) {
	r_lock(&rwlock);
	printf("hi im reader\n");
	pthread_spin_lock(&spinlock);
	sleep(4);
	printf("read finish...\n");
	pthread_spin_unlock(&spinlock);
	r_unlock(&rwlock);
	return NULL;
}

void *Write(void *arg) {
	w_lock(&rwlock);
	printf("hi im writer\n");
	pthread_spin_lock(&spinlock);
	sleep(4);
	printf("write finish...\n");
	pthread_spin_unlock(&spinlock);
	w_unlock(&rwlock);
	return NULL;
}

int main(void) {
	pthread_t r_tid[4];
	pthread_t w_tid[2];
	printf("start!\n");	
	init_rwlock(&rwlock);
	pthread_spin_init(&spinlock, 0);
	pthread_create(&r_tid[0], NULL, Read, NULL);
	pthread_create(&r_tid[1], NULL, Read, NULL);
	pthread_create(&w_tid[0], NULL, Write, NULL);
	pthread_create(&w_tid[1], NULL, Write, NULL);

	pthread_create(&r_tid[2], NULL, Read, NULL);
	pthread_create(&r_tid[3], NULL, Read, NULL);

	for (int i = 0; i < 4; i++)
		pthread_join(r_tid[i], NULL);
	for (int i = 0; i < 2; i++)
		pthread_join(w_tid[i], NULL);

	printf("\ncomplete...\n");
	exit(0);
}
