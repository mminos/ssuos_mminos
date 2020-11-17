#include "ealloc.h"

#define PAGEMAX 4
#define CHUNKMAX (PAGESIZE / MINALLOC)
typedef struct mm {
	char *addr;
	int chunk_size;
	int flag;
	int using;
	int next_idx;
	int prev_idx;
} mm;

//--------------------------------additional function
int find_next(int);
void merge_chunk(int);
//---------------------------------------------------

int pagenum;

int head_idx[PAGEMAX];
mm chunks[PAGEMAX][CHUNKMAX];

void init_alloc(void) 
{
	for (int i = 0; i < PAGEMAX; i++)
		head_idx[i] = -1;

	return;
}

void cleanup(void)
{
	return;
}

char *alloc(int size)
{
	int check = 0;
	int new_idx;
	char *new_page;
	mm tmp;
	mm new;
	
	if (size % MINALLOC != 0 || size > PAGESIZE)
		return NULL;

	for (int i = 0; i < pagenum; i++) {
		int tmp_idx = head_idx[i];

		while (tmp_idx != -1) {
			tmp = chunks[i][tmp_idx];

			if (tmp.next_idx == -1 && tmp.chunk_size < size) 
				break;

			if (tmp.chunk_size >= size && tmp.flag == 0) {
				if (tmp.chunk_size == size) {
					tmp.flag = 1;
					new = tmp;
					
					chunks[i][tmp_idx] = tmp;
					check = 1;
					break;
				}
				
				new_idx = find_next(i);
						
				new.addr = tmp.addr;
				new.chunk_size = size;
				new.flag = 1;
				new.using = 1;
				new.next_idx = tmp_idx;
				new.prev_idx = -1;

				if (tmp.prev_idx != -1) {
					new.prev_idx = tmp.prev_idx;
					chunks[i][new.prev_idx].next_idx = new_idx;
				}
				else	//tmp == head
					head_idx[i] = new_idx;
				
				tmp.prev_idx = new_idx;
				tmp.addr += new.chunk_size;
				tmp.chunk_size -= new.chunk_size;
				
				chunks[i][tmp_idx] = tmp;
				chunks[i][new_idx] = new;
				check = 1;
				break;
			}

			tmp_idx = tmp.next_idx;
		}

		if (check == 1)
			return new.addr;

	}
	
	if (check == 0) {
		if (pagenum == PAGEMAX)
			return NULL;
		
		new_page = mmap(0, PAGESIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

		head_idx[pagenum] = 0;

		chunks[pagenum][0].addr = new_page;
		chunks[pagenum][0].chunk_size = PAGESIZE;
		chunks[pagenum][0].flag = 0;
		chunks[pagenum][0].next_idx = -1;
		chunks[pagenum][0].prev_idx = -1;
		chunks[pagenum][0].using = 1;
		pagenum++;

		return alloc(size);
	}
}

void dealloc(char *addr)
{
	int check = -1;
	mm tmp;

	for (int i = 0; i < pagenum; i++) {
		int tmp_idx = head_idx[i];
		
		while (tmp_idx != -1) {
			tmp = chunks[i][tmp_idx];

			if (tmp.addr == addr) {
				tmp.flag = 0;
				check = i;
				chunks[i][tmp_idx] = tmp;
				break;
			}
			tmp_idx = tmp.next_idx;
		}

		if (check != -1) {
			merge_chunk(check);
			break;
		}
	}
}

void merge_chunk(int idx)
{
	int tmp_idx = head_idx[idx];
	int del_idx;
	mm tmp, del;

	while (tmp_idx != -1) {
		tmp = chunks[idx][tmp_idx];

		if (tmp.flag == 0 && tmp.next_idx != -1 && chunks[idx][tmp.next_idx].flag == 0) {
			del_idx = tmp.next_idx;
			del = chunks[idx][del_idx];

			tmp.chunk_size += del.chunk_size;
			tmp.next_idx = del.next_idx;

			if (del.next_idx != -1)
				chunks[idx][del.next_idx].prev_idx = tmp_idx;

			del.using = 0;
			
			chunks[idx][tmp_idx] = tmp;
			chunks[idx][del_idx] = del;
			continue;
		}
		tmp_idx = tmp.next_idx;
	}
}


int find_next(int pgnum)
{
	int next_idx = -1;
	
	for (int i = 0; i < CHUNKMAX; i++) {
		if (chunks[pgnum][i].using == 0) {
			next_idx = i;
			break;
		}
	}
	return next_idx;
}
