#include <stdio.h>
#include <stdarg.h>
#include "memalloc.h"
#include "list.h"
#include <pthread.h>


struct list free_list;   

pthread_mutex_t lock;

// Compares the addresses of a and b to order the free list
bool block_compare(const struct list_elem *a, const struct list_elem *b, void *aux) {
    return (a < b);
}

// Helper to combine 2 adjacent blocks on the free list
void combine_blocks (struct free_block * l, struct free_block * r) {
    l->length = l->length + r->length;
    list_remove(&r->elem);            
}

// Helper to check if 2 blocks in the free list are adjacent
bool is_adjacent (struct free_block * l, struct free_block * r) {
    return (void *)l + l->length == (void *)r;
}

// Returns the size of the free list
size_t mem_sizeof_free_list(void) {
    return list_size(&free_list);
}

// Prints the free list
void mem_dump_free_list(void) {
    struct list_elem* elem_p;
    struct free_block* fp;
    struct list * l = &free_list;
    int i = 0;

    // Loops through and prints the elements of the free list
    for (elem_p = list_begin(l); elem_p != list_end(l); elem_p = list_next(elem_p)) {
        fp = list_entry(elem_p, struct free_block, elem);
        printf("list[%d]: at%ld; len=%zu\n", i++, (long)fp, fp->length);
    }
}

// Initializes the memory and the free list
void mem_init(uint8_t *base, size_t length) {
    // Initializes list
    list_init(&free_list);
    // Creates free block the size of length at base
    struct free_block* first_node = (struct free_block*) base;
    first_node->length = length;
    // Adds free block to list
    list_push_front(&free_list, &first_node->elem);
    // Creates the mutex lock that will be used
    pthread_mutex_init(&lock, NULL);
}


// Allocates memory to users from free list
// Uses first-fit method
void * mem_alloc(size_t length) {
    struct list_elem* elem_p = NULL;
    struct free_block* fp = NULL;
    struct used_block* up = NULL;
    size_t adjusted_length;

    // Checks if memory asked for can fit a free block when user frees it
    if (length + sizeof(struct used_block) < sizeof(struct free_block)) {
        // Pads memory given to user
        adjusted_length = sizeof(struct free_block);
        length = adjusted_length - sizeof(struct used_block);
    }

    else {
        // Allocates memory given with space of the used_block included
        adjusted_length = length + sizeof(struct used_block);
    }

    pthread_mutex_lock(&lock);

    // Loops through list looking for a free block that fits memory asked for
    for (elem_p = list_begin(&free_list); elem_p != list_end(&free_list); elem_p = list_next(elem_p)) {
        fp = list_entry(elem_p, struct free_block, elem);        
        // If the length of block equals memory requested or will have unusable leftovers
        if (fp->length == adjusted_length || (fp->length - adjusted_length) < sizeof(struct free_block)) {
           // Uses whole block since leftovers cannot hold a free block 
           if (fp->length != adjusted_length) {
                // Pads the length asked for and readjusts to compensate for the big free block
                adjusted_length = fp->length;
                length = adjusted_length - sizeof(struct used_block);
            }
            // Removes free element from free list and sets memory address and length of the used block
            list_remove(elem_p);
            up = (struct used_block*) fp;
            up->length = length;
            pthread_mutex_unlock(&lock);
            // Returns memory address
            return (void*) up->data;

        } else if (fp->length > adjusted_length) {
            // Removes memory needed from the 'back' of the free block
            fp->length = fp->length - adjusted_length;
            // Gives memory address and length to the used block 
            up = (struct used_block*)((void*) fp + fp->length);
            up->length = length;
            pthread_mutex_unlock(&lock);
            // Returns memory address
            return (void*) up->data;
        }
    }

    pthread_mutex_unlock(&lock);
    // No suitable memory block found so returns NULL
    return NULL;
}

// Frees used block at address given by ptr and adds it to free list
void mem_free(void *ptr) {
    struct list_elem* temp_elem_p;
    struct used_block* up;
    struct free_block* fp;
    struct free_block* temp_fp;
    size_t adjusted_length;


    pthread_mutex_lock(&lock);

    // Initializes a used block at the address given - the header
    up = (struct used_block*) (ptr - sizeof(struct used_block));
    adjusted_length = up->length + sizeof(struct used_block);

    // Initializes a free block at address of used block and inserts it to list
    fp = (struct free_block*) up;
    fp->length = adjusted_length;
    list_insert_ordered(&free_list, &fp->elem, block_compare, NULL);

    // Check previous for head
    temp_elem_p = list_prev(&fp->elem);
    if (temp_elem_p != list_head(&free_list)) {
        temp_fp = list_entry(temp_elem_p, struct free_block, elem);

        // Checks for memory adjacency
        if (is_adjacent(temp_fp, fp)) {
            // Coalesces the two blocks
            combine_blocks(temp_fp, fp);
            // Update pointer
            fp = temp_fp;
        }
    }
    // Check next for tail
    temp_elem_p = list_next(&fp->elem);
    if (temp_elem_p != list_tail(&free_list)) {
        temp_fp = list_entry(temp_elem_p, struct free_block, elem);
        // Checks for memory adjacency
        if (is_adjacent(fp, temp_fp)) {
            // Coalesces the blocks
            combine_blocks(fp, temp_fp);           
        }
    }
    pthread_mutex_unlock(&lock);
}
