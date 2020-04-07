#include "ruby.h"
#define RUBY_DIGEST_API_VERSION 3
typedef int (*rb_digest_hash_init_func_t)(void *);
typedef void (*rb_digest_hash_update_func_t)(void *, unsigned char *, size_t);
typedef int (*rb_digest_hash_finish_func_t)(void *, unsigned char *);
typedef struct {
int api_version;
size_t digest_len;
size_t block_len;
size_t ctx_size;
rb_digest_hash_init_func_t init_func;
rb_digest_hash_update_func_t update_func;
rb_digest_hash_finish_func_t finish_func;
} rb_digest_metadata_t;
#define DEFINE_UPDATE_FUNC_FOR_UINT(name) void rb_digest_##name##_update(void *ctx, unsigned char *ptr, size_t size) { const unsigned int stride = 16384; for (; size > stride; size -= stride, ptr += stride) { name##_Update(ctx, ptr, stride); } if (size > 0) name##_Update(ctx, ptr, size); }
#define DEFINE_FINISH_FUNC_FROM_FINAL(name) int rb_digest_##name##_finish(void *ctx, unsigned char *ptr) { return name##_Final(ptr, ctx); }
static inline VALUE
rb_digest_namespace(void)
{
rb_require("digest");
return rb_path2class("Digest");
}
static inline ID
rb_id_metadata(void)
{
return rb_intern_const("metadata");
}
