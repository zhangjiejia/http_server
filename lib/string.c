#include <memory.h>
#include "string.h"

int get_char_index_of(char ch, const char* data) {
	if(data == NULL)
		return -1;
	
	const char* ptr = data;
	
	for(; (*(unsigned long long*)ptr & (sizeof(unsigned long long) - 1)) != 0; ++ptr) {
		if(*ptr == ch) return ptr - data;
		
		if(*ptr == 0) return -1;
	}
	
	unsigned long long change = 0;
	
	for(unsigned long i = 0; i < sizeof(unsigned long long); ++i) 
		change += (unsigned long long)ch << (i * 8);
	
	unsigned long long number_1 = *(unsigned long long*)ptr;
	unsigned long long number_2 = number_1 ^ change;
	unsigned long long magic = 0x7efefefefefefeffL;
	
	while(1) {
		if(ptr - data > STRING_MAX_LEN)
			break;
		
		if((((number_2 + magic) ^ ~number_2) & ~magic) != 0) {
			if(ptr[0] == ch) return ptr - data + 0;
			
			if(ptr[1] == ch) return ptr - data + 1;
			
			if(ptr[2] == ch) return ptr - data + 2;
			
			if(ptr[3] == ch) return ptr - data + 3;
			
			if(ptr[4] == ch) return ptr - data + 4;
			
			if(ptr[5] == ch) return ptr - data + 5;
			
			if(ptr[6] == ch) return ptr - data + 6;
			
			if(ptr[7] == ch) return ptr - data + 7;
		}
		
		if((((number_1 + magic) ^ ~number_1) & ~magic) != 0)
			break;
		
		ptr += sizeof(unsigned long long);
		
		number_1 = *(unsigned long long*)ptr;
		number_2 = number_1 ^ change;
	}
	
	return -1;
}

int get_str_index_of(char* str, const char* data) {
	if(str == NULL || data == NULL)
		return -1;
	
	int str_len = get_char_index_of('\0', str);
	int data_len = get_char_index_of('\0', data);
	
	if(str_len == -1 || data_len == -1 || str_len > data_len)
		return -1;
	
	const char* data_ptr = data;
	int offset = -1;
	
	while(1) {
		if(data_ptr - data >= data_len)
			break;
		
		if(memcmp(str, data_ptr, str_len) == 0)
			return data_ptr - data;
		
		data_ptr += str_len;
		
		offset = get_char_index_of(str[str_len - 1], data_ptr);
		
		if(offset == -1)
			break;
		
		data_ptr += offset - (str_len - 1);
	}
	
	return -1;
}




