#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //	Write the code for initializing your read-write lock.
	pthread_rwlock_init(&rw->lock, NULL);
        pthread_spin_init(&rw->busy, 0);
	rw->w_request = 0;
}

void r_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
	while (1) {
		if (rw->w_request == 0) {
			pthread_rwlock_rdlock(&rw->lock);
			break;
		}
	}
}

void r_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
	pthread_rwlock_unlock(&rw->lock);
}

void w_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
        pthread_spin_lock(&rw->busy);
	rw->w_request++;
        pthread_spin_unlock(&rw->busy);
	
	pthread_rwlock_wrlock(&rw->lock);

        pthread_spin_lock(&rw->busy);
	rw->w_request--;
        pthread_spin_unlock(&rw->busy);
}

void w_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
	pthread_rwlock_unlock(&rw->lock);
}
