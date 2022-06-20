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
    MallocMetadata* current = head;
    while (current){
        if (current->is_free && current->size>=size){
            current->is_free = false;
            return (void*)(current + sizeof(MallocMetadata));
        }
        current = current->next;
    }
    void* ptr = sbrk(size);
    if (ptr == (void*)(-1)){
        return nullptr;
    }
    return ptr;
}

