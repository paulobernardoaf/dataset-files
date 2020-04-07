

















#if !defined(AVCODEC_AUDIODSP_H)
#define AVCODEC_AUDIODSP_H

#include <stdint.h>

typedef struct AudioDSPContext {




int32_t (*scalarproduct_int16)(const int16_t *v1,
const int16_t *v2 , int len);















void (*vector_clip_int32)(int32_t *dst, const int32_t *src, int32_t min,
int32_t max, unsigned int len);

void (*vector_clipf)(float *dst ,
const float *src ,
int len ,
float min, float max);
} AudioDSPContext;

void ff_audiodsp_init(AudioDSPContext *c);
void ff_audiodsp_init_arm(AudioDSPContext *c);
void ff_audiodsp_init_ppc(AudioDSPContext *c);
void ff_audiodsp_init_x86(AudioDSPContext *c);

#endif 
