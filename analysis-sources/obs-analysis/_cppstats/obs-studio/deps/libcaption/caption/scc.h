#if defined(__cplusplus)
extern "C" {
#endif
#include "eia608.h"
typedef struct _scc_t {
double timestamp;
unsigned int cc_aloc;
unsigned int cc_size;
uint16_t cc_data[];
} scc_t;
scc_t* scc_new(int cc_count);
scc_t* scc_free(scc_t* scc);
size_t scc_to_608(scc_t** scc, const utf8_char_t* data);
#if defined(__cplusplus)
}
#endif
