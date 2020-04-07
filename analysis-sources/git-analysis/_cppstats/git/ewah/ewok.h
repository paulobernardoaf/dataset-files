struct strbuf;
typedef uint64_t eword_t;
#define BITS_IN_EWORD (sizeof(eword_t) * 8)
static inline uint32_t ewah_bit_popcount64(uint64_t x)
{
x = (x & 0x5555555555555555ULL) + ((x >> 1) & 0x5555555555555555ULL);
x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL);
return (x * 0x0101010101010101ULL) >> 56;
}
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR > 3))
#define ewah_bit_ctz64(x) __builtin_ctzll(x)
#else
static inline int ewah_bit_ctz64(uint64_t x)
{
int n = 0;
if ((x & 0xffffffff) == 0) { x >>= 32; n += 32; }
if ((x & 0xffff) == 0) { x >>= 16; n += 16; }
if ((x & 0xff) == 0) { x >>= 8; n += 8; }
if ((x & 0xf) == 0) { x >>= 4; n += 4; }
if ((x & 0x3) == 0) { x >>= 2; n += 2; }
if ((x & 0x1) == 0) { x >>= 1; n += 1; }
return n + !x;
}
#endif
struct ewah_bitmap {
eword_t *buffer;
size_t buffer_size;
size_t alloc_size;
size_t bit_size;
eword_t *rlw;
};
typedef void (*ewah_callback)(size_t pos, void *);
struct ewah_bitmap *ewah_pool_new(void);
void ewah_pool_free(struct ewah_bitmap *self);
struct ewah_bitmap *ewah_new(void);
void ewah_free(struct ewah_bitmap *self);
int ewah_serialize_to(struct ewah_bitmap *self,
int (*write_fun)(void *out, const void *buf, size_t len),
void *out);
int ewah_serialize_strbuf(struct ewah_bitmap *self, struct strbuf *);
ssize_t ewah_read_mmap(struct ewah_bitmap *self, const void *map, size_t len);
uint32_t ewah_checksum(struct ewah_bitmap *self);
void ewah_each_bit(struct ewah_bitmap *self, ewah_callback callback, void *payload);
void ewah_set(struct ewah_bitmap *self, size_t i);
struct ewah_iterator {
const eword_t *buffer;
size_t buffer_size;
size_t pointer;
eword_t compressed, literals;
eword_t rl, lw;
int b;
};
void ewah_iterator_init(struct ewah_iterator *it, struct ewah_bitmap *parent);
int ewah_iterator_next(eword_t *next, struct ewah_iterator *it);
void ewah_xor(
struct ewah_bitmap *ewah_i,
struct ewah_bitmap *ewah_j,
struct ewah_bitmap *out);
size_t ewah_add_empty_words(struct ewah_bitmap *self, int v, size_t number);
void ewah_add_dirty_words(
struct ewah_bitmap *self, const eword_t *buffer, size_t number, int negate);
size_t ewah_add(struct ewah_bitmap *self, eword_t word);
struct bitmap {
eword_t *words;
size_t word_alloc;
};
struct bitmap *bitmap_new(void);
struct bitmap *bitmap_word_alloc(size_t word_alloc);
void bitmap_set(struct bitmap *self, size_t pos);
void bitmap_unset(struct bitmap *self, size_t pos);
int bitmap_get(struct bitmap *self, size_t pos);
void bitmap_reset(struct bitmap *self);
void bitmap_free(struct bitmap *self);
int bitmap_equals(struct bitmap *self, struct bitmap *other);
int bitmap_is_subset(struct bitmap *self, struct bitmap *super);
struct ewah_bitmap * bitmap_to_ewah(struct bitmap *bitmap);
struct bitmap *ewah_to_bitmap(struct ewah_bitmap *ewah);
void bitmap_and_not(struct bitmap *self, struct bitmap *other);
void bitmap_or_ewah(struct bitmap *self, struct ewah_bitmap *other);
void bitmap_or(struct bitmap *self, const struct bitmap *other);
size_t bitmap_popcount(struct bitmap *self);
