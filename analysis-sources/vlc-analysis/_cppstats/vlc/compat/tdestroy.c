#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <stdlib.h>
#if defined(HAVE_SEARCH_H)
#include <search.h>
#endif
#if defined(HAVE_TFIND)
static __thread struct
{
const void **tab;
size_t count;
} list = { NULL, 0 };
static void list_nodes(const void *node, const VISIT which, const int depth)
{
(void) depth;
if (which != postorder && which != leaf)
return;
const void **tab = realloc(list.tab, sizeof (*tab) * (list.count + 1));
if (tab == NULL)
abort();
tab[list.count] = *(const void **)node;
list.tab = tab;
list.count++;
}
static __thread const void *smallest;
static int cmp_smallest(const void *a, const void *b)
{
if (a == b)
return 0;
if (a == smallest)
return -1;
if (b == smallest)
return +1;
abort();
}
void tdestroy(void *root, void (*freenode)(void *))
{
const void **tab;
size_t count;
assert(freenode != NULL);
assert(list.count == 0);
twalk(root, list_nodes);
tab = list.tab;
count = list.count;
list.tab = NULL;
list.count = 0;
for (size_t i = 0; i < count; i++)
{
void *node = (void *)(tab[i]);
smallest = node;
node = tdelete(node, &root, cmp_smallest);
assert(node != NULL);
}
assert (root == NULL);
for (size_t i = 0; i < count; i++)
freenode((void *)(tab[i]));
free(tab);
}
#endif 
