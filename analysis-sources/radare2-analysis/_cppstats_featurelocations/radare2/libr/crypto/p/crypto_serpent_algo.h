#if !defined(CRYPTO_SERPENT_ALGO_H)
#define CRYPTO_SERPENT_ALGO_H

#include <r_crypto.h>
#define DW_BY_BLOCK 4
#define DW_BY_USERKEY 8
#define NB_ROUNDS 32
#define NB_SUBKEYS 33
#define NIBBLES_BY_SUBKEY 32

struct serpent_state {
ut32 key[8];
int key_size;
};







void serpent_encrypt(struct serpent_state *st, ut32 in[DW_BY_BLOCK], ut32 out[DW_BY_BLOCK]);







void serpent_decrypt(struct serpent_state *st, ut32 in[DW_BY_BLOCK], ut32 out[DW_BY_BLOCK]);






void serpent_keyschedule(struct serpent_state st,
ut32 subkeys[NB_SUBKEYS*DW_BY_BLOCK]);

#endif
