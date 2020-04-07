#include <stdint.h>
typedef struct TTAEncDSPContext {
void (*filter_process)(int32_t *qm, int32_t *dx, int32_t *dl,
int32_t *error, int32_t *in, int32_t shift,
int32_t round);
} TTAEncDSPContext;
void ff_ttaencdsp_init(TTAEncDSPContext *c);
void ff_ttaencdsp_init_x86(TTAEncDSPContext *c);
