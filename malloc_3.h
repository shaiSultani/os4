//
// Created by student on 6/22/22.
//

#ifndef OS4_MALLOC_3_H
#define OS4_MALLOC_3_H

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

class SortedList{
public:
    MallocMetadata* head;
    MallocMetadata* wilderness;
    SortedList() : head(nullptr), wilderness(nullptr) {};
    void insert(MallocMetadata* metadata);
    void remove(MallocMetadata* node);

    void merge(MallocMetadata *low, MallocMetadata *mid, MallocMetadata *high);

    void split(MallocMetadata *curr, size_t size);

    void getNeighbors(MallocMetadata *curr, MallocMetadata **low, MallocMetadata **high);
};

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

#endif //OS4_MALLOC_3_H