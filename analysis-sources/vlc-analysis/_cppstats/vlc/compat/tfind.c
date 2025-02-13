#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#if !defined(HAVE_SEARCH_H)
#include <assert.h>
#include <stdlib.h>
typedef struct node {
char *key;
struct node *llink, *rlink;
} node_t;
void *
tdelete(vkey, vrootp, compar)
const void *vkey; 
void **vrootp; 
int (*compar) (const void *, const void *);
{
node_t **rootp = (node_t **)vrootp;
node_t *p, *q, *r;
int cmp;
assert(vkey != NULL);
assert(compar != NULL);
if (rootp == NULL || (p = *rootp) == NULL)
return NULL;
while ((cmp = (*compar)(vkey, (*rootp)->key)) != 0) {
p = *rootp;
rootp = (cmp < 0) ?
&(*rootp)->llink : 
&(*rootp)->rlink; 
if (*rootp == NULL)
return NULL; 
}
r = (*rootp)->rlink; 
if ((q = (*rootp)->llink) == NULL) 
q = r;
else if (r != NULL) { 
if (r->llink == NULL) { 
r->llink = q;
q = r;
} else { 
for (q = r->llink; q->llink != NULL; q = r->llink)
r = q;
r->llink = q->rlink;
q->llink = (*rootp)->llink;
q->rlink = (*rootp)->rlink;
}
}
if (p != *rootp)
free(*rootp); 
*rootp = q; 
return p;
}
static void
tdestroy_recurse(node_t* root, void (*free_action)(void *))
{
if (root->llink != NULL)
tdestroy_recurse(root->llink, free_action);
if (root->rlink != NULL)
tdestroy_recurse(root->rlink, free_action);
(*free_action) ((void *) root->key);
free(root);
}
void
tdestroy(vrootp, freefct)
void *vrootp;
void (*freefct)(void *);
{
node_t *root = (node_t *) vrootp;
if (root != NULL)
tdestroy_recurse(root, freefct);
}
void *
tfind(vkey, vrootp, compar)
const void *vkey; 
const void **vrootp; 
int (*compar) (const void *, const void *);
{
node_t * const *rootp = (node_t * const*)vrootp;
assert(vkey != NULL);
assert(compar != NULL);
if (rootp == NULL)
return NULL;
while (*rootp != NULL) { 
int r;
if ((r = (*compar)(vkey, (*rootp)->key)) == 0) 
return *rootp; 
rootp = (r < 0) ?
&(*rootp)->llink : 
&(*rootp)->rlink; 
}
return NULL;
}
void *
tsearch(vkey, vrootp, compar)
const void *vkey; 
void **vrootp; 
int (*compar) (const void *, const void *);
{
node_t *q;
node_t **rootp = (node_t **)vrootp;
assert(vkey != NULL);
assert(compar != NULL);
if (rootp == NULL)
return NULL;
while (*rootp != NULL) { 
int r;
if ((r = (*compar)(vkey, (*rootp)->key)) == 0) 
return *rootp; 
rootp = (r < 0) ?
&(*rootp)->llink : 
&(*rootp)->rlink; 
}
q = malloc(sizeof(node_t)); 
if (q != 0) { 
*rootp = q; 
q->key = (void*)vkey; 
q->llink = q->rlink = NULL;
}
return q;
}
static void
twalk_recurse(root, action, level)
const node_t *root; 
void (*action) (const void *, VISIT, int);
int level;
{
assert(root != NULL);
assert(action != NULL);
if (root->llink == NULL && root->rlink == NULL)
(*action)(root, leaf, level);
else {
(*action)(root, preorder, level);
if (root->llink != NULL)
twalk_recurse(root->llink, action, level + 1);
(*action)(root, postorder, level);
if (root->rlink != NULL)
twalk_recurse(root->rlink, action, level + 1);
(*action)(root, endorder, level);
}
}
void
twalk(vroot, action)
const void *vroot; 
void (*action) (const void *, VISIT, int);
{
if (vroot != NULL && action != NULL)
twalk_recurse(vroot, action, 0);
}
#endif 
