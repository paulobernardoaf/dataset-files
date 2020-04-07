#define _AL_AATREE Aatree
#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_aatree.h"
struct DelInfo
{
const void *key;
_al_cmp_t compare;
Aatree *last;
Aatree *deleted;
};
static Aatree nil = { 0, &nil, &nil, NULL, NULL };
static Aatree *skew(Aatree *T)
{
if (T == &nil)
return T;
if (T->left->level == T->level) {
Aatree *L = T->left;
T->left = L->right;
L->right = T;
return L;
}
return T;
}
static Aatree *split(Aatree *T)
{
if (T == &nil)
return T;
if (T->level == T->right->right->level) {
Aatree *R = T->right;
T->right = R->left;
R->left = T;
R->level = R->level + 1;
return R;
}
return T;
}
static Aatree *singleton(const void *key, void *value)
{
Aatree *T = al_malloc(sizeof(Aatree));
T->level = 1;
T->left = &nil;
T->right = &nil;
T->key = key;
T->value = value;
return T;
}
static Aatree *doinsert(Aatree *T, const void *key, void *value,
_al_cmp_t compare)
{
int cmp;
if (T == &nil) {
return singleton(key, value);
}
cmp = compare(key, T->key);
if (cmp < 0) {
T->left = doinsert(T->left, key, value, compare);
}
else if (cmp > 0) {
T->right = doinsert(T->right, key, value, compare);
}
else {
return T;
}
T = skew(T);
T = split(T);
return T;
}
Aatree *_al_aa_insert(Aatree *T, const void *key, void *value,
_al_cmp_t compare)
{
if (T == NULL)
T = &nil;
return doinsert(T, key, value, compare);
}
void *_al_aa_search(const Aatree *T, const void *key, _al_cmp_t compare)
{
if (T == NULL)
return NULL;
while (T != &nil) {
int cmp = compare(key, T->key);
if (cmp == 0)
return T->value;
T = (cmp < 0) ? T->left : T->right;
}
return NULL;
}
static Aatree *dodelete(struct DelInfo *info, Aatree *T, void **ret_value)
{
if (T == &nil)
return T;
info->last = T;
if (info->compare(info->key, T->key) < 0) {
T->left = dodelete(info, T->left, ret_value);
}
else {
info->deleted = T;
T->right = dodelete(info, T->right, ret_value);
}
if (T == info->last &&
info->deleted != &nil &&
info->compare(info->key, info->deleted->key) == 0)
{
Aatree *right = T->right;
*ret_value = info->deleted->value;
info->deleted->key = T->key;
info->deleted->value = T->value;
info->deleted = &nil;
al_free(T);
return right;
}
if (T->left->level < T->level - 1
|| T->right->level < T->level - 1)
{
T->level--;
if (T->right->level > T->level) {
T->right->level = T->level;
}
T = skew(T);
T->right = skew(T->right);
T->right->right = skew(T->right->right);
T = split(T);
T->right = split(T->right);
}
return T;
}
Aatree *_al_aa_delete(Aatree *T, const void *key, _al_cmp_t compare,
void **ret_value)
{
struct DelInfo info;
info.key = key;
info.compare = compare;
info.last = &nil;
info.deleted = &nil;
if (T) {
T = dodelete(&info, T, ret_value);
if (T == &nil)
T = NULL;
}
return T;
}
void _al_aa_free(Aatree *T)
{
if (T && T != &nil) {
_al_aa_free(T->left);
_al_aa_free(T->right);
al_free(T);
}
}
