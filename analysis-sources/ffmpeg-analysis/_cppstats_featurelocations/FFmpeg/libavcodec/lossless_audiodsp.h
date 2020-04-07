





















#if !defined(AVCODEC_LOSSLESS_AUDIODSP_H)
#define AVCODEC_LOSSLESS_AUDIODSP_H

#include <stdint.h>

typedef struct LLAudDSPContext {






int32_t (*scalarproduct_and_madd_int16)(int16_t *v1 ,
const int16_t *v2,
const int16_t *v3,
int len, int mul);

int32_t (*scalarproduct_and_madd_int32)(int16_t *v1 ,
const int32_t *v2,
const int16_t *v3,
int len, int mul);
} LLAudDSPContext;

void ff_llauddsp_init(LLAudDSPContext *c);
void ff_llauddsp_init_arm(LLAudDSPContext *c);
void ff_llauddsp_init_ppc(LLAudDSPContext *c);
void ff_llauddsp_init_x86(LLAudDSPContext *c);

#endif 
