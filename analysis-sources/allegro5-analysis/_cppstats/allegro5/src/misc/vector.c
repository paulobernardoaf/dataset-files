#include <stdlib.h>
#include <string.h>
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_vector.h"
#define ITEM_START(vec, idx) (vec->_items + ((idx) * vec->_itemsize))
void _al_vector_init(_AL_VECTOR *vec, size_t itemsize)
{
ASSERT(vec);
ASSERT(itemsize > 0);
vec->_itemsize = itemsize;
vec->_items = NULL;
vec->_size = 0;
vec->_unused = 0;
}
void *_al_vector_ref(const _AL_VECTOR *vec, unsigned int idx)
{
ASSERT(vec);
ASSERT(idx < vec->_size);
return ITEM_START(vec, idx);
}
void* _al_vector_ref_front(const _AL_VECTOR *vec)
{
return _al_vector_ref(vec, 0);
}
void* _al_vector_ref_back(const _AL_VECTOR *vec)
{
ASSERT(vec);
return _al_vector_ref(vec, vec->_size-1);
}
bool _al_vector_append_array(_AL_VECTOR *vec, unsigned int num, const void *arr)
{
ASSERT(vec);
ASSERT(arr);
ASSERT(num);
if (vec->_items == NULL) {
ASSERT(vec->_size == 0);
ASSERT(vec->_unused == 0);
vec->_items = al_malloc(vec->_itemsize * num);
ASSERT(vec->_items);
if (!vec->_items)
return false;
vec->_unused = num;
}
else if (vec->_unused < num) {
char *new_items;
new_items = al_realloc(vec->_items, (vec->_size + num) * vec->_itemsize);
ASSERT(new_items);
if (!new_items)
return false;
vec->_items = new_items;
vec->_unused = num;
}
memcpy(vec->_items + (vec->_size * vec->_itemsize),
arr, vec->_itemsize * num);
vec->_size += num;
vec->_unused -= num;
return true;
}
void* _al_vector_alloc_back(_AL_VECTOR *vec)
{
ASSERT(vec);
ASSERT(vec->_itemsize > 0);
{
if (vec->_items == NULL) {
ASSERT(vec->_size == 0);
ASSERT(vec->_unused == 0);
vec->_items = al_malloc(vec->_itemsize);
ASSERT(vec->_items);
if (!vec->_items)
return NULL;
vec->_unused = 1;
}
else if (vec->_unused == 0) {
char *new_items = al_realloc(vec->_items, 2 * vec->_size * vec->_itemsize);
ASSERT(new_items);
if (!new_items)
return NULL;
vec->_items = new_items;
vec->_unused = vec->_size;
}
vec->_size++;
vec->_unused--;
return ITEM_START(vec, vec->_size-1);
}
}
void* _al_vector_alloc_mid(_AL_VECTOR *vec, unsigned int index)
{
ASSERT(vec);
ASSERT(vec->_itemsize > 0);
{
if (vec->_items == NULL) {
ASSERT(index == 0);
return _al_vector_alloc_back(vec);
}
if (vec->_unused == 0) {
char *new_items = al_realloc(vec->_items, 2 * vec->_size * vec->_itemsize);
ASSERT(new_items);
if (!new_items)
return NULL;
vec->_items = new_items;
vec->_unused = vec->_size;
}
memmove(ITEM_START(vec, index + 1), ITEM_START(vec, index),
vec->_itemsize * (vec->_size - index));
vec->_size++;
vec->_unused--;
return ITEM_START(vec, index);
}
}
int _al_vector_find(const _AL_VECTOR *vec, const void *ptr_item)
{
ASSERT(vec);
ASSERT(ptr_item);
if (vec->_itemsize == sizeof(void *)) {
void **items = (void **)vec->_items;
unsigned int i;
for (i = 0; i < vec->_size; i++)
if (items[i] == *(void **)ptr_item)
return i;
}
else {
unsigned int i;
for (i = 0; i < vec->_size; i++)
if (memcmp(ITEM_START(vec, i), ptr_item, vec->_itemsize) == 0)
return i;
}
return -1;
}
bool _al_vector_contains(const _AL_VECTOR *vec, const void *ptr_item)
{
return _al_vector_find(vec, ptr_item) >= 0;
}
void _al_vector_delete_at(_AL_VECTOR *vec, unsigned int idx)
{
ASSERT(vec);
ASSERT(idx < vec->_size);
{
int to_move = vec->_size - idx - 1;
if (to_move > 0)
memmove(ITEM_START(vec, idx),
ITEM_START(vec, idx+1),
to_move * vec->_itemsize);
vec->_size--;
vec->_unused++;
memset(ITEM_START(vec, vec->_size), 0, vec->_itemsize);
}
}
bool _al_vector_find_and_delete(_AL_VECTOR *vec, const void *ptr_item)
{
int idx = _al_vector_find(vec, ptr_item);
if (idx >= 0) {
_al_vector_delete_at(vec, idx);
return true;
}
else
return false;
}
void _al_vector_free(_AL_VECTOR *vec)
{
ASSERT(vec);
if (vec->_items != NULL) {
al_free(vec->_items);
vec->_items = NULL;
}
vec->_size = 0;
vec->_unused = 0;
}
