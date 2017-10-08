#ifndef		__MPOOL_H__
#define		__MPOOL_H__

#define 	MPOOL_BLOCK_SIZE	64
#define		MPOOL_MIN_SIZE		4194304

typedef struct {
	unsigned int 	total;
	unsigned int 	current;
	unsigned int 	last;
	unsigned int 	ignore;
	void*			data;
} mpool;

mpool* create_mpool(unsigned int total_size);

void* get_block_from_mpool(mpool* mp, unsigned int size);

void free_mpool_block(mpool* mp, void* block);

#endif