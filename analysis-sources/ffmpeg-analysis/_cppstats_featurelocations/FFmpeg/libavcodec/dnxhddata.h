




















#if !defined(AVCODEC_DNXHDDATA_H)
#define AVCODEC_DNXHDDATA_H

#include <stdint.h>
#include "avcodec.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"


#define DNXHD_INTERLACED (1<<0)
#define DNXHD_MBAFF (1<<1)
#define DNXHD_444 (1<<2)


#define DNXHD_HEADER_INITIAL 0x000002800100
#define DNXHD_HEADER_444 0x000002800200


#define DNXHD_VARIABLE 0

typedef struct CIDEntry {
int cid;
unsigned int width, height;
unsigned int frame_size;
unsigned int coding_unit_size;
uint16_t flags;
int index_bits;
int bit_depth;
int eob_index;
const uint8_t *luma_weight, *chroma_weight;
const uint8_t *dc_codes, *dc_bits;
const uint16_t *ac_codes;
const uint8_t *ac_bits, *ac_info;
const uint16_t *run_codes;
const uint8_t *run_bits, *run;
int bit_rates[5]; 
AVRational packet_scale;
} CIDEntry;

extern const CIDEntry ff_dnxhd_cid_table[];

int ff_dnxhd_get_cid_table(int cid);
int ff_dnxhd_find_cid(AVCodecContext *avctx, int bit_depth);
void ff_dnxhd_print_profiles(AVCodecContext *avctx, int loglevel);

static av_always_inline uint64_t ff_dnxhd_check_header_prefix_hr(uint64_t prefix)
{
uint64_t data_offset = prefix >> 16;
if ((prefix & 0xFFFF0000FFFFLL) == 0x0300 &&
data_offset >= 0x0280 && data_offset <= 0x2170 &&
(data_offset & 3) == 0)
return prefix;
return 0;
}

static av_always_inline uint64_t ff_dnxhd_check_header_prefix(uint64_t prefix)
{
if (prefix == DNXHD_HEADER_INITIAL ||
prefix == DNXHD_HEADER_444 ||
ff_dnxhd_check_header_prefix_hr(prefix))
return prefix;
return 0;
}

static av_always_inline uint64_t ff_dnxhd_parse_header_prefix(const uint8_t *buf)
{
uint64_t prefix = AV_RB32(buf);
prefix = (prefix << 16) | buf[4] << 8;
return ff_dnxhd_check_header_prefix(prefix);
}

int avpriv_dnxhd_get_frame_size(int cid);
int avpriv_dnxhd_get_hr_frame_size(int cid, int w, int h);
int avpriv_dnxhd_get_interlaced(int cid);

#endif 
