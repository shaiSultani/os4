//
// Created by student on 6/20/22.
//
#include "malloc_1.h"

void* smalloc(size_t size){
    if (size == 0 || size > 100000000){
        return nullptr;
    }
    void* ptr = sbrk(size);
    if (ptr == (void*)(-1)){
        return nullptr;
    }
    return ptr;
}

