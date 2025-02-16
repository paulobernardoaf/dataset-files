



















#if !defined(AVCODEC_H2645_PARSE_H)
#define AVCODEC_H2645_PARSE_H

#include <stdint.h>

#include "libavutil/buffer.h"
#include "avcodec.h"
#include "get_bits.h"

#define MAX_MBPAIR_SIZE (256*1024) 

typedef struct H2645NAL {
uint8_t *rbsp_buffer;

int size;
const uint8_t *data;





int size_bits;

int raw_size;
const uint8_t *raw_data;

GetBitContext gb;




int type;




int temporal_id;




int nuh_layer_id;

int skipped_bytes;
int skipped_bytes_pos_size;
int *skipped_bytes_pos;



int ref_idc;
} H2645NAL;

typedef struct H2645RBSP {
uint8_t *rbsp_buffer;
AVBufferRef *rbsp_buffer_ref;
int rbsp_buffer_alloc_size;
int rbsp_buffer_size;
} H2645RBSP;


typedef struct H2645Packet {
H2645NAL *nals;
H2645RBSP rbsp;
int nb_nals;
int nals_allocated;
unsigned nal_buffer_size;
} H2645Packet;




int ff_h2645_extract_rbsp(const uint8_t *src, int length, H2645RBSP *rbsp,
H2645NAL *nal, int small_padding);















int ff_h2645_packet_split(H2645Packet *pkt, const uint8_t *buf, int length,
void *logctx, int is_nalff, int nal_length_size,
enum AVCodecID codec_id, int small_padding, int use_ref);




void ff_h2645_packet_uninit(H2645Packet *pkt);

static inline int get_nalsize(int nal_length_size, const uint8_t *buf,
int buf_size, int *buf_index, void *logctx)
{
int i, nalsize = 0;

if (*buf_index >= buf_size - nal_length_size) {

return AVERROR(EAGAIN);
}

for (i = 0; i < nal_length_size; i++)
nalsize = ((unsigned)nalsize << 8) | buf[(*buf_index)++];
if (nalsize <= 0 || nalsize > buf_size - *buf_index) {
av_log(logctx, AV_LOG_ERROR,
"Invalid NAL unit size (%d > %d).\n", nalsize, buf_size - *buf_index);
return AVERROR_INVALIDDATA;
}
return nalsize;
}

#endif 
