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
}

void SSU_Sem_down(SSU_Sem *s) {
	int status = 1;
	wait_q *thread;
	wait_q *tmp = s->head;

	if (s->key <= 0) {
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

		while (1) {
			if (s->key > 0) {
				if (s->head == NULL || s->head->status == &status) {

					s->key--;
					if (s->head != NULL) {
						tmp = s->head;
						s->head = s->head->next;
						free(tmp);
					}

					break;
				}
				usleep(1000);
			}
		}
	}
	else {
		s->key--;
	}
}

void SSU_Sem_up(SSU_Sem *s) {
	s->key++;
}
