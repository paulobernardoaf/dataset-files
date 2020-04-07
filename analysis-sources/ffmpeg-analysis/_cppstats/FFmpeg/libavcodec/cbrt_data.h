#include <stdint.h>
#include "config.h"
#if CONFIG_HARDCODED_TABLES
#define ff_cbrt_tableinit_fixed()
#define ff_cbrt_tableinit()
extern const uint32_t ff_cbrt_tab[1 << 13];
extern const uint32_t ff_cbrt_tab_fixed[1 << 13];
#else
void ff_cbrt_tableinit(void);
void ff_cbrt_tableinit_fixed(void);
extern uint32_t ff_cbrt_tab[1 << 13];
extern uint32_t ff_cbrt_tab_fixed[1 << 13];
#endif
