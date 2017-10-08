#include <stdlib.h>
#include <memory.h>
#include "mpool.h"

mpool* create_mpool(unsigned int total_size) {
	if(total_size < MPOOL_MIN_SIZE || total_size % MPOOL_BLOCK_SIZE != 0)
		return NULL;
	
	mpool* mp = (mpool*)malloc(sizeof(mpool) + total_size);
	mp->total = total_size / MPOOL_BLOCK_SIZE;
	mp->current = 0;
	mp->last = mp->total;
	mp->data = (void*)mp + sizeof(mpool);
	
	memset(mp->data, 0, total_size);
	
	return mp;
}

void* get_block_from_mpool(mpool* mp, unsigned int size) {
	if(mp == NULL || size == 0)
		return NULL;
	
	size += sizeof(unsigned int);
	
	unsigned int need = size / MPOOL_BLOCK_SIZE + 
		(size % MPOOL_BLOCK_SIZE == 0 ? 0 : 1);
	
	if(mp->last < need)
		return NULL;
	
	void* data = mp->data + mp->current * MPOOL_BLOCK_SIZE;
	unsigned int block_info = 0;
	unsigned int used = 0;
	unsigned int len = 0;
	unsigned int count = 0;
	unsigned int times = 1;
	
	while(1) {
		if(mp->current >= mp->total) {
			++times;
			mp->current = 0;
			data = mp->data + mp->current * MPOOL_BLOCK_SIZE;
		}
		
		if(times > 2)
			return NULL;
		
		block_info = *(unsigned int*)(mp->data + mp->current * MPOOL_BLOCK_SIZE);
		used = block_info >> 31;
		len = block_info & 0x7fffffff;
		
		if(used == 1) {
			count = 0;
			mp->current += len * MPOOL_BLOCK_SIZE;
			data = mp->data + mp->current * MPOOL_BLOCK_SIZE;
			continue;
		}
		
		if(len >= (need - count)) {
			mp->current += need - count;
			mp->last -= need;
			
			block_info = (1 << 31) ^ need;
			memcpy(data, &block_info, sizeof(unsigned int));
			
			if(len == (need - count))
				return data + sizeof(unsigned int);
			
			block_info = len - (need - count);
			memcpy(mp->data + mp->current * MPOOL_BLOCK_SIZE, &block_info, sizeof(unsigned int));
			
			return data + sizeof(unsigned int);
		}
		
		++count;
		++mp->current;
		
		if(count == need) {
			mp->last -= need;
			block_info = (1 << 31) ^ need;
			memcpy(data, &block_info, sizeof(unsigned int));
			return data + sizeof(unsigned int);
		}
	}
	
	return NULL;
}

void free_mpool_block(mpool* mp, void* block) {
	if(mp == NULL || block == NULL)
		return;
	
	block = block - sizeof(unsigned int);
	
	unsigned int block_info = *(unsigned int*)block;
	unsigned int used = block_info >> 31;
	unsigned int len = block_info & 0x7fffffff;
	
	if(used == 0)
		return;
	
	memcpy(block, &len, sizeof(unsigned int));
	
	mp->last += len;
}
