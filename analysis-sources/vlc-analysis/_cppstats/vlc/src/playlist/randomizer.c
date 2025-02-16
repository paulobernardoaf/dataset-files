#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#if defined(TEST_RANDOMIZER)
#undef NDEBUG
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_rand.h>
#include "randomizer.h"
#define NOT_SAME_BEFORE 1
void
randomizer_Init(struct randomizer *r)
{
vlc_vector_init(&r->items);
vlc_rand_bytes(r->xsubi, sizeof(r->xsubi));
r->loop = false;
r->head = 0;
r->next = 0;
r->history = 0;
}
void
randomizer_Destroy(struct randomizer *r)
{
vlc_vector_destroy(&r->items);
}
void
randomizer_SetLoop(struct randomizer *r, bool loop)
{
r->loop = loop;
}
static inline ssize_t
randomizer_IndexOf(struct randomizer *r, const vlc_playlist_item_t *item)
{
ssize_t index;
vlc_vector_index_of(&r->items, item, &index);
return index;
}
bool
randomizer_Count(struct randomizer *r)
{
return r->items.size;
}
void
randomizer_Reshuffle(struct randomizer *r)
{
r->head = 0;
r->next = 0;
r->history = r->items.size;
}
static inline void
swap_items(struct randomizer *r, int i, int j)
{
vlc_playlist_item_t *item = r->items.data[i];
r->items.data[i] = r->items.data[j];
r->items.data[j] = item;
}
static inline void
randomizer_DetermineOne_(struct randomizer *r, size_t avoid_last_n)
{
assert(r->head < r->items.size);
assert(r->items.size - r->head > avoid_last_n);
size_t range_len = r->items.size - r->head - avoid_last_n;
size_t selected = r->head + (nrand48(r->xsubi) % range_len);
swap_items(r, r->head, selected);
if (r->head == r->history)
r->history++;
r->head++;
}
static inline void
randomizer_DetermineOne(struct randomizer *r)
{
randomizer_DetermineOne_(r, 0);
}
static void
randomizer_AutoReshuffle(struct randomizer *r)
{
assert(r->items.size > 0);
r->head = 0;
r->next = 0;
r->history = 0; 
size_t avoid_last_n = NOT_SAME_BEFORE;
if (avoid_last_n > r->items.size - 1)
avoid_last_n = r->items.size - 1;
while (avoid_last_n)
randomizer_DetermineOne_(r, avoid_last_n--);
}
bool
randomizer_HasPrev(struct randomizer *r)
{
if (!r->loop)
return r->next > 1;
if (!r->items.size)
return false;
return (r->next + r->items.size - r->history) % r->items.size != 1;
}
bool
randomizer_HasNext(struct randomizer *r)
{
return r->loop || r->next < r->items.size;
}
vlc_playlist_item_t *
randomizer_PeekPrev(struct randomizer *r)
{
assert(randomizer_HasPrev(r));
size_t index = (r->next + r->items.size - 2) % r->items.size;
return r->items.data[index];
}
vlc_playlist_item_t *
randomizer_PeekNext(struct randomizer *r)
{
assert(randomizer_HasNext(r));
if (r->next == r->items.size && r->next == r->history)
{
assert(r->loop);
randomizer_AutoReshuffle(r);
}
if (r->next == r->head)
randomizer_DetermineOne(r);
return r->items.data[r->next];
}
vlc_playlist_item_t *
randomizer_Prev(struct randomizer *r)
{
assert(randomizer_HasPrev(r));
vlc_playlist_item_t *item = randomizer_PeekPrev(r);
r->next = r->next ? r->next - 1 : r->items.size - 1;
return item;
}
vlc_playlist_item_t *
randomizer_Next(struct randomizer *r)
{
assert(randomizer_HasNext(r));
vlc_playlist_item_t *item = randomizer_PeekNext(r);
r->next++;
if (r->next == r->items.size && r->next != r->head)
r->next = 0;
return item;
}
bool
randomizer_Add(struct randomizer *r, vlc_playlist_item_t *items[], size_t count)
{
if (!vlc_vector_insert_all(&r->items, r->history, items, count))
return false;
if (r->next > r->history)
r->next += count;
r->history += count;
return true;
}
static void
randomizer_SelectIndex(struct randomizer *r, size_t index)
{
vlc_playlist_item_t *selected = r->items.data[index];
if (r->history && index >= r->history)
{
if (index > r->history)
{
memmove(&r->items.data[r->history + 1],
&r->items.data[r->history],
(index - r->history) * sizeof(selected));
index = r->history;
}
r->history = (r->history + 1) % r->items.size;
}
if (index >= r->head)
{
r->items.data[index] = r->items.data[r->head];
r->items.data[r->head] = selected;
r->head++;
}
else if (index < r->items.size - 1)
{
memmove(&r->items.data[index],
&r->items.data[index + 1],
(r->head - index - 1) * sizeof(selected));
r->items.data[r->head - 1] = selected;
}
r->next = r->head;
}
void
randomizer_Select(struct randomizer *r, const vlc_playlist_item_t *item)
{
ssize_t index = randomizer_IndexOf(r, item);
assert(index != -1); 
randomizer_SelectIndex(r, (size_t) index);
}
static void
randomizer_RemoveAt(struct randomizer *r, size_t index)
{
if (index < r->next)
r->next--;
if (index < r->head)
{
memmove(&r->items.data[index],
&r->items.data[index + 1],
(r->head - index - 1) * sizeof(*r->items.data));
r->head--;
index = r->head; 
}
if (index < r->history)
{
r->items.data[index] = r->items.data[r->history - 1];
index = r->history - 1;
r->history--;
}
if (index < r->items.size - 1)
{
memmove(&r->items.data[index],
&r->items.data[index + 1],
(r->items.size - index - 1) * sizeof(*r->items.data));
}
r->items.size--;
}
static void
randomizer_RemoveOne(struct randomizer *r, const vlc_playlist_item_t *item)
{
ssize_t index = randomizer_IndexOf(r, item);
assert(index >= 0); 
randomizer_RemoveAt(r, index);
}
void
randomizer_Remove(struct randomizer *r, vlc_playlist_item_t *const items[],
size_t count)
{
for (size_t i = 0; i < count; ++i)
randomizer_RemoveOne(r, items[i]);
vlc_vector_autoshrink(&r->items);
}
void
randomizer_Clear(struct randomizer *r)
{
vlc_vector_clear(&r->items);
r->head = 0;
r->next = 0;
r->history = 0;
}
#if !defined(DOC)
#if defined(TEST_RANDOMIZER)
struct vlc_playlist_item {
size_t index;
};
static void
ArrayInit(vlc_playlist_item_t *array[], size_t len)
{
for (size_t i = 0; i < len; ++i)
{
array[i] = malloc(sizeof(*array[i]));
assert(array[i]);
array[i]->index = i;
}
}
static void
ArrayDestroy(vlc_playlist_item_t *array[], size_t len)
{
for (size_t i = 0; i < len; ++i)
free(array[i]);
}
static void
test_all_items_selected_exactly_once(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
bool selected[SIZE] = {0};
for (int i = 0; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
assert(!randomizer_HasNext(&randomizer)); 
for (int i = 0; i < SIZE; ++i)
assert(selected[i]); 
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_all_items_selected_exactly_once_per_cycle(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
randomizer_SetLoop(&randomizer, true);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
for (int cycle = 0; cycle < 4; ++cycle)
{
bool selected[SIZE] = {0};
for (int i = 0; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
assert(randomizer_HasNext(&randomizer)); 
for (int i = 0; i < SIZE; ++i)
assert(selected[i]); 
}
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_all_items_selected_exactly_once_with_additions(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, 75);
assert(ok);
bool selected[SIZE] = {0};
for (int i = 0; i < 50; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
ok = randomizer_Add(&randomizer, &items[75], 25);
assert(ok);
for (int i = 50; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
assert(!randomizer_HasNext(&randomizer)); 
for (int i = 0; i < SIZE; ++i)
assert(selected[i]); 
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_all_items_selected_exactly_once_with_removals(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
bool selected[SIZE] = {0};
for (int i = 0; i < 50; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
vlc_playlist_item_t *to_remove[20];
memcpy(to_remove, &randomizer.items.data[20], 10 * sizeof(*to_remove));
memcpy(&to_remove[10], &randomizer.items.data[70], 10 * sizeof(*to_remove));
randomizer_Remove(&randomizer, to_remove, 20);
for (int i = 50; i < SIZE - 10; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
assert(!randomizer_HasNext(&randomizer)); 
int count = 0;
for (int i = 0; i < SIZE; ++i)
if (selected[i])
count++;
assert(count == SIZE - 10);
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_cycle_after_manual_selection(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
randomizer_SetLoop(&randomizer, true);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, 100);
assert(ok);
randomizer_Select(&randomizer, randomizer.items.data[0]);
for (int i = 0; i < 2 * SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
}
assert(randomizer_HasNext(&randomizer)); 
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_cycle_with_additions_and_removals(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
randomizer_SetLoop(&randomizer, true);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, 80);
assert(ok);
for (int i = 0; i < 30; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
}
vlc_playlist_item_t *to_remove[20];
memcpy(to_remove, &randomizer.items.data[15], 10 * sizeof(*to_remove));
memcpy(&to_remove[10], &randomizer.items.data[60], 10 * sizeof(*to_remove));
randomizer_Remove(&randomizer, to_remove, 20);
for (int i = 0; i < 40; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
}
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_PeekNext(&randomizer);
assert(item);
assert(randomizer.items.size == 60);
assert(randomizer.history == 1);
vlc_playlist_item_t *history[59];
memcpy(history, &randomizer.items.data[1], 59 * sizeof(*history));
ok = randomizer_Add(&randomizer, &items[80], 20);
assert(ok);
assert(randomizer.items.size == 80);
assert(randomizer.history == 21);
for (int i = 0; i < 59; ++i)
assert(history[i] == randomizer.items.data[21 + i]);
memcpy(to_remove, &randomizer.items.data[30], 10 * sizeof(*to_remove));
randomizer_Remove(&randomizer, to_remove, 10);
assert(randomizer.items.size == 70);
assert(randomizer.history == 21);
for (int i = 0; i < 9; ++i)
assert(history[i] == randomizer.items.data[21 + i]);
for (int i = 0; i < 40; ++i)
assert(history[i + 19] == randomizer.items.data[30 + i]);
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_force_select_new_item(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
bool selected[SIZE] = {0};
for (int i = 0; i < SIZE; ++i)
{
vlc_playlist_item_t *item;
if (i != 50)
{
assert(randomizer_HasNext(&randomizer));
item = randomizer_Next(&randomizer);
}
else
{
item = randomizer.items.data[62];
randomizer_Select(&randomizer, item);
assert(randomizer.items.data[randomizer.next - 1] == item);
}
assert(item);
assert(!selected[item->index]); 
selected[item->index] = true;
}
assert(!randomizer_HasNext(&randomizer)); 
for (int i = 0; i < SIZE; ++i)
assert(selected[i]); 
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
}
static void
test_force_select_item_already_selected(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 100
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
bool selected[SIZE] = {0};
for (int i = 0; i < SIZE + 1; ++i)
{
vlc_playlist_item_t *item;
if (i != 50)
{
assert(randomizer_HasNext(&randomizer));
item = randomizer_Next(&randomizer);
}
else
{
item = randomizer.items.data[42];
randomizer_Select(&randomizer, item);
assert(randomizer.items.data[randomizer.next - 1] == item);
}
assert(item);
assert((i != 50) ^ selected[item->index]);
selected[item->index] = true;
}
assert(!randomizer_HasNext(&randomizer)); 
for (int i = 0; i < SIZE; ++i)
assert(selected[i]); 
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_prev(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 10
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
assert(!randomizer_HasPrev(&randomizer));
vlc_playlist_item_t *actual[SIZE];
for (int i = 0; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
actual[i] = randomizer_Next(&randomizer);
assert(actual[i]);
}
assert(!randomizer_HasNext(&randomizer));
for (int i = SIZE - 2; i >= 0; --i)
{
assert(randomizer_HasPrev(&randomizer));
vlc_playlist_item_t *item = randomizer_Prev(&randomizer);
assert(item == actual[i]);
}
assert(!randomizer_HasPrev(&randomizer));
for (int i = 1; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item == actual[i]);
}
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_prev_with_select(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 10
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
assert(!randomizer_HasPrev(&randomizer));
vlc_playlist_item_t *actual[SIZE];
for (int i = 0; i < 5; ++i)
{
assert(randomizer_HasNext(&randomizer));
actual[i] = randomizer_Next(&randomizer);
assert(actual[i]);
}
randomizer_Select(&randomizer, actual[2]);
vlc_playlist_item_t *item;
assert(randomizer_HasPrev(&randomizer));
item = randomizer_Prev(&randomizer);
assert(item == actual[4]);
assert(randomizer_HasPrev(&randomizer));
item = randomizer_Prev(&randomizer);
assert(item == actual[3]);
assert(randomizer_HasPrev(&randomizer));
item = randomizer_Prev(&randomizer);
assert(item == actual[1]);
assert(randomizer_HasPrev(&randomizer));
item = randomizer_Prev(&randomizer);
assert(item == actual[0]);
assert(!randomizer_HasPrev(&randomizer));
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_prev_across_reshuffle_loops(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
#define SIZE 10
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
assert(!randomizer_HasPrev(&randomizer));
vlc_playlist_item_t *actual[SIZE];
for (int i = 0; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
actual[i] = randomizer_Next(&randomizer);
assert(actual[i]);
}
assert(!randomizer_HasNext(&randomizer));
randomizer_SetLoop(&randomizer, true);
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *actualnew[4];
for (int i = 0; i < 4; ++i)
{
assert(randomizer_HasNext(&randomizer));
actualnew[i] = randomizer_Next(&randomizer);
assert(actualnew[i]);
}
for (int i = 2; i >= 0; --i)
{
assert(randomizer_HasPrev(&randomizer));
actualnew[i] = randomizer_Prev(&randomizer);
assert(actualnew[i]);
}
assert(actualnew[0] == randomizer.items.data[0]);
int index_in_actual = 9;
for (int i = 0; i < 6; ++i)
{
assert(randomizer_HasPrev(&randomizer));
vlc_playlist_item_t *item = randomizer_Prev(&randomizer);
int j;
for (j = 3; j >= 0; --j)
if (item == actualnew[j])
break;
bool in_actualnew = j != 0;
if (in_actualnew)
index_in_actual--;
else
assert(item == actual[index_in_actual]);
}
assert(!randomizer_HasPrev(&randomizer));
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_loop_respect_not_same_before(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
randomizer_SetLoop(&randomizer, true);
#define SIZE (NOT_SAME_BEFORE + 2)
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
vlc_playlist_item_t *actual[SIZE];
for (int i = 0; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
actual[i] = randomizer_Next(&randomizer);
}
for (int cycle = 0; cycle < 20; cycle++)
{
for (int i = 0; i < NOT_SAME_BEFORE; ++i)
{
assert(randomizer_HasNext(&randomizer));
actual[i] = randomizer_Next(&randomizer);
for (int j = (i + SIZE - NOT_SAME_BEFORE) % SIZE;
j != i;
j = (j + 1) % SIZE)
{
assert(actual[i] != actual[j]);
}
}
for (int i = NOT_SAME_BEFORE; i < SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
actual[i] = randomizer_Next(&randomizer);
}
}
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_loop_respect_not_same_before_impossible(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
randomizer_SetLoop(&randomizer, true);
#define SIZE NOT_SAME_BEFORE
vlc_playlist_item_t *items[SIZE];
ArrayInit(items, SIZE);
bool ok = randomizer_Add(&randomizer, items, SIZE);
assert(ok);
for (int i = 0; i < 10 * SIZE; ++i)
{
assert(randomizer_HasNext(&randomizer));
vlc_playlist_item_t *item = randomizer_Next(&randomizer);
assert(item);
}
ArrayDestroy(items, SIZE);
randomizer_Destroy(&randomizer);
#undef SIZE
}
static void
test_has_prev_next_empty(void)
{
struct randomizer randomizer;
randomizer_Init(&randomizer);
assert(!randomizer_HasPrev(&randomizer));
assert(!randomizer_HasNext(&randomizer));
randomizer_SetLoop(&randomizer, true);
assert(!randomizer_HasPrev(&randomizer));
assert(randomizer_HasNext(&randomizer));
randomizer_Destroy(&randomizer);
}
int main(void)
{
test_all_items_selected_exactly_once();
test_all_items_selected_exactly_once_per_cycle();
test_all_items_selected_exactly_once_with_additions();
test_all_items_selected_exactly_once_with_removals();
test_cycle_after_manual_selection();
test_cycle_with_additions_and_removals();
test_force_select_new_item();
test_force_select_item_already_selected();
test_prev();
test_prev_with_select();
test_prev_across_reshuffle_loops();
test_loop_respect_not_same_before();
test_loop_respect_not_same_before_impossible();
test_has_prev_next_empty();
}
#endif
#endif
