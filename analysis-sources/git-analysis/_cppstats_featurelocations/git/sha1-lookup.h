#if !defined(SHA1_LOOKUP_H)
#define SHA1_LOOKUP_H

typedef const unsigned char *sha1_access_fn(size_t index, void *table);

int sha1_pos(const unsigned char *sha1,
void *table,
size_t nr,
sha1_access_fn fn);




















int bsearch_hash(const unsigned char *sha1, const uint32_t *fanout_nbo,
const unsigned char *table, size_t stride, uint32_t *result);
#endif
