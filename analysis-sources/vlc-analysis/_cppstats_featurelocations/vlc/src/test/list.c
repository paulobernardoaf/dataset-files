



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#undef NDEBUG
#include <assert.h>

#include <vlc_common.h>
#include <vlc_list.h>

struct test_elem
{
int i;
struct vlc_list node;
};

static struct vlc_list *make_elem(int i)
{
struct test_elem *e = malloc(sizeof (*e));
if (e == NULL)
abort();

e->i = i;
return &e->node;
}

int main (void)
{
struct vlc_list head, *back;
struct test_elem *elem;
int count;

vlc_list_init(&head);
vlc_list_foreach(elem, &head, node)
assert(0); 
assert(vlc_list_is_empty(&head));

vlc_list_init(&head); 
vlc_list_prepend(make_elem(1), &head);
count = 0;
vlc_list_foreach(elem, &head, node)
assert(elem->i == 1), count++;
assert(count == 1);

back = make_elem(2);
vlc_list_append(back, &head);
count = 0;
vlc_list_foreach(elem, &head, node)
assert(elem->i == count + 1), count++;
assert(count == 2);

vlc_list_prepend(make_elem(3), &head);
vlc_list_remove(head.prev); 
free(vlc_list_entry(back, struct test_elem, node));
count = 0;
vlc_list_foreach(elem, &head, node)
assert(elem->i == (count ? 1 : 3)), count++;
assert(count == 2);

vlc_list_foreach(elem, &head, node)
{
vlc_list_remove(&elem->node);
free(elem);
}
assert(vlc_list_is_empty(&head));

for (int i = 20; i < 30; i++)
vlc_list_append(make_elem(i), &head);
for (int i = 19; i >= 10; i--)
vlc_list_prepend(make_elem(i), &head);

count = 0;
vlc_list_foreach(elem, &head, node)
assert(elem->i == count + 10), count++;
assert(count == 20);

count = 0;
for (elem = vlc_list_first_entry_or_null(&head, struct test_elem, node);
elem != NULL;
elem = vlc_list_next_entry_or_null(&head, elem,
struct test_elem, node))
assert(elem->i == count + 10), count++;
assert(count == 20);

count = 0;
for (elem = vlc_list_last_entry_or_null(&head, struct test_elem, node);
elem != NULL;
elem = vlc_list_prev_entry_or_null(&head, elem,
struct test_elem, node))
assert(elem->i == 29 - count), count++;
assert(count == 20);

count = 0;
vlc_list_foreach(elem, &head, node)
{
if (count & 1)
{
vlc_list_remove(&elem->node);
free(elem);
}
count++;
}
assert(count == 20);

count = 0;
vlc_list_foreach(elem, &head, node)
{
assert(elem->i == count * 2 + 10);
vlc_list_remove(&elem->node);
free(elem);
count++;
}
assert(count == 10);
assert(vlc_list_is_empty(&head));
return 0;
}
