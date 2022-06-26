#include <iostream>
#include "malloc_3.h"

void get_stats (size_t* num_free_blocks, size_t* num_free_bytes, size_t* num_allocated_blocks, size_t* num_allocated_bytes){
    *num_free_blocks = _num_free_blocks();
    *num_free_bytes = _num_free_bytes();
    *num_allocated_blocks = _num_allocated_blocks();
    *num_allocated_bytes = _num_allocated_bytes() - _num_meta_data_bytes();
}

int main() {
    size_t num_free_blocks;
    size_t num_free_bytes;
    size_t num_allocated_blocks;
    size_t num_allocated_bytes;
    void* one = smalloc(1);
    void* ten = smalloc(10);
    void* two = smalloc(2);
    get_stats( &num_free_blocks, &num_free_bytes, &num_allocated_blocks, &num_allocated_bytes);
    sfree (ten);
    get_stats( &num_free_blocks, &num_free_bytes, &num_allocated_blocks, &num_allocated_bytes);
    sfree (one);
    get_stats( &num_free_blocks, &num_free_bytes, &num_allocated_blocks, &num_allocated_bytes);
    return 0;
}
