#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
        pthread_rwlock_init(&rw->lock, NULL);
        rw->r_flag = 0;
  //	Write the code for initializing your read-write lock.
}

void r_lock(struct rw_lock * rw)
{
        if (rw->r_flag == 0) 
                rw->r_flag = 1;

        pthread_rwlock_rdlock(&rw->lock);
}

void r_unlock(struct rw_lock * rw)
{
	pthread_rwlock_unlock(&rw->lock);
}

void w_lock(struct rw_lock * rw)
{
        while (rw->r_flag == 0);
        pthread_rwlock_wrlock(&rw->lock);
}

void w_unlock(struct rw_lock * rw)
{
	pthread_rwlock_unlock(&rw->lock);
}
