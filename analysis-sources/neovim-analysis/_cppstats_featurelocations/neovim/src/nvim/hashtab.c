






















#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/hashtab.h"
#include "nvim/message.h"
#include "nvim/memory.h"


#define PERTURB_SHIFT 5

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "hashtab.c.generated.h"
#endif

char hash_removed;


void hash_init(hashtab_T *ht)
{

memset(ht, 0, sizeof(hashtab_T));
ht->ht_array = ht->ht_smallarray;
ht->ht_mask = HT_INIT_SIZE - 1;
}





void hash_clear(hashtab_T *ht)
{
if (ht->ht_array != ht->ht_smallarray) {
xfree(ht->ht_array);
}
}




void hash_clear_all(hashtab_T *ht, unsigned int off)
{
size_t todo = ht->ht_used;
for (hashitem_T *hi = ht->ht_array; todo > 0; ++hi) {
if (!HASHITEM_EMPTY(hi)) {
xfree(hi->hi_key - off);
todo--;
}
}
hash_clear(ht);
}










hashitem_T *hash_find(const hashtab_T *const ht, const char_u *const key)
{
return hash_lookup(ht, (const char *)key, STRLEN(key), hash_hash(key));
}













hashitem_T *hash_find_len(const hashtab_T *const ht, const char *const key,
const size_t len)
{
return hash_lookup(ht, key, len, hash_hash_len(key, len));
}












hashitem_T *hash_lookup(const hashtab_T *const ht,
const char *const key, const size_t key_len,
const hash_T hash)
{
#if defined(HT_DEBUG)
hash_count_lookup++;
#endif 





hash_T idx = hash & ht->ht_mask;
hashitem_T *hi = &ht->ht_array[idx];

if (hi->hi_key == NULL) {
return hi;
}

hashitem_T *freeitem = NULL;
if (hi->hi_key == HI_KEY_REMOVED) {
freeitem = hi;
} else if ((hi->hi_hash == hash)
&& (STRNCMP(hi->hi_key, key, key_len) == 0)
&& hi->hi_key[key_len] == NUL) {
return hi;
}








for (hash_T perturb = hash;; perturb >>= PERTURB_SHIFT) {
#if defined(HT_DEBUG)

hash_count_perturb++;
#endif 
idx = 5 * idx + perturb + 1;
hi = &ht->ht_array[idx & ht->ht_mask];

if (hi->hi_key == NULL) {
return freeitem == NULL ? hi : freeitem;
}

if ((hi->hi_hash == hash)
&& (hi->hi_key != HI_KEY_REMOVED)
&& (STRNCMP(hi->hi_key, key, key_len) == 0)
&& hi->hi_key[key_len] == NUL) {
return hi;
}

if ((hi->hi_key == HI_KEY_REMOVED) && (freeitem == NULL)) {
freeitem = hi;
}
}
}





void hash_debug_results(void)
{
#if defined(HT_DEBUG)
fprintf(stderr, "\r\n\r\n\r\n\r\n");
fprintf(stderr, "Number of hashtable lookups: %" PRId64 "\r\n",
(int64_t)hash_count_lookup);
fprintf(stderr, "Number of perturb loops: %" PRId64 "\r\n",
(int64_t)hash_count_perturb);
fprintf(stderr, "Percentage of perturb loops: %" PRId64 "%%\r\n",
(int64_t)(hash_count_perturb * 100 / hash_count_lookup));
#endif 
}








int hash_add(hashtab_T *ht, char_u *key)
{
hash_T hash = hash_hash(key);
hashitem_T *hi = hash_lookup(ht, (const char *)key, STRLEN(key), hash);
if (!HASHITEM_EMPTY(hi)) {
internal_error("hash_add()");
return FAIL;
}
hash_add_item(ht, hi, key, hash);
return OK;
}








void hash_add_item(hashtab_T *ht, hashitem_T *hi, char_u *key, hash_T hash)
{
ht->ht_used++;
if (hi->hi_key == NULL) {
ht->ht_filled++;
}
hi->hi_key = key;
hi->hi_hash = hash;


hash_may_resize(ht, 0);
}







void hash_remove(hashtab_T *ht, hashitem_T *hi)
{
ht->ht_used--;
hi->hi_key = HI_KEY_REMOVED;
hash_may_resize(ht, 0);
}





void hash_lock(hashtab_T *ht)
{
ht->ht_locked++;
}





void hash_unlock(hashtab_T *ht)
{
ht->ht_locked--;
hash_may_resize(ht, 0);
}








static void hash_may_resize(hashtab_T *ht, size_t minitems)
{

if (ht->ht_locked > 0) {
return;
}

#if defined(HT_DEBUG)
if (ht->ht_used > ht->ht_filled) {
EMSG("hash_may_resize(): more used than filled");
}

if (ht->ht_filled >= ht->ht_mask + 1) {
EMSG("hash_may_resize(): table completely filled");
}
#endif 

size_t minsize;
if (minitems == 0) {


if ((ht->ht_filled < HT_INIT_SIZE - 1)
&& (ht->ht_array == ht->ht_smallarray)) {
return;
}





size_t oldsize = ht->ht_mask + 1;
if ((ht->ht_filled * 3 < oldsize * 2) && (ht->ht_used > oldsize / 5)) {
return;
}

if (ht->ht_used > 1000) {

minsize = ht->ht_used * 2;
} else {

minsize = ht->ht_used * 4;
}
} else {

if (minitems < ht->ht_used) {

minitems = ht->ht_used;
}

minsize = minitems * 3 / 2;
}

size_t newsize = HT_INIT_SIZE;
while (newsize < minsize) {

newsize <<= 1;

assert(newsize != 0);
}

bool newarray_is_small = newsize == HT_INIT_SIZE;
bool keep_smallarray = newarray_is_small
&& ht->ht_array == ht->ht_smallarray;



hashitem_T temparray[HT_INIT_SIZE];
hashitem_T *oldarray = keep_smallarray
? memcpy(temparray, ht->ht_smallarray, sizeof(temparray))
: ht->ht_array;
hashitem_T *newarray = newarray_is_small
? ht->ht_smallarray
: xmalloc(sizeof(hashitem_T) * newsize);

memset(newarray, 0, sizeof(hashitem_T) * newsize);




hash_T newmask = newsize - 1;
size_t todo = ht->ht_used;

for (hashitem_T *olditem = oldarray; todo > 0; ++olditem) {
if (HASHITEM_EMPTY(olditem)) {
continue;
}



hash_T newi = olditem->hi_hash & newmask;
hashitem_T *newitem = &newarray[newi];
if (newitem->hi_key != NULL) {
for (hash_T perturb = olditem->hi_hash;; perturb >>= PERTURB_SHIFT) {
newi = 5 * newi + perturb + 1;
newitem = &newarray[newi & newmask];
if (newitem->hi_key == NULL) {
break;
}
}
}
*newitem = *olditem;
todo--;
}

if (ht->ht_array != ht->ht_smallarray) {
xfree(ht->ht_array);
}
ht->ht_array = newarray;
ht->ht_mask = newmask;
ht->ht_filled = ht->ht_used;
}

#define HASH_CYCLE_BODY(hash, p) hash = hash * 101 + *p++








hash_T hash_hash(const char_u *key)
{
hash_T hash = *key;

if (hash == 0) {
return (hash_T)0;
}



const uint8_t *p = key + 1;
while (*p != NUL) {
HASH_CYCLE_BODY(hash, p);
}

return hash;
}










hash_T hash_hash_len(const char *key, const size_t len)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
if (len == 0) {
return 0;
}

hash_T hash = *(uint8_t *)key;
const uint8_t *end = (uint8_t *)key + len;

const uint8_t *p = (const uint8_t *)key + 1;
while (p < end) {
HASH_CYCLE_BODY(hash, p);
}

return hash;
}

#undef HASH_CYCLE_BODY





const char_u *_hash_key_removed(void)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
return HI_KEY_REMOVED;
}
