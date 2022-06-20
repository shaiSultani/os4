//
// Created by student on 6/20/22.
//

#include "malloc_2.h"

MallocMetadata* head = nullptr;
MallocStats stats;

void* smalloc(size_t size){
    if (size == 0 || size > 100000000){
        return nullptr;
    }
    MallocMetadata* iterator = head;
    while (iterator){
        if (iterator->is_free && iterator->size >= size){
            iterator->is_free = false;
            return (void*)(iterator + sizeof(MallocMetadata));
        }
        if (!iterator->next){
            break;
        }
        iterator = iterator->next;
    }
    void* program_break = sbrk(size + sizeof(MallocMetadata));
    if (program_break == (void*)(-1)){
        return nullptr;
    }
    MallocMetadata* curr = (MallocMetadata*)program_break; ///sbrk points to old program break
    curr->size = size;
    curr->is_free = false;
    if (!head){
        head = curr;
    }
    else{
        curr->prev = iterator;
        iterator->next = curr;
    }
    stats.num_allocated_blocks++;
    stats.num_allocated_bytes += size;
    return (void*)(curr + sizeof(MallocMetadata));
}

void* scalloc(size_t num, size_t size){
    size_t total_size = num * size;
    void* mem_block = smalloc(total_size);
    if(!mem_block){
        return nullptr;
    }
    std::memset(mem_block, 0, total_size);
    stats.num_allocated_blocks++;
    stats.num_allocated_bytes += total_size;
    return mem_block;
}

size_t _num_free_blocks(){
    return stats.num_free_blocks;
}

size_t _num_free_bytes(){
    return stats.num_free_bytes;
}

size_t _num_allocated_blocks(){
    return stats.num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return stats.num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return stats.num_allocated_blocks * sizeof(MallocMetadata);
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}

