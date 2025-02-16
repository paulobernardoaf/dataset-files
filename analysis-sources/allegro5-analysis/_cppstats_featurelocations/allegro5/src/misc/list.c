





















#include <stdlib.h>
#include <string.h>

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_list.h"


ALLEGRO_DEBUG_CHANNEL("list")



struct _AL_LIST {



_AL_LIST_ITEM* root;
size_t size;
size_t capacity;
size_t item_size;
size_t item_size_with_extra;
_AL_LIST_ITEM* next_free;
void* user_data;
_AL_LIST_DTOR dtor;
};


struct _AL_LIST_ITEM {
_AL_LIST* list;
_AL_LIST_ITEM* next;
_AL_LIST_ITEM* prev;
void* data;
_AL_LIST_ITEM_DTOR dtor;
};



static _AL_LIST* list_do_create(size_t capacity, size_t item_extra_size);
static bool list_is_static(_AL_LIST* list);

static _AL_LIST_ITEM* list_get_free_item(_AL_LIST* list);
static _AL_LIST_ITEM* list_create_item(_AL_LIST* list);
static void list_destroy_item(_AL_LIST* list, _AL_LIST_ITEM* item);




























static _AL_LIST* list_do_create(size_t capacity, size_t extra_item_size)
{
size_t i;
size_t memory_size;
uint8_t* memory_ptr;
_AL_LIST* list = NULL;
_AL_LIST_ITEM* item = NULL;
_AL_LIST_ITEM* prev = NULL;






memory_size = sizeof(_AL_LIST) + (capacity + 1) * (sizeof(_AL_LIST_ITEM) + extra_item_size);

memory_ptr = (uint8_t*)al_malloc(memory_size);
if (NULL == memory_ptr) {
ALLEGRO_ERROR("Out of memory.");
return NULL;
}

list = (_AL_LIST*)memory_ptr;
memory_ptr += sizeof(_AL_LIST);
list->size = 0;
list->capacity = capacity;
list->item_size = sizeof(_AL_LIST_ITEM);
list->item_size_with_extra = sizeof(_AL_LIST_ITEM) + extra_item_size;
list->next_free = (_AL_LIST_ITEM*)memory_ptr;
list->user_data = NULL;
list->dtor = NULL;



prev = NULL;
item = list->next_free;
for (i = 0; i <= list->capacity; ++i) {

memory_ptr += list->item_size_with_extra;
item->list = list;
item->next = (_AL_LIST_ITEM*)memory_ptr;
prev = item;
item = item->next;
}


prev->next = NULL;


list->root = list_get_free_item(list);
list->root->dtor = NULL;
list->root->next = list->root;
list->root->prev = list->root;

return list;
}





static bool list_is_static(_AL_LIST* list)
{
return 0 != list->capacity;
}






static _AL_LIST_ITEM* list_get_free_item(_AL_LIST* list)
{
_AL_LIST_ITEM* item;





item = list->next_free;
if (NULL != item)
list->next_free = item->next;

return item;
}





static _AL_LIST_ITEM* list_create_item(_AL_LIST* list)
{
_AL_LIST_ITEM* item = NULL;

if (list_is_static(list)) {




item = list_get_free_item(list);
}
else {

item = (_AL_LIST_ITEM*)al_malloc(list->item_size_with_extra);

item->list = list;
}

return item;
}






static void list_destroy_item(_AL_LIST* list, _AL_LIST_ITEM* item)
{
ASSERT(list == item->list);

if (NULL != item->dtor)
item->dtor(item->data, list->user_data);

if (list_is_static(list)) {
item->next = list->next_free;
list->next_free = item;
}
else
al_free(item);
}








_AL_LIST* _al_list_create(void)
{
return list_do_create(0, 0);
}









_AL_LIST* _al_list_create_static(size_t capacity)
{
if (capacity < 1) {

ALLEGRO_ERROR("Cannot create static list without any capacity.");
return NULL;
}

return list_do_create(capacity, 0);
}






void _al_list_destroy(_AL_LIST* list)
{
if (NULL == list)
return;

if (list->dtor)
list->dtor(list->user_data);

_al_list_clear(list);

al_free(list);
}





void _al_list_set_dtor(_AL_LIST* list, _AL_LIST_DTOR dtor)
{
list->dtor = dtor;
}





_AL_LIST_DTOR _al_list_get_dtor(_AL_LIST* list)
{
return list->dtor;
}







_AL_LIST_ITEM* _al_list_push_front(_AL_LIST* list, void* data)
{
return _al_list_insert_after(list, list->root, data);
}






_AL_LIST_ITEM* _al_list_push_front_ex(_AL_LIST* list, void* data, _AL_LIST_ITEM_DTOR dtor)
{
return _al_list_insert_after_ex(list, list->root, data, dtor);
}







_AL_LIST_ITEM* _al_list_push_back(_AL_LIST* list, void* data)
{
return _al_list_insert_before(list, list->root, data);
}






_AL_LIST_ITEM* _al_list_push_back_ex(_AL_LIST* list, void* data, _AL_LIST_ITEM_DTOR dtor)
{
return _al_list_insert_before_ex(list, list->root, data, dtor);
}





void _al_list_pop_front(_AL_LIST* list)
{
if (list->size > 0)
_al_list_erase(list, list->root->next);
}





void _al_list_pop_back(_AL_LIST* list)
{
if (list->size > 0)
_al_list_erase(list, list->root->prev);
}







_AL_LIST_ITEM* _al_list_insert_after(_AL_LIST* list, _AL_LIST_ITEM* where, void* data)
{
return _al_list_insert_after_ex(list, where, data, NULL);
}






_AL_LIST_ITEM* _al_list_insert_after_ex(_AL_LIST* list, _AL_LIST_ITEM* where, void* data, _AL_LIST_ITEM_DTOR dtor)
{
_AL_LIST_ITEM* item;

ASSERT(list == where->list);

item = list_create_item(list);
if (NULL == item)
return NULL;

item->data = data;
item->dtor = dtor;

item->prev = where;
item->next = where->next;

where->next->prev = item;
where->next = item;

list->size++;

return item;
}







_AL_LIST_ITEM* _al_list_insert_before(_AL_LIST* list, _AL_LIST_ITEM* where, void* data)
{
return _al_list_insert_before_ex(list, where, data, NULL);
}






_AL_LIST_ITEM* _al_list_insert_before_ex(_AL_LIST* list, _AL_LIST_ITEM* where, void* data, _AL_LIST_ITEM_DTOR dtor)
{
_AL_LIST_ITEM* item;

ASSERT(list == where->list);

item = list_create_item(list);
if (NULL == item)
return NULL;

item->data = data;
item->dtor = dtor;

item->next = where;
item->prev = where->prev;

where->prev->next = item;
where->prev = item;

list->size++;

return item;
}





void _al_list_erase(_AL_LIST* list, _AL_LIST_ITEM* item)
{
if (NULL == item)
return;

ASSERT(list == item->list);

item->prev->next = item->next;
item->next->prev = item->prev;

list->size--;

list_destroy_item(list, item);
}





void _al_list_clear(_AL_LIST* list)
{
_AL_LIST_ITEM* item;
_AL_LIST_ITEM* next;

item = _al_list_front(list);

while (NULL != item) {

next = _al_list_next(list, item);

_al_list_erase(list, item);

item = next;
}
}





void _al_list_remove(_AL_LIST* list, void* data)
{
_AL_LIST_ITEM* item = NULL;
_AL_LIST_ITEM* next = NULL;

item = _al_list_find_first(list, data);

while (NULL != item) {

next = _al_list_find_after(list, item, data);

_al_list_erase(list, item);

item = next;
}
}





bool _al_list_is_empty(_AL_LIST* list)
{
return 0 == list->size;
}





bool _al_list_contains(_AL_LIST* list, void* data)
{
return NULL != _al_list_find_first(list, data);
}





_AL_LIST_ITEM* _al_list_find_first(_AL_LIST* list, void* data)
{
return _al_list_find_after(list, list->root, data);
}





_AL_LIST_ITEM* _al_list_find_last(_AL_LIST* list, void* data)
{
return _al_list_find_before(list, list->root, data);
}





_AL_LIST_ITEM* _al_list_find_after(_AL_LIST* list, _AL_LIST_ITEM* where, void* data)
{
_AL_LIST_ITEM* item;

ASSERT(list == where->list);

for (item = where->next; item != list->root; item = item->next)
if (item->data == data)
return item;

return NULL;
}





_AL_LIST_ITEM* _al_list_find_before(_AL_LIST* list, _AL_LIST_ITEM* where, void* data)
{
_AL_LIST_ITEM* item;

ASSERT(list == where->list);

for (item = where->prev; item != list->root; item = item->prev)
if (item->data == data)
return item;

return NULL;
}





size_t _al_list_size(_AL_LIST* list)
{
return list->size;
}





_AL_LIST_ITEM* _al_list_at(_AL_LIST* list, size_t index)
{
if (index >= list->size)
return NULL;

if (index < list->size / 2) {

_AL_LIST_ITEM* item = list->root->next;

while (index--)
item = item->next;

return item;
}
else {

_AL_LIST_ITEM* item = list->root->prev;

index = list->size - index;

while (index--)
item = item->prev;

return item;
}
}





_AL_LIST_ITEM* _al_list_front(_AL_LIST* list)
{
if (list->size > 0)
return list->root->next;
else
return NULL;
}





_AL_LIST_ITEM* _al_list_back(_AL_LIST* list)
{
if (list->size > 0)
return list->root->prev;
else
return NULL;
}





_AL_LIST_ITEM* _al_list_next(_AL_LIST* list, _AL_LIST_ITEM* item)
{
ASSERT(list == item->list);
(void)list;

if (item->next != item->list->root)
return item->next;
else
return NULL;
}





_AL_LIST_ITEM* _al_list_previous(_AL_LIST* list, _AL_LIST_ITEM* item)
{
ASSERT(list == item->list);
(void)list;

if (item->prev != item->list->root)
return item->prev;
else
return NULL;
}






_AL_LIST_ITEM* _al_list_next_circular(_AL_LIST* list, _AL_LIST_ITEM* item)
{
ASSERT(list == item->list);

if (item->next != item->list->root)
return item->next;
else
return list->root->next;
}






_AL_LIST_ITEM* _al_list_previous_circular(_AL_LIST* list, _AL_LIST_ITEM* item)
{
ASSERT(list == item->list);

if (item->prev != item->list->root)
return item->prev;
else
return list->root->prev;
}





void* _al_list_item_data(_AL_LIST_ITEM* item)
{
return item->data;
}





void _al_list_item_set_dtor(_AL_LIST_ITEM* item, _AL_LIST_ITEM_DTOR dtor)
{
item->dtor = dtor;
}





_AL_LIST_ITEM_DTOR _al_list_item_get_dtor(_AL_LIST_ITEM* item)
{
return item->dtor;
}





void _al_list_set_user_data(_AL_LIST* list, void* user_data)
{
list->user_data = user_data;
}





void* _al_list_get_user_data(_AL_LIST* list)
{
return list->user_data;
}
