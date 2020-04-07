#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include "r_list.h"
#if defined(__cplusplus)
extern "C" {
#endif
#define R_RBTREE_MAX_HEIGHT 62
typedef struct r_rb_node_t {
struct r_rb_node_t *child[2];
bool red;
} RBNode;
typedef RBNode* RBTree;
typedef int (*RBComparator)(const void *incoming, const RBNode *in_tree, void *user);
typedef void (*RBNodeFree)(RBNode *node, void *user);
typedef void (*RBNodeSum)(RBNode *node);
typedef struct r_rb_iter_t {
int len;
RBNode *path[R_RBTREE_MAX_HEIGHT];
} RBIter;
typedef int (*RContRBCmp)(void *incoming, void *in, void *user);
typedef void (*RContRBFree)(void *);
typedef struct r_containing_rb_node_t {
RBNode node;
void *data;
} RContRBNode;
typedef struct r_containing_rb_tree_t {
RContRBNode *root;
RContRBFree free;
} RContRBTree;
R_API bool r_rbtree_aug_delete(RBNode **root, void *data, RBComparator cmp, void *cmp_user, RBNodeFree freefn, void *free_user, RBNodeSum sum);
R_API bool r_rbtree_aug_insert(RBNode **root, void *data, RBNode *node, RBComparator cmp, void *cmp_user, RBNodeSum sum);
R_API bool r_rbtree_aug_update_sum(RBNode *root, void *data, RBNode *node, RBComparator cmp, void *cmp_user, RBNodeSum sum);
R_API bool r_rbtree_delete(RBNode **root, void *data, RBComparator cmp, void *cmp_user, RBNodeFree freefn, void *free_user);
R_API RBNode *r_rbtree_find(RBNode *root, void *data, RBComparator cmp, void *user);
R_API void r_rbtree_free(RBNode *root, RBNodeFree freefn, void *user);
R_API void r_rbtree_insert(RBNode **root, void *data, RBNode *node, RBComparator cmp, void *user);
R_API RBNode *r_rbtree_lower_bound(RBNode *root, void *data, RBComparator cmp, void *user);
R_API RBNode *r_rbtree_upper_bound(RBNode *root, void *data, RBComparator cmp, void *user);
R_API RBIter r_rbtree_first(RBNode *root);
R_API RBIter r_rbtree_last(RBNode *root);
R_API RBIter r_rbtree_lower_bound_forward(RBNode *root, void *data, RBComparator cmp, void *user);
R_API RBIter r_rbtree_upper_bound_backward(RBNode *root, void *data, RBComparator cmp, void *user);
#define r_rbtree_iter_get(it, struc, rb) (container_of ((it)->path[(it)->len-1], struc, rb))
#define r_rbtree_iter_has(it) ((it)->len)
R_API void r_rbtree_iter_next(RBIter *it);
R_API void r_rbtree_iter_prev(RBIter *it);
#define r_rbtree_iter_while(it, data, struc, rb) for (; r_rbtree_iter_has(&it) && (data = r_rbtree_iter_get (&it, struc, rb)); r_rbtree_iter_next (&(it)))
#define r_rbtree_iter_while_prev(it, data, struc, rb) for (; r_rbtree_iter_has(&it) && (data = r_rbtree_iter_get (&it, struc, rb)); r_rbtree_iter_prev (&(it)))
#define r_rbtree_foreach(root, it, data, struc, rb) for ((it) = r_rbtree_first (root); r_rbtree_iter_has(&it) && (data = r_rbtree_iter_get (&it, struc, rb)); r_rbtree_iter_next (&(it)))
#define r_rbtree_foreach_prev(root, it, data, struc, rb) for ((it) = r_rbtree_last (root); r_rbtree_iter_has(&it) && (data = r_rbtree_iter_get (&it, struc, rb)); r_rbtree_iter_prev (&(it)))
R_API RContRBTree *r_rbtree_cont_new(void);
R_API RContRBTree *r_rbtree_cont_newf(RContRBFree f);
R_API bool r_rbtree_cont_insert(RContRBTree *tree, void *data, RContRBCmp cmp, void *user);
R_API bool r_rbtree_cont_delete(RContRBTree *tree, void *data, RContRBCmp cmp, void *user);
R_API void *r_rbtree_cont_find(RContRBTree *tree, void *data, RContRBCmp cmp, void *user);
#define r_rbtree_cont_foreach(tree, it, dat) for ((it) = r_rbtree_first (&tree->root->node); r_rbtree_iter_has(&it) && (dat = r_rbtree_iter_get (&it, RContRBNode, node)->data); r_rbtree_iter_next (&(it)))
#define r_rbtree_cont_foreach_prev(tree, it, dat) for ((it) = r_rbtree_last (&tree->root->node); r_rbtree_iter_has(&it) && (dat = r_rbtree_iter_get (&it, RContRBNode, node)->data); r_rbtree_iter_prev (&(it)))
R_API void r_rbtree_cont_free(RContRBTree *tree);
#if defined(__cplusplus)
}
#endif
