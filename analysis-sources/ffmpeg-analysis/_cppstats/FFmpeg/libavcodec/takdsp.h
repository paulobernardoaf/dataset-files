#include <stdint.h>
typedef struct TAKDSPContext {
void (*decorrelate_ls)(int32_t *p1, int32_t *p2, int length);
void (*decorrelate_sr)(int32_t *p1, int32_t *p2, int length);
void (*decorrelate_sm)(int32_t *p1, int32_t *p2, int length);
void (*decorrelate_sf)(int32_t *p1, int32_t *p2, int length, int dshift, int dfactor);
} TAKDSPContext;
void ff_takdsp_init(TAKDSPContext *c);
void ff_takdsp_init_x86(TAKDSPContext *c);
