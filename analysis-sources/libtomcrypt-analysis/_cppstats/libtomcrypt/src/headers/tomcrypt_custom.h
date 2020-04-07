#if !defined(XMALLOC)
#define XMALLOC malloc
#endif
#if !defined(XREALLOC)
#define XREALLOC realloc
#endif
#if !defined(XCALLOC)
#define XCALLOC calloc
#endif
#if !defined(XFREE)
#define XFREE free
#endif
#if !defined(XMEMSET)
#define XMEMSET memset
#endif
#if !defined(XMEMCPY)
#define XMEMCPY memcpy
#endif
#if !defined(XMEMMOVE)
#define XMEMMOVE memmove
#endif
#if !defined(XMEMCMP)
#define XMEMCMP memcmp
#endif
#if !defined(XMEM_NEQ)
#define XMEM_NEQ mem_neq
#endif
#if !defined(XSTRCMP)
#define XSTRCMP strcmp
#endif
#if !defined(XSTRLEN)
#define XSTRLEN strlen
#endif
#if !defined(XSTRNCPY)
#define XSTRNCPY strncpy
#endif
#if !defined(XCLOCK)
#define XCLOCK clock
#endif
#if !defined(XQSORT)
#define XQSORT qsort
#endif
#if ( defined(malloc) || defined(realloc) || defined(calloc) || defined(free) || defined(memset) || defined(memcpy) || defined(memcmp) || defined(strcmp) || defined(strlen) || defined(strncpy) || defined(clock) || defined(qsort) ) && !defined(LTC_NO_PROTOTYPES)
#define LTC_NO_PROTOTYPES
#endif
#if defined LTC_NOTHING && !defined LTC_EASY
#define LTC_NO_CIPHERS
#define LTC_NO_MODES
#define LTC_NO_HASHES
#define LTC_NO_MACS
#define LTC_NO_PRNGS
#define LTC_NO_PK
#define LTC_NO_PKCS
#define LTC_NO_MISC
#endif 
#if defined(LTC_EASY)
#define LTC_NO_CIPHERS
#define LTC_RIJNDAEL
#define LTC_BLOWFISH
#define LTC_DES
#define LTC_CAST5
#define LTC_NO_MODES
#define LTC_ECB_MODE
#define LTC_CBC_MODE
#define LTC_CTR_MODE
#define LTC_NO_HASHES
#define LTC_SHA1
#define LTC_SHA3
#define LTC_SHA512
#define LTC_SHA384
#define LTC_SHA256
#define LTC_SHA224
#define LTC_HASH_HELPERS
#define LTC_NO_MACS
#define LTC_HMAC
#define LTC_OMAC
#define LTC_CCM_MODE
#define LTC_NO_PRNGS
#define LTC_SPRNG
#define LTC_YARROW
#define LTC_DEVRANDOM
#define LTC_TRY_URANDOM_FIRST
#define LTC_RNG_GET_BYTES
#define LTC_RNG_MAKE_PRNG
#define LTC_NO_PK
#define LTC_MRSA
#define LTC_MECC
#define LTC_NO_MISC
#define LTC_BASE64
#endif
#if defined(LTC_MINIMAL)
#define LTC_RIJNDAEL
#define LTC_SHA256
#define LTC_YARROW
#define LTC_CTR_MODE
#define LTC_RNG_MAKE_PRNG
#define LTC_RNG_GET_BYTES
#define LTC_DEVRANDOM
#define LTC_TRY_URANDOM_FIRST
#undef LTC_NO_FILE
#endif
#if !defined(LTC_NO_TEST)
#define LTC_TEST
#endif
#if !defined(LTC_NO_MATH)
#endif 
#if !defined(LTC_NO_CIPHERS)
#define LTC_BLOWFISH
#define LTC_RC2
#define LTC_RC5
#define LTC_RC6
#define LTC_SAFERP
#define LTC_RIJNDAEL
#define LTC_XTEA
#define LTC_TWOFISH
#if !defined(LTC_NO_TABLES)
#define LTC_TWOFISH_TABLES
#else
#define LTC_TWOFISH_SMALL
#endif
#define LTC_DES
#define LTC_CAST5
#define LTC_NOEKEON
#define LTC_SKIPJACK
#define LTC_SAFER
#define LTC_KHAZAD
#define LTC_ANUBIS
#define LTC_ANUBIS_TWEAK
#define LTC_KSEED
#define LTC_KASUMI
#define LTC_MULTI2
#define LTC_CAMELLIA
#define LTC_IDEA
#define LTC_SERPENT
#define LTC_TEA
#define LTC_CHACHA
#define LTC_SALSA20
#define LTC_XSALSA20
#define LTC_SOSEMANUK
#define LTC_RABBIT
#define LTC_RC4_STREAM
#define LTC_SOBER128_STREAM
#endif 
#if !defined(LTC_NO_MODES)
#define LTC_CFB_MODE
#define LTC_OFB_MODE
#define LTC_ECB_MODE
#define LTC_CBC_MODE
#define LTC_CTR_MODE
#define LTC_F8_MODE
#define LTC_LRW_MODE
#if !defined(LTC_NO_TABLES)
#define LTC_LRW_TABLES
#endif
#define LTC_XTS_MODE
#endif 
#if !defined(LTC_NO_HASHES)
#define LTC_CHC_HASH
#define LTC_WHIRLPOOL
#define LTC_SHA3
#define LTC_KECCAK
#define LTC_SHA512
#define LTC_SHA512_256
#define LTC_SHA512_224
#define LTC_SHA384
#define LTC_SHA256
#define LTC_SHA224
#define LTC_TIGER
#define LTC_SHA1
#define LTC_MD5
#define LTC_MD4
#define LTC_MD2
#define LTC_RIPEMD128
#define LTC_RIPEMD160
#define LTC_RIPEMD256
#define LTC_RIPEMD320
#define LTC_BLAKE2S
#define LTC_BLAKE2B
#define LTC_HASH_HELPERS
#endif 
#if !defined(LTC_NO_MACS)
#define LTC_HMAC
#define LTC_OMAC
#define LTC_PMAC
#define LTC_XCBC
#define LTC_F9_MODE
#define LTC_PELICAN
#define LTC_POLY1305
#define LTC_BLAKE2SMAC
#define LTC_BLAKE2BMAC
#define LTC_EAX_MODE
#define LTC_OCB_MODE
#define LTC_OCB3_MODE
#define LTC_CCM_MODE
#define LTC_GCM_MODE
#define LTC_CHACHA20POLY1305_MODE
#if !defined(LTC_NO_TABLES)
#define LTC_GCM_TABLES
#endif
#if defined(LTC_GCM_TABLES)
#endif
#endif 
#if !defined(LTC_NO_PRNGS)
#define LTC_YARROW
#define LTC_SPRNG
#define LTC_RC4
#define LTC_CHACHA20_PRNG
#define LTC_FORTUNA
#define LTC_SOBER128
#define LTC_DEVRANDOM
#define LTC_TRY_URANDOM_FIRST
#define LTC_RNG_GET_BYTES
#define LTC_RNG_MAKE_PRNG
#endif 
#if defined(LTC_YARROW)
#if defined(ENCRYPT_ONLY)
#define LTC_YARROW_AES 0
#else
#define LTC_YARROW_AES 2
#endif
#endif
#if defined(LTC_FORTUNA)
#if !defined(LTC_FORTUNA_RESEED_RATELIMIT_STATIC) && ((defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) || defined(_WIN32))
#define LTC_FORTUNA_RESEED_RATELIMIT_TIMED
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 17)
#define LTC_CLOCK_GETTIME
#endif
#elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
#define LTC_CLOCK_GETTIME
#endif
#else
#if !defined(LTC_FORTUNA_WD)
#define LTC_FORTUNA_WD 10
#endif
#if defined(LTC_FORTUNA_RESEED_RATELIMIT_TIMED)
#undef LTC_FORTUNA_RESEED_RATELIMIT_TIMED
#warning "undef'ed LTC_FORTUNA_RESEED_RATELIMIT_TIMED, looks like your architecture doesn't support it"
#endif
#endif
#if !defined(LTC_FORTUNA_POOLS)
#define LTC_FORTUNA_POOLS 32
#endif
#endif 
#if !defined(LTC_NO_PK)
#define LTC_MRSA
#define LTC_MDH
#define LTC_DH768
#define LTC_DH1024
#define LTC_DH1536
#define LTC_DH2048
#if defined(LTM_DESC) || defined(GMP_DESC)
#define LTC_DH3072
#define LTC_DH4096
#define LTC_DH6144
#define LTC_DH8192
#endif
#define LTC_MDSA
#define LTC_CURVE25519
#define LTC_MECC
#define LTC_ECC_SHAMIR
#if defined(TFM_DESC) && defined(LTC_MECC)
#define LTC_MECC_ACCEL
#endif
#endif 
#if defined(LTC_MRSA) && !defined(LTC_NO_RSA_BLINDING)
#define LTC_RSA_BLINDING
#endif 
#if defined(LTC_MRSA) && !defined(LTC_NO_RSA_CRT_HARDENING)
#define LTC_RSA_CRT_HARDENING
#endif 
#if defined(LTC_MECC) && !defined(LTC_NO_ECC_TIMING_RESISTANT)
#define LTC_ECC_TIMING_RESISTANT
#endif
#if !defined(LTC_NO_PKCS)
#define LTC_PKCS_1
#define LTC_PKCS_5
#define LTC_PKCS_8
#define LTC_PKCS_12
#define LTC_DER
#endif 
#if !defined(LTC_NO_MISC)
#define LTC_BASE64
#define LTC_BASE64_URL
#define LTC_BASE32
#define LTC_BASE16
#define LTC_BCRYPT
#if !defined(LTC_BCRYPT_DEFAULT_ROUNDS)
#define LTC_BCRYPT_DEFAULT_ROUNDS 10
#endif
#if !defined(LTC_NO_HKDF)
#define LTC_HKDF
#endif 
#define LTC_ADLER32
#define LTC_CRC32
#define LTC_SSH
#define LTC_PADDING
#define LTC_PBES
#endif 
#if defined(LTC_MECC)
#if !defined(LTC_NO_CURVES)
#define LTC_ECC_BRAINPOOLP160R1
#define LTC_ECC_BRAINPOOLP160T1
#define LTC_ECC_BRAINPOOLP192R1
#define LTC_ECC_BRAINPOOLP192T1
#define LTC_ECC_BRAINPOOLP224R1
#define LTC_ECC_BRAINPOOLP224T1
#define LTC_ECC_BRAINPOOLP256R1
#define LTC_ECC_BRAINPOOLP256T1
#define LTC_ECC_BRAINPOOLP320R1
#define LTC_ECC_BRAINPOOLP320T1
#define LTC_ECC_BRAINPOOLP384R1
#define LTC_ECC_BRAINPOOLP384T1
#define LTC_ECC_BRAINPOOLP512R1
#define LTC_ECC_BRAINPOOLP512T1
#define LTC_ECC_PRIME192V2
#define LTC_ECC_PRIME192V3
#define LTC_ECC_PRIME239V1
#define LTC_ECC_PRIME239V2
#define LTC_ECC_PRIME239V3
#define LTC_ECC_SECP112R1
#define LTC_ECC_SECP112R2
#define LTC_ECC_SECP128R1
#define LTC_ECC_SECP128R2
#define LTC_ECC_SECP160K1
#define LTC_ECC_SECP160R1
#define LTC_ECC_SECP160R2
#define LTC_ECC_SECP192K1
#define LTC_ECC_SECP192R1
#define LTC_ECC_SECP224K1
#define LTC_ECC_SECP224R1
#define LTC_ECC_SECP256K1
#define LTC_ECC_SECP256R1
#define LTC_ECC_SECP384R1
#define LTC_ECC_SECP521R1
#endif
#endif
#if defined(LTC_DER)
#if !defined(LTC_DER_MAX_RECURSION)
#define LTC_DER_MAX_RECURSION 30
#endif
#endif
#if defined(LTC_MECC) || defined(LTC_MRSA) || defined(LTC_MDSA) || defined(LTC_SSH)
#define LTC_MPI
#if !defined(LTC_PK_MAX_RETRIES)
#define LTC_PK_MAX_RETRIES 20
#endif
#endif
#if defined(LTC_MRSA)
#define LTC_PKCS_1
#endif
#if defined(LTC_MRSA) || defined(LTC_MECC)
#define LTC_PKCS_8
#endif
#if defined(LTC_PKCS_8)
#define LTC_PADDING
#define LTC_PBES
#endif
#if defined(LTC_PELICAN) && !defined(LTC_RIJNDAEL)
#error Pelican-MAC requires LTC_RIJNDAEL
#endif
#if defined(LTC_EAX_MODE) && !(defined(LTC_CTR_MODE) && defined(LTC_OMAC))
#error LTC_EAX_MODE requires CTR and LTC_OMAC mode
#endif
#if defined(LTC_YARROW) && !defined(LTC_CTR_MODE)
#error LTC_YARROW requires LTC_CTR_MODE chaining mode to be defined!
#endif
#if defined(LTC_DER) && !defined(LTC_MPI)
#error ASN.1 DER requires MPI functionality
#endif
#if (defined(LTC_MDSA) || defined(LTC_MRSA) || defined(LTC_MECC)) && !defined(LTC_DER)
#error PK requires ASN.1 DER functionality, make sure LTC_DER is enabled
#endif
#if defined(LTC_BCRYPT) && !defined(LTC_BLOWFISH)
#error LTC_BCRYPT requires LTC_BLOWFISH
#endif
#if defined(LTC_CHACHA20POLY1305_MODE) && (!defined(LTC_CHACHA) || !defined(LTC_POLY1305))
#error LTC_CHACHA20POLY1305_MODE requires LTC_CHACHA + LTC_POLY1305
#endif
#if defined(LTC_CHACHA20_PRNG) && !defined(LTC_CHACHA)
#error LTC_CHACHA20_PRNG requires LTC_CHACHA
#endif
#if defined(LTC_XSALSA20) && !defined(LTC_SALSA20)
#error LTC_XSALSA20 requires LTC_SALSA20
#endif
#if defined(LTC_RC4) && !defined(LTC_RC4_STREAM)
#error LTC_RC4 requires LTC_RC4_STREAM
#endif
#if defined(LTC_SOBER128) && !defined(LTC_SOBER128_STREAM)
#error LTC_SOBER128 requires LTC_SOBER128_STREAM
#endif
#if defined(LTC_BLAKE2SMAC) && !defined(LTC_BLAKE2S)
#error LTC_BLAKE2SMAC requires LTC_BLAKE2S
#endif
#if defined(LTC_BLAKE2BMAC) && !defined(LTC_BLAKE2B)
#error LTC_BLAKE2BMAC requires LTC_BLAKE2B
#endif
#if defined(LTC_SPRNG) && !defined(LTC_RNG_GET_BYTES)
#error LTC_SPRNG requires LTC_RNG_GET_BYTES
#endif
#if defined(LTC_NO_MATH) && (defined(LTM_DESC) || defined(TFM_DESC) || defined(GMP_DESC))
#error LTC_NO_MATH defined, but also a math descriptor
#endif
#if defined(LTC_PTHREAD)
#include <pthread.h>
#define LTC_MUTEX_GLOBAL(x) pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
#define LTC_MUTEX_PROTO(x) extern pthread_mutex_t x;
#define LTC_MUTEX_TYPE(x) pthread_mutex_t x;
#define LTC_MUTEX_INIT(x) LTC_ARGCHK(pthread_mutex_init(x, NULL) == 0);
#define LTC_MUTEX_LOCK(x) LTC_ARGCHK(pthread_mutex_lock(x) == 0);
#define LTC_MUTEX_UNLOCK(x) LTC_ARGCHK(pthread_mutex_unlock(x) == 0);
#define LTC_MUTEX_DESTROY(x) LTC_ARGCHK(pthread_mutex_destroy(x) == 0);
#else
#define LTC_MUTEX_GLOBAL(x)
#define LTC_MUTEX_PROTO(x)
#define LTC_MUTEX_TYPE(x)
#define LTC_MUTEX_INIT(x)
#define LTC_MUTEX_LOCK(x)
#define LTC_MUTEX_UNLOCK(x)
#define LTC_MUTEX_DESTROY(x)
#endif
#if !defined(LTC_NO_FILE)
#if !defined(LTC_FILE_READ_BUFSIZE)
#define LTC_FILE_READ_BUFSIZE 8192
#endif
#endif
#if !defined(LTC_ECC_SECP112R1) && defined(LTC_ECC112)
#define LTC_ECC_SECP112R1
#undef LTC_ECC112
#endif
#if !defined(LTC_ECC_SECP128R1) && defined(LTC_ECC128)
#define LTC_ECC_SECP128R1
#undef LTC_ECC128
#endif
#if !defined(LTC_ECC_SECP160R1) && defined(LTC_ECC160)
#define LTC_ECC_SECP160R1
#undef LTC_ECC160
#endif
#if !defined(LTC_ECC_SECP192R1) && defined(LTC_ECC192)
#define LTC_ECC_SECP192R1
#undef LTC_ECC192
#endif
#if !defined(LTC_ECC_SECP224R1) && defined(LTC_ECC224)
#define LTC_ECC_SECP224R1
#undef LTC_ECC224
#endif
#if !defined(LTC_ECC_SECP256R1) && defined(LTC_ECC256)
#define LTC_ECC_SECP256R1
#undef LTC_ECC256
#endif
#if !defined(LTC_ECC_SECP384R1) && defined(LTC_ECC384)
#define LTC_ECC_SECP384R1
#undef LTC_ECC384
#endif
#if !defined(LTC_ECC_SECP512R1) && defined(LTC_ECC521)
#define LTC_ECC_SECP521R1
#undef LTC_ECC521
#endif
