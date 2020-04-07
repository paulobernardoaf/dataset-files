



















#if !defined(AVCODEC_AV1_PARSE_H)
#define AVCODEC_AV1_PARSE_H

#include <stdint.h>

#include "av1.h"
#include "avcodec.h"
#include "get_bits.h"

typedef struct AV1OBU {

int size;
const uint8_t *data;





int size_bits;


int raw_size;
const uint8_t *raw_data;


GetBitContext gb;

int type;

int temporal_id;
int spatial_id;
} AV1OBU;


typedef struct AV1Packet {
AV1OBU *obus;
int nb_obus;
int obus_allocated;
unsigned obus_allocated_size;
} AV1Packet;








int ff_av1_extract_obu(AV1OBU *obu, const uint8_t *buf, int length,
void *logctx);








int ff_av1_packet_split(AV1Packet *pkt, const uint8_t *buf, int length,
void *logctx);




void ff_av1_packet_uninit(AV1Packet *pkt);

static inline int64_t leb128(GetBitContext *gb) {
int64_t ret = 0;
int i;

for (i = 0; i < 8; i++) {
int byte = get_bits(gb, 8);
ret |= (int64_t)(byte & 0x7f) << (i * 7);
if (!(byte & 0x80))
break;
}
return ret;
}

static inline int parse_obu_header(const uint8_t *buf, int buf_size,
int64_t *obu_size, int *start_pos, int *type,
int *temporal_id, int *spatial_id)
{
GetBitContext gb;
int ret, extension_flag, has_size_flag;
int64_t size;

ret = init_get_bits8(&gb, buf, FFMIN(buf_size, 2 + 8)); 
if (ret < 0)
return ret;

if (get_bits1(&gb) != 0) 
return AVERROR_INVALIDDATA;

*type = get_bits(&gb, 4);
extension_flag = get_bits1(&gb);
has_size_flag = get_bits1(&gb);
skip_bits1(&gb); 

if (extension_flag) {
*temporal_id = get_bits(&gb, 3);
*spatial_id = get_bits(&gb, 2);
skip_bits(&gb, 3); 
} else {
*temporal_id = *spatial_id = 0;
}

*obu_size = has_size_flag ? leb128(&gb)
: buf_size - 1 - extension_flag;

if (get_bits_left(&gb) < 0)
return AVERROR_INVALIDDATA;

*start_pos = get_bits_count(&gb) / 8;

size = *obu_size + *start_pos;

if (size > buf_size)
return AVERROR_INVALIDDATA;

return size;
}

static inline int get_obu_bit_length(const uint8_t *buf, int size, int type)
{
int v;


if (type == AV1_OBU_TILE_GROUP ||
type == AV1_OBU_TILE_LIST ||
type == AV1_OBU_FRAME) {
if (size > INT_MAX / 8)
return AVERROR(ERANGE);
else
return size * 8;
}

while (size > 0 && buf[size - 1] == 0)
size--;

if (!size)
return 0;

v = buf[size - 1];

if (size > INT_MAX / 8)
return AVERROR(ERANGE);
size *= 8;


if (v)
size -= ff_ctz(v) + 1;

return size;
}

#endif 
