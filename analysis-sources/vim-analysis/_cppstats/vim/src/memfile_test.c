#undef NDEBUG
#include <assert.h>
#define NO_VIM_MAIN
#include "main.c"
#include "memfile.c"
#define index_to_key(i) ((i) ^ 15167)
#define TEST_COUNT 50000
static void
test_mf_hash(void)
{
mf_hashtab_T ht;
mf_hashitem_T *item;
blocknr_T key;
long_u i;
long_u num_buckets;
mf_hash_init(&ht);
for (i = 0; i < TEST_COUNT; i++)
{
assert(ht.mht_count == i);
num_buckets = ht.mht_mask + 1;
assert(num_buckets > 0 && (num_buckets & (num_buckets - 1)) == 0);
assert(ht.mht_count <= (num_buckets << MHT_LOG_LOAD_FACTOR));
if (i < (MHT_INIT_SIZE << MHT_LOG_LOAD_FACTOR))
{
assert(num_buckets == MHT_INIT_SIZE);
assert(ht.mht_buckets == ht.mht_small_buckets);
}
else
{
assert(num_buckets > MHT_INIT_SIZE);
assert(ht.mht_buckets != ht.mht_small_buckets);
}
key = index_to_key(i);
assert(mf_hash_find(&ht, key) == NULL);
item = LALLOC_CLEAR_ONE(mf_hashitem_T);
assert(item != NULL);
item->mhi_key = key;
mf_hash_add_item(&ht, item);
assert(mf_hash_find(&ht, key) == item);
if (ht.mht_mask + 1 != num_buckets)
{
assert(ht.mht_mask + 1 == num_buckets * MHT_GROWTH_FACTOR);
assert(i + 1 == (num_buckets << MHT_LOG_LOAD_FACTOR));
}
}
for (i = 0; i < TEST_COUNT; i++)
{
key = index_to_key(i);
item = mf_hash_find(&ht, key);
assert(item != NULL);
assert(item->mhi_key == key);
}
for (i = 0; i < TEST_COUNT; i++)
{
if (i % 100 < 70)
{
key = index_to_key(i);
item = mf_hash_find(&ht, key);
assert(item != NULL);
assert(item->mhi_key == key);
mf_hash_rem_item(&ht, item);
assert(mf_hash_find(&ht, key) == NULL);
mf_hash_add_item(&ht, item);
assert(mf_hash_find(&ht, key) == item);
mf_hash_rem_item(&ht, item);
assert(mf_hash_find(&ht, key) == NULL);
vim_free(item);
}
}
for (i = 0; i < TEST_COUNT; i++)
{
key = index_to_key(i);
item = mf_hash_find(&ht, key);
if (i % 100 < 70)
{
assert(item == NULL);
}
else
{
assert(item != NULL);
assert(item->mhi_key == key);
}
}
mf_hash_free_all(&ht);
}
int
main(void)
{
test_mf_hash();
return 0;
}
