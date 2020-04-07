



















#if !defined(AVCODEC_TTADATA_H)
#define AVCODEC_TTADATA_H

#include "internal.h"

#define MAX_ORDER 16
typedef struct TTAFilter {
int32_t shift, round, error;
int32_t qm[MAX_ORDER];
int32_t dx[MAX_ORDER];
int32_t dl[MAX_ORDER];
} TTAFilter;

typedef struct TTARice {
uint32_t k0, k1, sum0, sum1;
} TTARice;

typedef struct TTAChannel {
int32_t predictor;
TTAFilter filter;
TTARice rice;
} TTAChannel;

extern const uint32_t ff_tta_shift_1[];
extern const uint32_t * const ff_tta_shift_16;
extern const uint8_t ff_tta_filter_configs[];

void ff_tta_rice_init(TTARice *c, uint32_t k0, uint32_t k1);
void ff_tta_filter_init(TTAFilter *c, int32_t shift);
#endif 
