//
// Created by student on 6/20/22.
//

#ifndef OS4_MALLOC_2_H
#define OS4_MALLOC_2_H

#include <unistd.h>
#include <cmath>
#include <cstring>


typedef struct malloc_meta_data_t{
    size_t size;
    bool is_free;
    struct malloc_meta_data_t* next;
    struct malloc_meta_data_t* prev;
} MallocMetadata;

typedef struct stats_t{
    size_t num_free_blocks = 0;
    size_t num_free_bytes = 0;
    size_t num_allocated_blocks = 0;
    size_t num_allocated_bytes = 0;
} MallocStats;

void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

#endif //OS4_MALLOC_2_H
