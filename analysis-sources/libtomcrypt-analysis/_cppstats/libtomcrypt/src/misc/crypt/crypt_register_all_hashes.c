#include "tomcrypt_private.h"
#define REGISTER_HASH(h) do {LTC_ARGCHK(register_hash(h) != -1); } while(0)
int register_all_hashes(void)
{
#if defined(LTC_TIGER)
REGISTER_HASH(&tiger_desc);
#endif
#if defined(LTC_MD2)
REGISTER_HASH(&md2_desc);
#endif
#if defined(LTC_MD4)
REGISTER_HASH(&md4_desc);
#endif
#if defined(LTC_MD5)
REGISTER_HASH(&md5_desc);
#endif
#if defined(LTC_SHA1)
REGISTER_HASH(&sha1_desc);
#endif
#if defined(LTC_SHA224)
REGISTER_HASH(&sha224_desc);
#endif
#if defined(LTC_SHA256)
REGISTER_HASH(&sha256_desc);
#endif
#if defined(LTC_SHA384)
REGISTER_HASH(&sha384_desc);
#endif
#if defined(LTC_SHA512)
REGISTER_HASH(&sha512_desc);
#endif
#if defined(LTC_SHA512_224)
REGISTER_HASH(&sha512_224_desc);
#endif
#if defined(LTC_SHA512_256)
REGISTER_HASH(&sha512_256_desc);
#endif
#if defined(LTC_SHA3)
REGISTER_HASH(&sha3_224_desc);
REGISTER_HASH(&sha3_256_desc);
REGISTER_HASH(&sha3_384_desc);
REGISTER_HASH(&sha3_512_desc);
#endif
#if defined(LTC_KECCAK)
REGISTER_HASH(&keccak_224_desc);
REGISTER_HASH(&keccak_256_desc);
REGISTER_HASH(&keccak_384_desc);
REGISTER_HASH(&keccak_512_desc);
#endif
#if defined(LTC_RIPEMD128)
REGISTER_HASH(&rmd128_desc);
#endif
#if defined(LTC_RIPEMD160)
REGISTER_HASH(&rmd160_desc);
#endif
#if defined(LTC_RIPEMD256)
REGISTER_HASH(&rmd256_desc);
#endif
#if defined(LTC_RIPEMD320)
REGISTER_HASH(&rmd320_desc);
#endif
#if defined(LTC_WHIRLPOOL)
REGISTER_HASH(&whirlpool_desc);
#endif
#if defined(LTC_BLAKE2S)
REGISTER_HASH(&blake2s_128_desc);
REGISTER_HASH(&blake2s_160_desc);
REGISTER_HASH(&blake2s_224_desc);
REGISTER_HASH(&blake2s_256_desc);
#endif
#if defined(LTC_BLAKE2S)
REGISTER_HASH(&blake2b_160_desc);
REGISTER_HASH(&blake2b_256_desc);
REGISTER_HASH(&blake2b_384_desc);
REGISTER_HASH(&blake2b_512_desc);
#endif
#if defined(LTC_CHC_HASH)
REGISTER_HASH(&chc_desc);
LTC_ARGCHK(chc_register(find_cipher_any("aes", 8, 16)) == CRYPT_OK);
#endif
return CRYPT_OK;
}
