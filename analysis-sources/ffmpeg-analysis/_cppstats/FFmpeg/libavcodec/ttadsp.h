#include <stdint.h>
typedef struct TTADSPContext {
void (*filter_process)(int32_t *qm, int32_t *dx, int32_t *dl,
int32_t *error, int32_t *in, int32_t shift,
int32_t round);
} TTADSPContext;
void ff_ttadsp_init(TTADSPContext *c);
void ff_ttadsp_init_x86(TTADSPContext *c);
