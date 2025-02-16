












#include "tomcrypt_private.h"

#if defined(LTC_GCM_MODE)





void gcm_mult_h(const gcm_state *gcm, unsigned char *I)
{
unsigned char T[16];
#if defined(LTC_GCM_TABLES)
int x;
#if defined(LTC_GCM_TABLES_SSE2)
asm("movdqa (%0),%%xmm0"::"r"(&gcm->PC[0][I[0]][0]));
for (x = 1; x < 16; x++) {
asm("pxor (%0),%%xmm0"::"r"(&gcm->PC[x][I[x]][0]));
}
asm("movdqa %%xmm0,(%0)"::"r"(&T));
#else
int y;
XMEMCPY(T, &gcm->PC[0][I[0]][0], 16);
for (x = 1; x < 16; x++) {
#if defined(LTC_FAST)
for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
*(LTC_FAST_TYPE_PTR_CAST(T + y)) ^= *(LTC_FAST_TYPE_PTR_CAST(&gcm->PC[x][I[x]][y]));
}
#else
for (y = 0; y < 16; y++) {
T[y] ^= gcm->PC[x][I[x]][y];
}
#endif 
}
#endif 
#else
gcm_gf_mult(gcm->H, I, T);
#endif
XMEMCPY(I, T, 16);
}
#endif




