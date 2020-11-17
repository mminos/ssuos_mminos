#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "SSU_Sem.h"

void SSU_Sem_init(SSU_Sem *s, int value) {
	s->key = value;
	s->head = NULL;
	s->wait_lock = 0;
       	pthread_mutex_init(&s->lock, 0);
}


void SSU_Sem_down(SSU_Sem *s) {
	int status = 1;
	wait_q *thread;
	wait_q *tmp;

	pthread_mutex_lock(&s->lock);
	if (s->key <= 0) {
		tmp = s->head;
		thread = (wait_q *)malloc(sizeof(wait_q));
		thread->status = &status;
		thread->next = NULL;
		
		if (tmp == NULL)
			s->head = thread;
		else {
			while (tmp->next != NULL)
				tmp = tmp->next;
			tmp->next = thread;
		}
		pthread_mutex_unlock(&s->lock);
		
		while (1) {
			if (s->key > 0) {
				if (s->wait_lock == 0 && s->head->status == &status) {
                                        s->wait_lock = 1;

					s->key--;
                                        tmp = s->head;
					s->head = s->head->next;
					free(tmp);

                                        s->wait_lock = 0;
					break;
				}
			}
		}
	}
	else {
		s->key--;
		pthread_mutex_unlock(&s->lock);
	}
}

void SSU_Sem_up(SSU_Sem *s) {
	s->key++;
}
