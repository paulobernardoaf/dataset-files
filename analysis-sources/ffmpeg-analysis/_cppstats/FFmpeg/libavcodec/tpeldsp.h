#include <stdint.h>
typedef void (*tpel_mc_func)(uint8_t *block ,
const uint8_t *pixels ,
int line_size, int w, int h);
typedef struct TpelDSPContext {
tpel_mc_func put_tpel_pixels_tab[11]; 
tpel_mc_func avg_tpel_pixels_tab[11]; 
} TpelDSPContext;
void ff_tpeldsp_init(TpelDSPContext *c);
