

















#if !defined(AVCODEC_VORBISDSP_H)
#define AVCODEC_VORBISDSP_H

#include <stdint.h>

typedef struct VorbisDSPContext {

void (*vorbis_inverse_coupling)(float *mag, float *ang,
intptr_t blocksize);
} VorbisDSPContext;

void ff_vorbisdsp_init(VorbisDSPContext *dsp);


void ff_vorbisdsp_init_aarch64(VorbisDSPContext *dsp);
void ff_vorbisdsp_init_x86(VorbisDSPContext *dsp);
void ff_vorbisdsp_init_arm(VorbisDSPContext *dsp);
void ff_vorbisdsp_init_ppc(VorbisDSPContext *dsp);

#endif 
