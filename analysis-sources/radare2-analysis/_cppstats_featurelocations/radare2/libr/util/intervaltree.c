

#include <r_util/r_intervaltree.h>
#include <r_util/r_assert.h>

#define unwrap(rbnode) container_of (rbnode, RIntervalNode, node)

static void node_max(RBNode *node) {
RIntervalNode *intervalnode = unwrap (node);
intervalnode->max_end = intervalnode->end;
int i;
for (i = 0; i < 2; i++) {
if (node->child[i]) {
ut64 end = unwrap (node->child[i])->max_end;
if (end > intervalnode->max_end) {
intervalnode->max_end = end;
}
}
}
}

static int cmp(const void *incoming, const RBNode *in_tree, void *user) {
ut64 incoming_start = *(ut64 *)incoming;
ut64 other_start = container_of (in_tree, const RIntervalNode, node)->start;
if (incoming_start < other_start) {
return -1;
}
if (incoming_start > other_start) {
return 1;
}
return 0;
}


static int cmp_exact_node(const void *incoming, const RBNode *in_tree, void *user) {
RIntervalNode *incoming_node = (RIntervalNode *)incoming;
const RIntervalNode *node = container_of (in_tree, const RIntervalNode, node);
if (node == incoming_node) {
return 0;
}
if (incoming_node->start < node->start) {
return -1;
}
if (incoming_node->start > node->start) {
return 1;
}



RBIter *path_cache = user;
if (!path_cache->len) {
RBNode *cur = (RBNode *)&node->node;

while (cur) {
path_cache->path[path_cache->len++] = cur;
if (incoming_node->start <= unwrap (cur)->start) {
cur = cur->child[0];
} else {
cur = cur->child[1];
}
}



while (r_rbtree_iter_has (path_cache)) {
RIntervalNode *intervalnode = r_rbtree_iter_get (path_cache, RIntervalNode, node);
if (intervalnode == incoming_node || intervalnode->start > incoming_node->start) {
break;
}

RBNode *rbnode = &intervalnode->node;
if (rbnode->child[1]) {

for (rbnode = rbnode->child[1]; rbnode; rbnode = rbnode->child[0]) {
path_cache->path[path_cache->len++] = rbnode;
}
} else {

do {
rbnode = path_cache->path[--path_cache->len];
} while (path_cache->len && path_cache->path[path_cache->len - 1]->child[1] == rbnode);
}
}
}

RBNode *next_child = NULL;

size_t i;
for (i = 0; i < path_cache->len - 1; i++) {
if (unwrap (path_cache->path[i]) == node) {
next_child = path_cache->path[i + 1];
break;
}
}


return (next_child && node->node.child[0] == next_child) ? -1 : 1;
}

R_API void r_interval_tree_init(RIntervalTree *tree, RIntervalNodeFree free) {
tree->root = NULL;
tree->free = free;
}

static void interval_node_free(RBNode *node, void *user) {
RIntervalNode *ragenode = unwrap (node);
if (user) {
((RContRBFree)user) (ragenode->data);
}
free (ragenode);
}

R_API void r_interval_tree_fini(RIntervalTree *tree) {
if (!tree || !tree->root) {
return;
}
r_rbtree_free (&tree->root->node, interval_node_free, tree->free);
}

R_API bool r_interval_tree_insert(RIntervalTree *tree, ut64 start, ut64 end, void *data) {
r_return_val_if_fail (end >= start, false);
RIntervalNode *node = R_NEW0 (RIntervalNode);
if (!node) {
return false;
}
node->start = start;
node->end = end;
node->data = data;
RBNode *root = tree->root ? &tree->root->node : NULL;
bool r = r_rbtree_aug_insert (&root, &start, &node->node, cmp, NULL, node_max);
tree->root = unwrap (root);
if (!r) {
free (node);
}
return r;
}

R_API bool r_interval_tree_delete(RIntervalTree *tree, RIntervalNode *node, bool free) {
RBNode *root = &tree->root->node;
RBIter path_cache = { 0 };
bool r = r_rbtree_aug_delete (&root, node, cmp_exact_node, &path_cache, interval_node_free, free ? tree->free : NULL, node_max);
tree->root = root ? unwrap (root) : NULL;
return r;
}

R_API bool r_interval_tree_resize(RIntervalTree *tree, RIntervalNode *node, ut64 new_start, ut64 new_end) {
r_return_val_if_fail (new_end >= new_start, false);
if (node->start != new_start) {

void *data = node->data;
if (!r_interval_tree_delete (tree, node, false)) {
return false;
}
return r_interval_tree_insert (tree, new_start, new_end, data);
}
if (node->end != new_end) {

node->end = new_end;
RBIter path_cache = { 0 };
return r_rbtree_aug_update_sum (&tree->root->node, node, &node->node, cmp_exact_node, &path_cache, node_max);
}

return true;
}



R_API RIntervalNode *r_interval_tree_node_at(RIntervalTree *tree, ut64 start) {
RIntervalNode *node = tree->root;
while (node) {
if (start < node->start) {
node = unwrap (node->node.child[0]);
} else if (start > node->start) {
node = unwrap (node->node.child[1]);
} else {
return node;
}
}
return NULL;
}

R_API RBIter r_interval_tree_first_at(RIntervalTree *tree, ut64 start) {
RBIter it = { 0 };


RIntervalNode *top_intervalnode = r_interval_tree_node_at (tree, start);
if (!top_intervalnode) {
return it;
}


RBNode *node = &top_intervalnode->node;
while (node) {
if (start <= unwrap (node)->start) {
it.path[it.len++] = node;
node = node->child[0];
} else {
node = node->child[1];
}
}

return it;
}

R_API RIntervalNode *r_interval_tree_node_at_data(RIntervalTree *tree, ut64 start, void *data) {
RBIter it = r_interval_tree_first_at (tree, start);
while (r_rbtree_iter_has (&it)) {
RIntervalNode *intervalnode = r_rbtree_iter_get (&it, RIntervalNode, node);
if (intervalnode->start != start) {
break;
}
if (intervalnode->data == data) {
return intervalnode;
}
r_rbtree_iter_next (&it);
}
return NULL;
}

R_API void r_interval_tree_all_at(RIntervalTree *tree, ut64 start, RIntervalIterCb cb, void *user) {
RBIter it = r_interval_tree_first_at (tree, start);
while (r_rbtree_iter_has (&it)) {
RIntervalNode *intervalnode = r_rbtree_iter_get (&it, RIntervalNode, node);
if (intervalnode->start != start) {
break;
}
cb (intervalnode, user);
r_rbtree_iter_next (&it);
}
}

R_API void r_interval_node_all_in(RIntervalNode *node, ut64 value, bool end_inclusive, RIntervalIterCb cb, void *user) {
while (node && value < node->start) {

node = unwrap (node->node.child[0]);
}
if (!node) {
return;
}
if (end_inclusive ? value > node->max_end : value >= node->max_end) {
return;
}
if (end_inclusive ? value <= node->end : value < node->end) {
cb (node, user);
}

r_interval_node_all_in (unwrap (node->node.child[0]), value, end_inclusive, cb, user);
r_interval_node_all_in (unwrap (node->node.child[1]), value, end_inclusive, cb, user);
}

R_API void r_interval_tree_all_in(RIntervalTree *tree, ut64 value, bool end_inclusive, RIntervalIterCb cb, void *user) {

r_interval_node_all_in (tree->root, value, end_inclusive, cb, user);
}

static void r_interval_node_all_intersect(RIntervalNode *node, ut64 start, ut64 end, bool end_inclusive, RIntervalIterCb cb, void *user) {
r_return_if_fail (end >= start);
while (node && (end_inclusive ? end < node->start : end <= node->start)) {

node = unwrap (node->node.child[0]);
}
if (!node) {
return;
}
if (end_inclusive ? start > node->max_end : start >= node->max_end) {
return;
}
if (end_inclusive ? start <= node->end : start < node->end) {
cb (node, user);
}

r_interval_node_all_intersect (unwrap (node->node.child[0]), start, end, end_inclusive, cb, user);
r_interval_node_all_intersect (unwrap (node->node.child[1]), start, end, end_inclusive, cb, user);
}

R_API void r_interval_tree_all_intersect(RIntervalTree *tree, ut64 start, ut64 end, bool end_inclusive, RIntervalIterCb cb, void *user) {
r_interval_node_all_intersect (tree->root, start, end, end_inclusive, cb, user);
}
