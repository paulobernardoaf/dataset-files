#if !defined(R_DES_H)
#define R_DES_H

#define DES_KEY_SIZE 8
#define DES_BLOCK_SIZE 8

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct des_context_t {
ut32 round_key_lo[16]; 
ut32 round_key_hi[16]; 
int round;
} RDESContext;

R_API void r_des_permute_key (ut32 *keylo, ut32 *keyhi);
R_API void r_des_permute_block0 (ut32 *blocklo, ut32 *blockhi);
R_API void r_des_permute_block1 (ut32 *blocklo, ut32 *blockhi);
R_API void r_des_round_key (int i, ut32 *keylo, ut32 *keyhi, ut32 *deskeylo, ut32 *deskeyhi);
R_API void r_des_round (ut32 *buflo, ut32 *bufhi, ut32 *roundkeylo, ut32 *roundkeyhi);

#if defined(__cplusplus)
}
#endif

#endif 
