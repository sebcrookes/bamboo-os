#pragma once

#include <stdint.h>

typedef struct {
    uint64_t length;
    void** items;
} vector_t;

vector_t* vector_init();
void vector_free(vector_t* v);

/* Returns the size of the vector */
uint64_t vector_size(vector_t* v);

/* Adds an item to the vector at the end */
void vector_add(vector_t* v, void* address);

/* Gets an item at the given index
Returns nullptr if the index was out of bounds */
void* vector_get(vector_t* v, uint64_t index);

/* Inserts an item at the given index */
void vector_insert(vector_t* v, uint64_t index, void* address);

/* Frees all items in the vector
Warning: presumes all items are actually freeable */
void vector_free_all(vector_t* v);

/* Deletes all items in the vector without freeing them */
void vector_delete_all(vector_t* v);

/* Deletes the item at the given index and shifts everything
ahead of that index down one item */
void vector_delete_at(vector_t* v, uint64_t index);
