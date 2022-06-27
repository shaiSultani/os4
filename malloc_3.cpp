//
// Created by student on 6/22/22.
//
#include "malloc_3.h"

MallocStats stats;
SortedList list = SortedList();

/*-------------------------Sorted List----------------------------------*/

void SortedList::insert(MallocMetadata* newNode) {
    if (head == nullptr){//if list is empty
        head = newNode;
        wilderness = newNode;
    }
    else if (head->size > newNode->size || (head->size == newNode->size && head > newNode)) {  // insert as first node
        newNode->next = head;
        newNode->next->prev = newNode;
        head = newNode;
        wilderness = newNode->next;
    } else {
        MallocMetadata *temp = head;
        while (temp->next && (temp->next->size < newNode->size || (temp->next->size == newNode->size && temp->next < newNode))) {
            temp = temp->next;
        }
        ///insert newNode as temp->next
        newNode->next = temp->next;
        if (temp->next) //newNode is not inserted at the end of the list
            newNode->next->prev = newNode;
        else{
            wilderness = newNode;
        }
        temp->next = newNode;
        newNode->prev = temp;
    }
}

void SortedList::remove(MallocMetadata* node){
    if(node == head){
        head = node->next;
        head->prev = nullptr;
        return;
    }
    if(node == wilderness){
        wilderness = node->prev;
        wilderness->next = nullptr;
        return;
    }
    else{
        node->prev->next = node->next;
        if(node->next != nullptr)
            node->next->prev = node->prev;
    }
}

bool SortedList::merge(MallocMetadata* low, MallocMetadata* mid, MallocMetadata* high){
    if (high && mid!=high && high->is_free){
        list.remove(high);
        list.remove(mid);
        mid->size += high->size + _size_meta_data();
        mid->is_free = true;
        list.insert(mid);
        if (wilderness == high){
            wilderness = mid;
        }
        return true;
    }
    if (low && mid!= low&& low->is_free){
        list.remove(mid);
        list.remove(low);
        low->size += mid->size + _size_meta_data();
        list.insert(low);
        if (wilderness == mid){
            wilderness = low;
        }
        return true;
    }
    return false;
}

void SortedList::split(MallocMetadata* curr, size_t size) {
    list.remove(curr);
    curr->size = size;
    curr->is_free = false;
    list.insert(curr);
    MallocMetadata* free = (MallocMetadata*)(_size_meta_data()+size);
    free->size = curr->size - size - _size_meta_data();
    free->is_free = true;
    list.insert(free);
    if (wilderness == curr){
        wilderness = free;
        return;
    }
    MallocMetadata* high;
    list.getNeighbors(free, nullptr, &high);
    list.merge(nullptr, free, high);
}

void SortedList::getNeighbors(MallocMetadata* curr, MallocMetadata** low, MallocMetadata** high) {
    *low = 0;
    if (list.head){
        *low = list.head;
    }
    *high = list.wilderness;
    MallocMetadata* it = list.head;
    while (it){
        if (it > *low && it < curr){
            *low = it;
        }
        if ((it + it->size) < *high && it > curr){
            *high = it;
        }
        if (!it->next){
            break;
        }
        it = it->next;
    }
}


/*------------------------- Malloc functions----------------------------------*/

void* smalloc(size_t size){
    if (size == 0 || size > 100000000){
        return nullptr;
    }
    MallocMetadata* iterator = list.head;
    while (iterator){
        if (iterator->is_free && iterator->size >= size){
            long free_space = iterator->size - size - _size_meta_data();
            if (free_space >= 128){
                list.split(iterator, size);
            }
            else{
                iterator->is_free = false;
            }
            return (void*)((char*)iterator + sizeof(MallocMetadata));
        }
        if (!iterator->next){
            break;
        }
        iterator = iterator->next;
    }
    ///wilderness check
    MallocMetadata* wilderness = list.wilderness;
    if (!wilderness || !wilderness->is_free){ ///empty heap or full wilderness
        void* program_break = sbrk(size + sizeof(MallocMetadata));
        if (program_break == (void*)(-1)){
            return nullptr;
        }
        MallocMetadata* curr = (MallocMetadata*)program_break;
        curr->size = size;
        curr->is_free = false;
        list.insert(curr);
        wilderness = curr;
        return (void*)((char*)curr + sizeof(MallocMetadata));
    }
    else { ///wilderness free
        list.remove(wilderness);
        size_t gap = size - wilderness->size;
        void* program_break = sbrk(gap);
        wilderness->size = size;
        wilderness->is_free = false;
        list.insert(wilderness);
        return (void*)((char*)wilderness + sizeof(MallocMetadata));
    }
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
    MallocMetadata *curr= (MallocMetadata*)((char*)p - sizeof(MallocMetadata));
    if(curr->is_free)
        return;
    curr->is_free = true;
    MallocMetadata* low;
    MallocMetadata* high;
    list.getNeighbors(curr, &low, &high);
    list.merge(low, curr, high);
}

void* srealloc(void* oldp, size_t size) {
    if (size == 0 || size > 100000000)
        return nullptr;
    if (oldp == nullptr)
        return smalloc(size);
    MallocMetadata *curr = (MallocMetadata *) ((char *) oldp - sizeof(MallocMetadata));
    MallocMetadata *low;
    MallocMetadata *high;
    list.getNeighbors(curr, &low, &high);
    size_t low_size = low->size;
    size_t high_size = high->size;
    size_t curr_size = curr->size;

    ///a
    long free_space = curr_size - size;
    if (free_space) {
        if (free_space >= 128) {
            list.split(curr, size);
        } else {
            curr->is_free = false;
        }
        return (void *) ((char *) curr + sizeof(MallocMetadata));
    }

    ///b
    if (low && low != curr && low->is_free) {
        long free_space_b = curr_size + low_size + _size_meta_data() - size;
        if (free_space_b) {
            list.merge(low, curr, nullptr);
            if (free_space_b >= 128) {
                list.split(low, size);
            }
            std::memmove(low + _size_meta_data(), oldp, curr_size);
            return (void *) ((char *) low + sizeof(MallocMetadata));
        }
        if (list.wilderness == curr) {
            list.merge(low, curr, nullptr);
            list.remove(low);
            size_t gap = size - low->size;
            void *program_break = sbrk(gap);
            low->size = size;
            low->is_free = false;
            list.insert(low);
            std::memmove(low + _size_meta_data(), oldp, curr_size);
            return (void *) ((char *) curr + sizeof(MallocMetadata));
        }
    }

    ///c
    if (list.wilderness == curr) {
        list.remove(curr);
        size_t gap = size - curr->size;
        void *program_break = sbrk(gap);
        curr->size = size;
        curr->is_free = false;
        list.insert(curr);
        return (void *) ((char *) curr + sizeof(MallocMetadata));
    }

    ///d
    if (high && high != curr && high->is_free) {
        long free_space_d = curr_size + high_size + _size_meta_data() - size;
        if (free_space_d) {
            list.merge(nullptr, curr, high);
            if (free_space_d >= 128) {
                list.split(curr, size);
            }
            return (void *) ((char *) curr + sizeof(MallocMetadata));
        }
    }

    ///e
    long free_space_e = curr_size + high_size + low_size + 2 * _size_meta_data() - size;
    if (high && high != curr && high->is_free &&
        low && low != curr && low->is_free && free_space_e) {
        list.merge(low, curr, high);
        if (free_space_e >= 128) {
            list.split(low, size);
        }
        std::memmove(low + _size_meta_data(), oldp, curr_size);
        return (void *) ((char *) curr + sizeof(MallocMetadata));
    }

    ///f
    if (list.wilderness == high){
        if (high && high != curr && high->is_free &&
            low && low != curr && low->is_free) {
            list.merge(low, curr, high);
            size_t gap = -free_space_e;
            void *program_break = sbrk(gap);
            list.remove(low);
            low->size = size;
            list.insert(low);
            low->is_free = false;
            std::memmove(low + _size_meta_data(), oldp, curr_size);
            return (void *) ((char *) low + sizeof(MallocMetadata));
        }
        if (high && high != curr && high->is_free) {
            list.merge(nullptr, curr, high);
            size_t gap = size - high_size - _size_meta_data() - curr_size;
            void *program_break = sbrk(gap);
            list.remove(curr);
            curr->size = size;
            list.insert(curr);
            curr->is_free = false;
            return (void *) ((char *) curr + sizeof(MallocMetadata));
        }
    }

    ///h
    sfree(curr + _size_meta_data());
    return smalloc(size);
}

void update() {
    stats.num_free_bytes = 0;
    stats.num_allocated_blocks = 0;
    stats.num_free_blocks = 0;
    stats.num_allocated_bytes = 0;
    MallocMetadata *it = list.head;
    while (it) {
        stats.num_allocated_blocks++;
        stats.num_allocated_bytes += it->size;
        if (it->is_free) {
            stats.num_free_blocks++;
            stats.num_free_bytes += it->size;
        }
        if (!it->next) {
            break;
        }
        it = it->next;
    }
}

size_t _num_free_blocks(){
    update();
    return stats.num_free_blocks;
}

size_t _num_free_bytes(){
    update();
    return stats.num_free_bytes;
}

size_t _num_allocated_blocks(){
    update();
    return stats.num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    update();
    return stats.num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    update();
    return stats.num_allocated_blocks * sizeof(MallocMetadata);
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}