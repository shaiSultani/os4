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
            stats.num_free_bytes -= iterator->size;
            stats.num_free_blocks --;
            return (void*)((char*)iterator + sizeof(MallocMetadata));
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
    return (void*)((char*)curr + sizeof(MallocMetadata));
}

void* scalloc(size_t num, size_t size){
    size_t total_size = num * size;
    void* mem_block = smalloc(total_size);
    if(!mem_block){
        return nullptr;
    }
    std::memset(mem_block, 0, total_size);
    return mem_block;
}

void sfree(void* p){
    if(p== nullptr)
        return;
    MallocMetadata *curr_metadata= (MallocMetadata*)((char*)p - sizeof(MallocMetadata));
    if(curr_metadata->is_free)
        return;
    curr_metadata->is_free = true;
    stats.num_free_bytes += curr_metadata->size;
    stats.num_free_blocks ++;
}

void* srealloc(void* oldp, size_t size){
    if (size == 0 || size > 100000000)
        return nullptr;
    if (oldp == nullptr)
        return smalloc(size);
    MallocMetadata *curr_metadata= (MallocMetadata*)((char*)oldp - sizeof(MallocMetadata));
    size_t old_size = curr_metadata->size;
    if( old_size >= size)
        return oldp;
    void* newp = smalloc(size);
    if (newp == nullptr)
        return nullptr;
    std::memmove(newp, oldp, curr_metadata->size);
    curr_metadata->is_free = true;
    stats.num_free_bytes += old_size;
    stats.num_free_blocks ++;
    return newp;
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

