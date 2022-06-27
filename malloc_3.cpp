//
// Created by student on 6/22/22.
//
#include "malloc_3.h"

const int MMAP_needed_size = 131072;
MallocStats stats;
SortedList list = SortedList();
mmapList mmap_list = mmapList();

/*-------------------------Sorted List----------------------------------*/

void SortedList::insert(MallocMetadata* newNode) {
    if (!wilderness || newNode > wilderness){
        wilderness = newNode;
    }
    if (head == nullptr){//if list is empty
        head = newNode;
    }
    else if (head->size > newNode->size || (head->size == newNode->size && head > newNode)) {  // insert as first node
        newNode->next = head;
        newNode->next->prev = newNode;
        head = newNode;
    } else {
        MallocMetadata *temp = head;
        while (temp->next && (temp->next->size < newNode->size || (temp->next->size == newNode->size && temp->next < newNode))) {
            temp = temp->next;
        }
        ///insert newNode as temp->next
        newNode->next = temp->next;
        if (temp->next) //newNode is not inserted at the end of the list
            newNode->next->prev = newNode;
        temp->next = newNode;
        newNode->prev = temp;
    }
}

void SortedList::remove(MallocMetadata* node){
    if (node == wilderness){
        MallocMetadata* it = list.head;
        if (!it){
            wilderness = nullptr;
        }
        MallocMetadata* new_wilderness = 0;
        while (it){
            if (it > new_wilderness && it != node){
                new_wilderness = it;
            }
            if (it->next){
                break;
            }
            it = it->next;
        }
    }
    if(node == head){
        if (head->next){
            head = node->next;
            head->prev = nullptr;
            return;
        }
        head = nullptr;
        wilderness = nullptr;
        return;
    }
    else{
        node->prev->next = node->next;
        if(node->next != nullptr)
            node->next->prev = node->prev;
    }
}

bool SortedList::merge(MallocMetadata* low, MallocMetadata* mid, MallocMetadata* high){
    bool merged = false;
    if (high && mid!=high && high->is_free){
        list.remove(high);
        list.remove(mid);
        mid->size += high->size + _size_meta_data();
        mid->is_free = true;
        list.insert(mid);
        if (wilderness == high){
            wilderness = mid;
        }
        merged = true;
    }
    if (low && mid!= low&& low->is_free){
        list.remove(mid);
        list.remove(low);
        low->size += mid->size + _size_meta_data();
        list.insert(low);
        if (wilderness == mid){
            wilderness = low;
        }
        merged = true;
    }
    return merged;
}

void SortedList::split(MallocMetadata* curr, size_t size) {
    size_t curr_size = curr->size;
    list.remove(curr);
    curr->size = size;
    curr->is_free = false;
    list.insert(curr);
    void* dst = (char*)curr + _size_meta_data() + size;
    MallocMetadata* free = static_cast<MallocMetadata *>(std::memmove(dst, curr,
                                                                      _size_meta_data()));
    free->size = curr_size - size - _size_meta_data();
    free->is_free = true;
    list.insert(free);
    if (wilderness == curr){
        wilderness = free;
        return;
    }
    MallocMetadata* high;
    MallocMetadata* low;
    list.getNeighbors(free, &low, &high);
    list.merge(free, free, high);
}

void SortedList::getNeighbors(MallocMetadata* curr, MallocMetadata** low, MallocMetadata** high) {
    *low = 0;
    *high = list.wilderness;
    MallocMetadata* it = list.head;
    while (it){
        if (it > *low && it < curr){
            *low = it;
        }
        if (((char*)it + it->size + _size_meta_data()) <= (char*)*high && it > curr){
            *high = it;
        }
        if (!it->next){
            break;
        }
        it = it->next;
    }
    if (*low == 0){
        *low = curr;
    }
}
/*-------------------------MMAP List------------------------------------------*/
void mmapList::insert(MallocMetadata* newNode) {
    // list is empty
    if(head == nullptr){
        newNode->prev = nullptr;
        newNode->next = nullptr;
        head = newNode;
        tail = newNode;
        return;
    }
    // only one element
    if(head == tail) {
        tail = newNode;
        tail->prev = head;
        head->next = tail;
        tail->next = nullptr;
        return;
    }
    tail->next = newNode;
    newNode->prev=tail;
    newNode->next = nullptr;
    tail = newNode;
}
void mmapList::remove(MallocMetadata* node) {
    if(node == nullptr)
        return;
    if( head == tail){ //only one element
        head = nullptr;
        tail = nullptr;
        return;
    }
    if(node == head){
        head = node-> next;
        head->prev= nullptr;
        return;
    }
    if(node == tail){
        tail = node->prev;
        tail->next = nullptr;
        return;
    }
    node->prev->next = node->next;
    if(node->next != nullptr)
        node->next->prev = node->prev;
}

/*------------------------- Malloc functions----------------------------------*/

void* smalloc(size_t size){
    if (size == 0 || size > 100000000){
        return nullptr;
    }
    if (size%8){
        size += 8 - size%8;
    }
    if(size>=MMAP_needed_size){
        void* alloc = mmap(nullptr, sizeof(MallocMetadata) + size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,0);
        if(alloc == MAP_FAILED)
            return nullptr;
        MallocMetadata* new_alloc = (MallocMetadata*)alloc;
        new_alloc->size = size;
        new_alloc->is_free = false;
        new_alloc->next= nullptr;
        new_alloc->prev= nullptr;
        mmap_list.insert(new_alloc);
        return (void*)((char*)alloc + sizeof(MallocMetadata));
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
        if (!wilderness && (size_t)sbrk(0)%8 != 0){
            sbrk (8 - (size_t)sbrk(0)%8);
        }
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
        sbrk(gap);
        wilderness->size = size;
        wilderness->is_free = false;
        list.insert(wilderness);
        return (void*)((char*)wilderness + sizeof(MallocMetadata));
    }
}

void* scalloc(size_t num, size_t size){
    size_t total_size = num * size;
    if (total_size%8){
        total_size += 8 - total_size%8;
    }
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
    if(curr->size >= MMAP_needed_size){
        mmap_list.remove(curr);
        munmap((void*)curr, curr->size + sizeof(MallocMetadata));
        return;
    }
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
    if (size%8){
        size += 8 - size%8;
    }
    MallocMetadata *curr = (MallocMetadata *) ((char *) oldp - sizeof(MallocMetadata));
    if(size>= MMAP_needed_size){ //mmap block
        if(curr->size == size)
            return oldp;
        void* new_alloc = smalloc(size);
        if(!new_alloc)
            return nullptr;
        size_t n_size = curr->size < size? curr->size : size;
        std::memmove(new_alloc,oldp,n_size);
        mmap_list.remove(curr);
        munmap((void*)curr, sizeof(MallocMetadata)+ curr->size);
        return new_alloc;
    }

    MallocMetadata *low;
    MallocMetadata *high;
    list.getNeighbors(curr, &low, &high);
    size_t low_size = low->size;
    size_t high_size = high->size;
    size_t curr_size = curr->size;

    ///a
    long free_space = curr_size - size;
    if (free_space >= 0) {
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
        if (free_space_b >= 0) {
            list.merge(low, curr, nullptr);
            low->is_free = false;
            if (free_space_b >= 128) {
                list.split(low, size);
            }
            std::memmove((char*)low + _size_meta_data(), oldp, curr_size);
            return (void *) ((char *) low + sizeof(MallocMetadata));
        }
        if (list.wilderness == curr) {
            list.merge(low, curr, nullptr);
            list.remove(low);
            size_t gap = size - low->size;
            sbrk(gap);
            low->size = size;
            low->is_free = false;
            list.insert(low);
            std::memmove((char*)low + _size_meta_data(), oldp, curr_size);
            return (void *) ((char *) low + sizeof(MallocMetadata));
        }
    }

    ///c
    if (list.wilderness == curr) {
        list.remove(curr);
        size_t gap = size - curr->size;
        sbrk(gap);
        curr->size = size;
        curr->is_free = false;
        list.insert(curr);
        return (void *) ((char *) curr + sizeof(MallocMetadata));
    }

    ///d
    if (high && high != curr && high->is_free) {
        long free_space_d = curr_size + high_size + _size_meta_data() - size;
        if (free_space_d >= 0) {
            list.merge(nullptr, curr, high);
            curr->is_free = false;
            if (free_space_d >= 128) {
                list.split(curr, size);
            }
            return (void *) ((char *) curr + sizeof(MallocMetadata));
        }
    }

    ///e
    long free_space_e = curr_size + high_size + low_size + 2 * _size_meta_data() - size;
    if (high && high != curr && high->is_free &&
        low && low != curr && low->is_free && free_space_e >= 0) {
        list.merge(low, curr, high);
        low->is_free = false;
        if (free_space_e >= 128) {
            list.split(low, size);
        }
        std::memmove((char*)low + _size_meta_data(), oldp, curr_size);
        return (void *) ((char *) low + sizeof(MallocMetadata));
    }

    ///f
    if (list.wilderness == high){
        if (high && high != curr && high->is_free &&
            low && low != curr && low->is_free) {
            list.merge(low, curr, high);
            size_t gap = -free_space_e;
            sbrk(gap);
            list.remove(low);
            low->size = size;
            list.insert(low);
            low->is_free = false;
            std::memmove((char*)low + _size_meta_data(), oldp, curr_size);
            return (void *) ((char *) low + sizeof(MallocMetadata));
        }
        if (high && high != curr && high->is_free) {
            list.merge(nullptr, curr, high);
            size_t gap = size - high_size - _size_meta_data() - curr_size;
            sbrk(gap);
            list.remove(curr);
            curr->size = size;
            list.insert(curr);
            curr->is_free = false;
            return (void *) ((char *) curr + sizeof(MallocMetadata));
        }
    }

    ///h
    sfree((char*)curr + _size_meta_data());
    void* target = smalloc(size);
    std::memmove(target, oldp, curr_size);
    return target;
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
    it = mmap_list.head;
    while(it){
        stats.num_allocated_blocks++;
        stats.num_allocated_bytes += it->size;
        if(!it->next)
            break;
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