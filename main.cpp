#include <iostream>
#include "malloc_3.h"
static inline size_t aligned_size(size_t size)
{
    return (size % 8) ? (size & (size_t)(-8)) + 8 : size;
}

void get_stats (size_t* num_free_blocks, size_t* num_free_bytes, size_t* num_allocated_blocks, size_t* num_allocated_bytes){
    *num_free_blocks = _num_free_blocks();
    *num_free_bytes = _num_free_bytes();
    *num_allocated_blocks = _num_allocated_blocks();
    *num_allocated_bytes = _num_allocated_bytes();
}

int main() {
    size_t num_free_blocks;
    size_t num_free_bytes;
    size_t num_allocated_blocks;
    size_t num_allocated_bytes;
    get_stats(&num_free_blocks, &num_free_bytes, &num_allocated_blocks, &num_allocated_bytes);
    //verify_blocks(0, 0, 0, 0);
    void *base = sbrk(0);
    char *pad1 = (char *)smalloc(32);
    char *a = (char *)smalloc(32);
    char *pad2 = (char *)smalloc(32);
    char *b = (char *)smalloc(160);
    char *pad3 = (char *)smalloc(32);
    //REQUIRE(pad1 != nullptr);
    //REQUIRE(a != nullptr);
    //REQUIRE(pad2 != nullptr);
    //REQUIRE(b != nullptr);
    //REQUIRE(pad3 != nullptr);

    size_t pad_size = 32 * 3;
    size_t blocks_size = 32 + 160;

    //verify_blocks(5, blocks_size + pad_size, 0, 0);
    //verify_size(base);
    //populate_array(a, 32);

    sfree(b);
    //verify_blocks(5, blocks_size + pad_size, 1, 160);
    //verify_size(base);

    char *new_a = (char *)srealloc(a, 160);
    //REQUIRE(new_a != nullptr);
    //REQUIRE(new_a == b);
    //verify_blocks(5, blocks_size + pad_size, 1, 32);
    //verify_size(base);
    //validate_array(new_a, 32);

    sfree(new_a);
    //verify_blocks(5, blocks_size + pad_size, 2, blocks_size);
    //verify_size(base);

    sfree(pad1);
    sfree(pad2);
    sfree(pad3);
    //verify_blocks(1, blocks_size + pad_size + 4 * _size_meta_data(), 1, blocks_size + pad_size + 4 * _size_meta_data());
    //verify_size(base);
}