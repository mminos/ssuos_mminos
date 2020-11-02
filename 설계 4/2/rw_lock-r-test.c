#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //	Write the code for initializing your read-write lock.
	pthread_rwlock_init(&rw->lock, NULL);
	rw->w_request = 0;
	rw->r_num = 0;
}

void r_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
	while (1) {
		if (rw->w_request != 0 && rw->r_num == 0) {
			continue;
		}
		else {
			pthread_rwlock_rdlock(&rw->lock);
			rw->r_num++;
			break;
		}
	}
}

void r_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
	pthread_rwlock_unlock(&rw->lock);
	rw->r_num--;
}

void w_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
	rw->w_request++;
	pthread_rwlock_wrlock(&rw->lock);
	rw->w_request--;
}

void w_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
	pthread_rwlock_unlock(&rw->lock);
}
