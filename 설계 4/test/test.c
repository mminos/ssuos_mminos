#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

void *thread (void *arg) {
	pthread_rwlock_rdlock(&lock);
	printf("rdlock start\n");
	sleep(5);
	printf("rdlock end\n");
	pthread_rwlock_unlock(&lock);
	return NULL;
}

void *thread_w(void *arg) {
	pthread_rwlock_wrlock(&lock);
	printf("wrlock start\n");
	sleep(4);
	printf("wrlock end\n");
	pthread_rwlock_unlock(&lock);
	return NULL;
}

int main(void) {
	pthread_t tid1, tid2;
	pthread_t tid_w;

	pthread_create(&tid1, NULL, thread, NULL);
	pthread_create(&tid_w, NULL, thread_w, NULL);
	pthread_create(&tid2, NULL, thread, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_join(tid_w, NULL);
	printf("complete\n");
	pthread_rwlock_destroy(&lock);
	exit(0);
}
