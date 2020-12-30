#include "ssufs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	/* 1 */
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	int inode_number;

	if (open_namei(filename) >= 0) 
		return -1;
	
	if ((inode_number = ssufs_allocInode()) < 0)
		return -1;

	inode->status = INODE_IN_USE;
	memcpy(inode->name, filename, MAX_NAME_STRLEN);
	inode->file_size = 0;
	for (int i = 0; i < MAX_FILE_SIZE; i++)
		inode->direct_blocks[i] = -1;

	ssufs_writeInode(inode_number, inode);
	
	free(inode);
	return inode_number;
}

void ssufs_delete(char *filename){
	/* 2 */
	int inode_number = open_namei(filename);
	
	if (inode_number < 0)
		return;
	
	for (int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == inode_number) {
			ssufs_close(i);
			break;
		}
	}

	ssufs_freeInode(inode_number);	
}

int ssufs_open(char *filename){
	/* 3 */
	int inode_number = open_namei(filename);
	int file_handle = ssufs_allocFileHandle();

	if (inode_number < 0 || file_handle < 0)
		return -1;
	
	file_handle_array[file_handle].inode_number = inode_number;
	return file_handle;
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	/* 4 */
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	char *tmp_buf = (char *)malloc(BLOCKSIZE * MAX_FILE_SIZE);
	int inode_number = file_handle_array[file_handle].inode_number;
	int offset = file_handle_array[file_handle].offset;

	if (inode_number < 0)
		return -1;
	
	memset(tmp_buf, 0, BLOCKSIZE * MAX_FILE_SIZE);
	ssufs_readInode(inode_number, inode);

	if (inode->file_size < nbytes + offset)
		return -1;

	for (int i = 0; i < MAX_FILE_SIZE; i++) {
		int idx = inode->direct_blocks[i];
		if (idx < 0) 
			break;

		ssufs_readDataBlock(idx, tmp_buf + BLOCKSIZE * i);
	}
	memcpy(buf, tmp_buf + offset, nbytes);
	ssufs_lseek(file_handle, nbytes);

	free(tmp_buf);
	free(inode);
	return 0;
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	/* 5 */
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	char *tmp_buf = (char *)malloc(BLOCKSIZE * MAX_FILE_SIZE);
	int inode_number = file_handle_array[file_handle].inode_number;
	int offset = file_handle_array[file_handle].offset;
	int fsize = 0;

	if (inode_number < 0 || offset + nbytes > BLOCKSIZE * MAX_FILE_SIZE)
		return -1;

	memset(tmp_buf, 0, BLOCKSIZE * MAX_FILE_SIZE);
	ssufs_readInode(inode_number, inode);

	int num_blocks = (offset + nbytes - 1) / BLOCKSIZE + 1;

	for (int i = 0; i < MAX_FILE_SIZE; i++) {
		int idx = inode->direct_blocks[i];
		if (idx < 0) {
			if (i < num_blocks) {
				if ((idx = ssufs_allocDataBlock()) < 0)
					return -1;
				inode->direct_blocks[i] = idx;
				ssufs_writeDataBlock(idx, tmp_buf + BLOCKSIZE * i);
			}
			else
				break;
		}
		ssufs_readDataBlock(idx, tmp_buf + BLOCKSIZE * i);
	}
	memcpy(tmp_buf + offset, buf, nbytes);

	for (int i = 0; i < MAX_FILE_SIZE; i++) {
		int idx = inode->direct_blocks[i];
		if (idx < 0) 
			break;

		ssufs_writeDataBlock(idx, tmp_buf + BLOCKSIZE * i);
	}

	if (inode->file_size < offset + nbytes)
		inode->file_size = offset + nbytes;
	
	ssufs_writeInode(inode_number, inode);
	ssufs_lseek(file_handle, nbytes);

	free(tmp_buf);
	free(inode);
	return 0;
}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);
	
	int fsize = tmp->file_size;
	
	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {
		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}
