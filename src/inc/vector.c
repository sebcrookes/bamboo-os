#include "vector.h"

#include "../mem/heap.h"

/*
    Vectors are arrays which can grow dynamically in BambooOS.
    A vector is simply a void** which changes size dynamically depending on 
    whether you add / delete items
*/

vector_t* vector_init() {
    vector_t* v = (vector_t*) malloc(sizeof(vector_t));

    v->length = 0;
    v->items = (void**) malloc(sizeof(void*));

    return v;
}

void vector_free(vector_t* v) {
    v->length = 0;
    free((void*)v->items);
    free((void*)v);
}

uint64_t vector_size(vector_t* v) {
    return v->length;
}

void vector_add(vector_t* v, void* address) {
    v->length++;
    v->items = (void**) realloc((void*) v->items, v->length * sizeof(void*));
    v->items[v->length - 1] = address;
}

void* vector_get(vector_t* v, uint64_t index) {
    if(index >= v->length)
        return NULL;

    return v->items[index];
}

void vector_insert(vector_t* v, uint64_t index, void* address) {
    if(index > v->length) return;
    v->length++;
    v->items = (void**) realloc((void*) v->items, v->length * sizeof(void*));
    for (uint64_t i = v->length - 1; i > index; i--) {
        v->items[i] = v->items[i - 1];
    }
    
    v->items[index] = address;
}

void vector_free_all(vector_t* v) {
    for(uint64_t i = 0; i < v->length; i++) {
        free(v->items[i]);
    }
    vector_delete_all(v);
}

void vector_delete_all(vector_t* v) {
    v->length = 0;
    free(v->items);
    v->items = (void**) malloc(sizeof(void*));
}

void vector_delete_at(vector_t* v, uint64_t index) {
    if(index + 1 < v->length) {
        for(uint64_t i = index + 1; i < v->length; i++) {
            v->items[i - 1] = v->items[i];
        }
    }
    v->length--;
    //v->items = (void**) realloc((void*) v->items, v->length * sizeof(void*));
}
