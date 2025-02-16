


#include <ruby/ruby.h>
#include "../digest.h"
#if defined(SHA2_USE_OPENSSL)
#include "sha2ossl.h"
#elif defined(SHA2_USE_COMMONDIGEST)
#include "sha2cc.h"
#else
#include "sha2.h"
#endif

#define FOREACH_BITLEN(func) func(256) func(384) func(512)

#define DEFINE_ALGO_METADATA(bitlen) static const rb_digest_metadata_t sha##bitlen = { RUBY_DIGEST_API_VERSION, SHA##bitlen##_DIGEST_LENGTH, SHA##bitlen##_BLOCK_LENGTH, sizeof(SHA##bitlen##_CTX), (rb_digest_hash_init_func_t)SHA##bitlen##_Init, (rb_digest_hash_update_func_t)SHA##bitlen##_Update, (rb_digest_hash_finish_func_t)SHA##bitlen##_Finish, };










FOREACH_BITLEN(DEFINE_ALGO_METADATA)






void
Init_sha2(void)
{
VALUE mDigest, cDigest_Base;
ID id_metadata = rb_id_metadata();

#define DECLARE_ALGO_CLASS(bitlen) VALUE cDigest_SHA##bitlen;


FOREACH_BITLEN(DECLARE_ALGO_CLASS)

mDigest = rb_digest_namespace();
cDigest_Base = rb_path2class("Digest::Base");

#define DEFINE_ALGO_CLASS(bitlen) cDigest_SHA##bitlen = rb_define_class_under(mDigest, "SHA" #bitlen, cDigest_Base); rb_ivar_set(cDigest_SHA##bitlen, id_metadata, Data_Wrap_Struct(0, 0, 0, (void *)&sha##bitlen));





#undef RUBY_UNTYPED_DATA_WARNING
#define RUBY_UNTYPED_DATA_WARNING 0
FOREACH_BITLEN(DEFINE_ALGO_CLASS)
}
