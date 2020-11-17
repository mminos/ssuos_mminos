#include "alloc.h"

#define CHUNKMAX (PAGESIZE / MINALLOC)
typedef struct mm {
	char *addr;
	int chunk_size;
	int flag;
	int using;
	int next_idx;
	int prev_idx;
}mm;

//---------------------------additional function
int find_next(void);
void merge_chunk(void);
//---------------------------------------------

int head_idx = -1;
mm chunks[CHUNKMAX];

int init_alloc(void)
{
	char *addr;
	
	addr = mmap(0, PAGESIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

	if (addr < 0) 
		return 1;

	head_idx = 0;
	
	chunks[0].addr = addr;
	chunks[0].chunk_size = PAGESIZE;
	chunks[0].flag = 0;
	chunks[0].next_idx = -1;
	chunks[0].prev_idx = -1;
	chunks[0].using = 1;

	return 0;
}

int cleanup(void)
{
	int res = munmap(chunks[head_idx].addr, PAGESIZE);
	memset(chunks, 0, sizeof(chunks));
	head_idx = -1;

	if (res < 0) return 1;
	else return 0;
}

char *alloc(int size)
{
	int tmp_idx;
	int new_idx;
	mm tmp, new;

	if (size % MINALLOC != 0 || size > PAGESIZE)
		return NULL;
	
	tmp_idx = head_idx;

	while (tmp_idx != -1) {
		tmp = chunks[tmp_idx];

		if (tmp.next_idx == -1 && tmp.chunk_size < size)
			return NULL;

		if (tmp.chunk_size >= size && tmp.flag == 0) {
			if (tmp.chunk_size == size) {
				tmp.flag = 1;
				new = tmp;

				chunks[tmp_idx] = tmp;
				break;
			}

			new_idx = find_next();

			new.addr = tmp.addr;
			new.chunk_size = size;
			new.flag = 1;
			new.using = 1;
			new.next_idx = tmp_idx;
			new.prev_idx = -1;

			if (tmp.prev_idx != -1) {
				new.prev_idx = tmp.prev_idx;
				chunks[new.prev_idx].next_idx = new_idx;
			}
			else
				head_idx = new_idx;

			tmp.prev_idx = new_idx;
			tmp.addr += new.chunk_size;
			tmp.chunk_size -= new.chunk_size;

			chunks[tmp_idx] = tmp;
			chunks[new_idx] = new;
			break;
		}
		tmp_idx = tmp.next_idx;
	}

	return new.addr;
}

void dealloc(char *addr)
{
	int tmp_idx;
	mm tmp;

	tmp_idx = head_idx;

	while (tmp_idx != -1) {
		tmp = chunks[tmp_idx];

		if (tmp.addr == addr) {
			tmp.flag = 0;
			chunks[tmp_idx] = tmp;
			break;
		}
		tmp_idx = tmp.next_idx;
	}

	merge_chunk();
}

int find_next(void)
{
	int next_idx = -1;

	for (int i = 0; i < CHUNKMAX; i++) {
		if (chunks[i].using == 0) {
			next_idx = i;
			break;
		}
	}
	return next_idx;
}

void merge_chunk(void)
{
	int tmp_idx = head_idx;
	int del_idx;
	mm tmp, del;

	while (tmp_idx != -1) {
		tmp = chunks[tmp_idx];

		if (tmp.flag == 0 && tmp.next_idx != -1 && chunks[tmp.next_idx].flag == 0) {
			del_idx = tmp.next_idx;
			del = chunks[del_idx];

			tmp.chunk_size += del.chunk_size;
			tmp.next_idx = del.next_idx;

			if (del.next_idx != -1)
				chunks[del.next_idx].prev_idx = tmp_idx;

			del.using = 0;

			chunks[tmp_idx] = tmp;
			chunks[del_idx] = del;
			continue;
		}
		tmp_idx = tmp.next_idx;
	}
}

