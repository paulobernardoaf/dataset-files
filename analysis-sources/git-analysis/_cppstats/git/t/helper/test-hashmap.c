#include "test-tool.h"
#include "git-compat-util.h"
#include "hashmap.h"
#include "strbuf.h"
struct test_entry
{
int padding; 
struct hashmap_entry ent;
char key[FLEX_ARRAY];
};
static const char *get_value(const struct test_entry *e)
{
return e->key + strlen(e->key) + 1;
}
static int test_entry_cmp(const void *cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *keydata)
{
const int ignore_case = cmp_data ? *((int *)cmp_data) : 0;
const struct test_entry *e1, *e2;
const char *key = keydata;
e1 = container_of(eptr, const struct test_entry, ent);
e2 = container_of(entry_or_key, const struct test_entry, ent);
if (ignore_case)
return strcasecmp(e1->key, key ? key : e2->key);
else
return strcmp(e1->key, key ? key : e2->key);
}
static struct test_entry *alloc_test_entry(unsigned int hash,
char *key, char *value)
{
size_t klen = strlen(key);
size_t vlen = strlen(value);
struct test_entry *entry = xmalloc(st_add4(sizeof(*entry), klen, vlen, 2));
hashmap_entry_init(&entry->ent, hash);
memcpy(entry->key, key, klen + 1);
memcpy(entry->key + klen + 1, value, vlen + 1);
return entry;
}
#define HASH_METHOD_FNV 0
#define HASH_METHOD_I 1
#define HASH_METHOD_IDIV10 2
#define HASH_METHOD_0 3
#define HASH_METHOD_X2 4
#define TEST_SPARSE 8
#define TEST_ADD 16
#define TEST_SIZE 100000
static unsigned int hash(unsigned int method, unsigned int i, const char *key)
{
unsigned int hash = 0;
switch (method & 3)
{
case HASH_METHOD_FNV:
hash = strhash(key);
break;
case HASH_METHOD_I:
hash = i;
break;
case HASH_METHOD_IDIV10:
hash = i / 10;
break;
case HASH_METHOD_0:
hash = 0;
break;
}
if (method & HASH_METHOD_X2)
hash = 2 * hash;
return hash;
}
static void perf_hashmap(unsigned int method, unsigned int rounds)
{
struct hashmap map;
char buf[16];
struct test_entry **entries;
unsigned int *hashes;
unsigned int i, j;
ALLOC_ARRAY(entries, TEST_SIZE);
ALLOC_ARRAY(hashes, TEST_SIZE);
for (i = 0; i < TEST_SIZE; i++) {
xsnprintf(buf, sizeof(buf), "%i", i);
entries[i] = alloc_test_entry(0, buf, "");
hashes[i] = hash(method, i, entries[i]->key);
}
if (method & TEST_ADD) {
for (j = 0; j < rounds; j++) {
hashmap_init(&map, test_entry_cmp, NULL, 0);
for (i = 0; i < TEST_SIZE; i++) {
hashmap_entry_init(&entries[i]->ent, hashes[i]);
hashmap_add(&map, &entries[i]->ent);
}
hashmap_free(&map);
}
} else {
hashmap_init(&map, test_entry_cmp, NULL, 0);
j = (method & TEST_SPARSE) ? TEST_SIZE / 10 : TEST_SIZE;
for (i = 0; i < j; i++) {
hashmap_entry_init(&entries[i]->ent, hashes[i]);
hashmap_add(&map, &entries[i]->ent);
}
for (j = 0; j < rounds; j++) {
for (i = 0; i < TEST_SIZE; i++) {
hashmap_get_from_hash(&map, hashes[i],
entries[i]->key);
}
}
hashmap_free(&map);
}
}
#define DELIM " \t\r\n"
int cmd__hashmap(int argc, const char **argv)
{
struct strbuf line = STRBUF_INIT;
struct hashmap map;
int icase;
icase = argc > 1 && !strcmp("ignorecase", argv[1]);
hashmap_init(&map, test_entry_cmp, &icase, 0);
while (strbuf_getline(&line, stdin) != EOF) {
char *cmd, *p1 = NULL, *p2 = NULL;
unsigned int hash = 0;
struct test_entry *entry;
cmd = strtok(line.buf, DELIM);
if (!cmd || *cmd == '#')
continue;
p1 = strtok(NULL, DELIM);
if (p1) {
hash = icase ? strihash(p1) : strhash(p1);
p2 = strtok(NULL, DELIM);
}
if (!strcmp("add", cmd) && p1 && p2) {
entry = alloc_test_entry(hash, p1, p2);
hashmap_add(&map, &entry->ent);
} else if (!strcmp("put", cmd) && p1 && p2) {
entry = alloc_test_entry(hash, p1, p2);
entry = hashmap_put_entry(&map, entry, ent);
puts(entry ? get_value(entry) : "NULL");
free(entry);
} else if (!strcmp("get", cmd) && p1) {
entry = hashmap_get_entry_from_hash(&map, hash, p1,
struct test_entry, ent);
if (!entry)
puts("NULL");
hashmap_for_each_entry_from(&map, entry, ent)
puts(get_value(entry));
} else if (!strcmp("remove", cmd) && p1) {
struct hashmap_entry key;
struct hashmap_entry *rm;
hashmap_entry_init(&key, hash);
rm = hashmap_remove(&map, &key, p1);
entry = rm ? container_of(rm, struct test_entry, ent)
: NULL;
puts(entry ? get_value(entry) : "NULL");
free(entry);
} else if (!strcmp("iterate", cmd)) {
struct hashmap_iter iter;
hashmap_for_each_entry(&map, &iter, entry,
ent )
printf("%s %s\n", entry->key, get_value(entry));
} else if (!strcmp("size", cmd)) {
printf("%u %u\n", map.tablesize,
hashmap_get_size(&map));
} else if (!strcmp("intern", cmd) && p1) {
const char *i1 = strintern(p1);
const char *i2 = strintern(p1);
if (strcmp(i1, p1))
printf("strintern(%s) returns %s\n", p1, i1);
else if (i1 == p1)
printf("strintern(%s) returns input pointer\n", p1);
else if (i1 != i2)
printf("strintern(%s) != strintern(%s)", i1, i2);
else
printf("%s\n", i1);
} else if (!strcmp("perfhashmap", cmd) && p1 && p2) {
perf_hashmap(atoi(p1), atoi(p2));
} else {
printf("Unknown command %s\n", cmd);
}
}
strbuf_release(&line);
hashmap_free_entries(&map, struct test_entry, ent);
return 0;
}
