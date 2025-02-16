





















#if !defined(AVCODEC_AMR_H)
#define AVCODEC_AMR_H

#include <string.h>

#include "avcodec.h"

#if defined(AMR_USE_16BIT_TABLES)
typedef uint16_t R_TABLE_TYPE;
#else
typedef uint8_t R_TABLE_TYPE;
#endif
















static inline void ff_amr_bit_reorder(uint16_t *out, int size,
const uint8_t *data,
const R_TABLE_TYPE *ord_table)
{
int field_size;

memset(out, 0, size);
while ((field_size = *ord_table++)) {
int field = 0;
int field_offset = *ord_table++;
while (field_size--) {
int bit = *ord_table++;
field <<= 1;
field |= data[bit >> 3] >> (bit & 7) & 1;
}
out[field_offset >> 1] = field;
}
}

#endif 
