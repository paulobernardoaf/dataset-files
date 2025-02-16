#include "git-compat-util.h"
#if defined(SHA1_PPC)
#include "ppc/sha1.h"
#elif defined(SHA1_APPLE)
#include <CommonCrypto/CommonDigest.h>
#elif defined(SHA1_OPENSSL)
#include <openssl/sha.h>
#elif defined(SHA1_DC)
#include "sha1dc_git.h"
#else 
#include "block-sha1/sha1.h"
#endif
#if defined(SHA256_GCRYPT)
#define SHA256_NEEDS_CLONE_HELPER
#include "sha256/gcrypt.h"
#elif defined(SHA256_OPENSSL)
#include <openssl/sha.h>
#else
#include "sha256/block/sha256.h"
#endif
#if !defined(platform_SHA_CTX)
#define platform_SHA_CTX SHA_CTX
#define platform_SHA1_Init SHA1_Init
#define platform_SHA1_Update SHA1_Update
#define platform_SHA1_Final SHA1_Final
#endif
#define git_SHA_CTX platform_SHA_CTX
#define git_SHA1_Init platform_SHA1_Init
#define git_SHA1_Update platform_SHA1_Update
#define git_SHA1_Final platform_SHA1_Final
#if !defined(platform_SHA256_CTX)
#define platform_SHA256_CTX SHA256_CTX
#define platform_SHA256_Init SHA256_Init
#define platform_SHA256_Update SHA256_Update
#define platform_SHA256_Final SHA256_Final
#endif
#define git_SHA256_CTX platform_SHA256_CTX
#define git_SHA256_Init platform_SHA256_Init
#define git_SHA256_Update platform_SHA256_Update
#define git_SHA256_Final platform_SHA256_Final
#if defined(platform_SHA256_Clone)
#define git_SHA256_Clone platform_SHA256_Clone
#endif
#if defined(SHA1_MAX_BLOCK_SIZE)
#include "compat/sha1-chunked.h"
#undef git_SHA1_Update
#define git_SHA1_Update git_SHA1_Update_Chunked
#endif
static inline void git_SHA1_Clone(git_SHA_CTX *dst, const git_SHA_CTX *src)
{
memcpy(dst, src, sizeof(*dst));
}
#if !defined(SHA256_NEEDS_CLONE_HELPER)
static inline void git_SHA256_Clone(git_SHA256_CTX *dst, const git_SHA256_CTX *src)
{
memcpy(dst, src, sizeof(*dst));
}
#endif
#define GIT_HASH_UNKNOWN 0
#define GIT_HASH_SHA1 1
#define GIT_HASH_SHA256 2
#define GIT_HASH_NALGOS (GIT_HASH_SHA256 + 1)
union git_hash_ctx {
git_SHA_CTX sha1;
git_SHA256_CTX sha256;
};
typedef union git_hash_ctx git_hash_ctx;
typedef void (*git_hash_init_fn)(git_hash_ctx *ctx);
typedef void (*git_hash_clone_fn)(git_hash_ctx *dst, const git_hash_ctx *src);
typedef void (*git_hash_update_fn)(git_hash_ctx *ctx, const void *in, size_t len);
typedef void (*git_hash_final_fn)(unsigned char *hash, git_hash_ctx *ctx);
struct git_hash_algo {
const char *name;
uint32_t format_id;
size_t rawsz;
size_t hexsz;
size_t blksz;
git_hash_init_fn init_fn;
git_hash_clone_fn clone_fn;
git_hash_update_fn update_fn;
git_hash_final_fn final_fn;
const struct object_id *empty_tree;
const struct object_id *empty_blob;
};
extern const struct git_hash_algo hash_algos[GIT_HASH_NALGOS];
int hash_algo_by_name(const char *name);
int hash_algo_by_id(uint32_t format_id);
int hash_algo_by_length(int len);
static inline int hash_algo_by_ptr(const struct git_hash_algo *p)
{
return p - hash_algos;
}
#define GIT_SHA1_RAWSZ 20
#define GIT_SHA1_HEXSZ (2 * GIT_SHA1_RAWSZ)
#define GIT_SHA1_BLKSZ 64
#define GIT_SHA256_RAWSZ 32
#define GIT_SHA256_HEXSZ (2 * GIT_SHA256_RAWSZ)
#define GIT_SHA256_BLKSZ 64
#define GIT_MAX_RAWSZ GIT_SHA256_RAWSZ
#define GIT_MAX_HEXSZ GIT_SHA256_HEXSZ
#define GIT_MAX_BLKSZ GIT_SHA256_BLKSZ
struct object_id {
unsigned char hash[GIT_MAX_RAWSZ];
};
#define the_hash_algo the_repository->hash_algo
