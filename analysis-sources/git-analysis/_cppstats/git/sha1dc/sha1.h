#if defined(__cplusplus)
extern "C" {
#endif
#if !defined(SHA1DC_NO_STANDARD_INCLUDES)
#include <stdint.h>
#endif
void sha1_compression_states(uint32_t[5], const uint32_t[16], uint32_t[80], uint32_t[80][5]);
typedef void(*sha1_recompression_type)(uint32_t*, uint32_t*, const uint32_t*, const uint32_t*);
typedef void(*collision_block_callback)(uint64_t, const uint32_t*, const uint32_t*, const uint32_t*, const uint32_t*);
typedef struct {
uint64_t total;
uint32_t ihv[5];
unsigned char buffer[64];
int found_collision;
int safe_hash;
int detect_coll;
int ubc_check;
int reduced_round_coll;
collision_block_callback callback;
uint32_t ihv1[5];
uint32_t ihv2[5];
uint32_t m1[80];
uint32_t m2[80];
uint32_t states[80][5];
} SHA1_CTX;
void SHA1DCInit(SHA1_CTX*);
void SHA1DCSetSafeHash(SHA1_CTX*, int);
void SHA1DCSetUseUBC(SHA1_CTX*, int);
void SHA1DCSetUseDetectColl(SHA1_CTX*, int);
void SHA1DCSetDetectReducedRoundCollision(SHA1_CTX*, int);
void SHA1DCSetCallback(SHA1_CTX*, collision_block_callback);
void SHA1DCUpdate(SHA1_CTX*, const char*, size_t);
int SHA1DCFinal(unsigned char[20], SHA1_CTX*);
#if defined(__cplusplus)
}
#endif
#if defined(SHA1DC_CUSTOM_TRAILING_INCLUDE_SHA1_H)
#include SHA1DC_CUSTOM_TRAILING_INCLUDE_SHA1_H
#endif
