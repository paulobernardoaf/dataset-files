


























#if !defined(AVCODEC_ELSDEC_H)
#define AVCODEC_ELSDEC_H

#include <stdint.h>
#include <sys/types.h>

#define ELS_EXPGOLOMB_LEN 10

typedef struct ElsDecCtx {
const uint8_t *in_buf;
unsigned x;
size_t data_size;
int j, t, diff, err;
} ElsDecCtx;

typedef struct ElsRungNode {
uint8_t rung;
uint16_t next_index;
} ElsRungNode;

typedef struct ElsUnsignedRung {
uint8_t prefix_rung[ELS_EXPGOLOMB_LEN + 1];
ElsRungNode *rem_rung_list;
size_t rung_list_size;
uint16_t avail_index;
} ElsUnsignedRung;

void ff_els_decoder_init(ElsDecCtx *ctx, const uint8_t *in, size_t data_size);
void ff_els_decoder_uninit(ElsUnsignedRung *rung);
int ff_els_decode_bit(ElsDecCtx *ctx, unsigned char *rung);
unsigned ff_els_decode_unsigned(ElsDecCtx *ctx, ElsUnsignedRung *ur);

#endif 
