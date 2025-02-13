#include <ruby/ruby.h>
#include "../digest.h"
#if defined(SHA1_USE_OPENSSL)
#include "sha1ossl.h"
#elif defined(SHA1_USE_COMMONDIGEST)
#include "sha1cc.h"
#else
#include "sha1.h"
#endif
static const rb_digest_metadata_t sha1 = {
RUBY_DIGEST_API_VERSION,
SHA1_DIGEST_LENGTH,
SHA1_BLOCK_LENGTH,
sizeof(SHA1_CTX),
(rb_digest_hash_init_func_t)SHA1_Init,
(rb_digest_hash_update_func_t)SHA1_Update,
(rb_digest_hash_finish_func_t)SHA1_Finish,
};
void
Init_sha1(void)
{
VALUE mDigest, cDigest_Base, cDigest_SHA1;
#if 0
mDigest = rb_define_module("Digest"); 
#endif
mDigest = rb_digest_namespace();
cDigest_Base = rb_path2class("Digest::Base");
cDigest_SHA1 = rb_define_class_under(mDigest, "SHA1", cDigest_Base);
#undef RUBY_UNTYPED_DATA_WARNING
#define RUBY_UNTYPED_DATA_WARNING 0
rb_iv_set(cDigest_SHA1, "metadata",
Data_Wrap_Struct(0, 0, 0, (void *)&sha1));
}
