#include <iostream>
#include "malloc_3.h"

void get_stats (size_t* num_free_blocks, size_t* num_free_bytes, size_t* num_allocated_blocks, size_t* num_allocated_bytes){
    *num_free_blocks = _num_free_blocks();
    *num_free_bytes = _num_free_bytes();
    *num_allocated_blocks = _num_allocated_blocks();
    *num_allocated_bytes = _num_allocated_bytes();
}

int main() {
    {
        size_t num_free_blocks;
        size_t num_free_bytes;
        size_t num_allocated_blocks;
        size_t num_allocated_bytes;
        //verify_blocks(0, 0, 0, 0);

        void *base = sbrk(0);
        char *a = (char *)smalloc(10);
        //REQUIRE(a != nullptr);
        char *b = (char *)smalloc(10);
        //REQUIRE(b != nullptr);
        char *c = (char *)smalloc(10);
        //REQUIRE(c != nullptr);

        //verify_blocks(3, 16 * 3, 0, 0);
        //verify_size(base);

        sfree(b);
        //verify_blocks(3, 16 * 3, 1, 16);
        //verify_size(base);
        sfree(a);
        //verify_blocks(2, 16 * 3 + _size_meta_data(), 1, 16 * 2 + _size_meta_data());
        //verify_size(base);
        sfree(c);
        //verify_blocks(1, 16 * 3 + _size_meta_data() * 2, 1, 16 * 3 + _size_meta_data() * 2);
        //verify_size(base);

        char *new_a = (char *)smalloc(10);
        //REQUIRE(a == new_a);
        char *new_b = (char *)smalloc(10);
        //REQUIRE(b != new_b);
        char *new_c = (char *)smalloc(10);
        //REQUIRE(c != new_c);

        //verify_blocks(3, 16 * 5 + _size_meta_data() * 2, 0, 0);
        //verify_size(base);

        sfree(new_a);
        //verify_blocks(3, 16 * 5 + _size_meta_data() * 2, 1, 16 * 3 + _size_meta_data() * 2);
        //verify_size(base);
        sfree(new_b);
        //verify_blocks(2, 16 * 5 + _size_meta_data() * 3, 1, 16 * 4 + _size_meta_data() * 3);
        //verify_size(base);
        sfree(new_c);
        //verify_blocks(1, 16 * 5 + _size_meta_data() * 4, 1, 16 * 5 + _size_meta_data() * 4);
        //verify_size(base);
    }
}
