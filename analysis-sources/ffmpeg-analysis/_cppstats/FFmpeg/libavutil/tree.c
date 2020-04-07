#include "error.h"
#include "log.h"
#include "mem.h"
#include "tree.h"
typedef struct AVTreeNode {
struct AVTreeNode *child[2];
void *elem;
int state;
} AVTreeNode;
const int av_tree_node_size = sizeof(AVTreeNode);
struct AVTreeNode *av_tree_node_alloc(void)
{
return av_mallocz(sizeof(struct AVTreeNode));
}
void *av_tree_find(const AVTreeNode *t, void *key,
int (*cmp)(const void *key, const void *b), void *next[2])
{
if (t) {
unsigned int v = cmp(key, t->elem);
if (v) {
if (next)
next[v >> 31] = t->elem;
return av_tree_find(t->child[(v >> 31) ^ 1], key, cmp, next);
} else {
if (next) {
av_tree_find(t->child[0], key, cmp, next);
av_tree_find(t->child[1], key, cmp, next);
}
return t->elem;
}
}
return NULL;
}
void *av_tree_insert(AVTreeNode **tp, void *key,
int (*cmp)(const void *key, const void *b), AVTreeNode **next)
{
AVTreeNode *t = *tp;
if (t) {
unsigned int v = cmp(t->elem, key);
void *ret;
if (!v) {
if (*next)
return t->elem;
else if (t->child[0] || t->child[1]) {
int i = !t->child[0];
void *next_elem[2];
av_tree_find(t->child[i], key, cmp, next_elem);
key = t->elem = next_elem[i];
v = -i;
} else {
*next = t;
*tp = NULL;
return NULL;
}
}
ret = av_tree_insert(&t->child[v >> 31], key, cmp, next);
if (!ret) {
int i = (v >> 31) ^ !!*next;
AVTreeNode **child = &t->child[i];
t->state += 2 * i - 1;
if (!(t->state & 1)) {
if (t->state) {
if ((*child)->state * 2 == -t->state) {
*tp = (*child)->child[i ^ 1];
(*child)->child[i ^ 1] = (*tp)->child[i];
(*tp)->child[i] = *child;
*child = (*tp)->child[i ^ 1];
(*tp)->child[i ^ 1] = t;
(*tp)->child[0]->state = -((*tp)->state > 0);
(*tp)->child[1]->state = (*tp)->state < 0;
(*tp)->state = 0;
} else {
*tp = *child;
*child = (*child)->child[i ^ 1];
(*tp)->child[i ^ 1] = t;
if ((*tp)->state)
t->state = 0;
else
t->state >>= 1;
(*tp)->state = -t->state;
}
}
}
if (!(*tp)->state ^ !!*next)
return key;
}
return ret;
} else {
*tp = *next;
*next = NULL;
if (*tp) {
(*tp)->elem = key;
return NULL;
} else
return key;
}
}
void av_tree_destroy(AVTreeNode *t)
{
if (t) {
av_tree_destroy(t->child[0]);
av_tree_destroy(t->child[1]);
av_free(t);
}
}
void av_tree_enumerate(AVTreeNode *t, void *opaque,
int (*cmp)(void *opaque, void *elem),
int (*enu)(void *opaque, void *elem))
{
if (t) {
int v = cmp ? cmp(opaque, t->elem) : 0;
if (v >= 0)
av_tree_enumerate(t->child[0], opaque, cmp, enu);
if (v == 0)
enu(opaque, t->elem);
if (v <= 0)
av_tree_enumerate(t->child[1], opaque, cmp, enu);
}
}
