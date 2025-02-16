#include <stddef.h>
#include <stdint.h>
enum ATAAlgorithm {
PARALLEL,
SERIAL,
NB_ATAA
};
typedef struct ATADenoiseDSPContext {
void (*filter_row)(const uint8_t *src, uint8_t *dst,
const uint8_t **srcf,
int w, int mid, int size,
int thra, int thrb);
} ATADenoiseDSPContext;
void ff_atadenoise_init_x86(ATADenoiseDSPContext *dsp, int depth, int algorithm);
