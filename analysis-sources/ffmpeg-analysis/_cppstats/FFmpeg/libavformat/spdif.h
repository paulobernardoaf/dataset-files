#include <stdint.h>
#include "avformat.h"
#define SYNCWORD1 0xF872
#define SYNCWORD2 0x4E1F
#define BURST_HEADER_SIZE 0x8
enum IEC61937DataType {
IEC61937_AC3 = 0x01, 
IEC61937_MPEG1_LAYER1 = 0x04, 
IEC61937_MPEG1_LAYER23 = 0x05, 
IEC61937_MPEG2_EXT = 0x06, 
IEC61937_MPEG2_AAC = 0x07, 
IEC61937_MPEG2_LAYER1_LSF = 0x08, 
IEC61937_MPEG2_LAYER2_LSF = 0x09, 
IEC61937_MPEG2_LAYER3_LSF = 0x0A, 
IEC61937_DTS1 = 0x0B, 
IEC61937_DTS2 = 0x0C, 
IEC61937_DTS3 = 0x0D, 
IEC61937_ATRAC = 0x0E, 
IEC61937_ATRAC3 = 0x0F, 
IEC61937_ATRACX = 0x10, 
IEC61937_DTSHD = 0x11, 
IEC61937_WMAPRO = 0x12, 
IEC61937_MPEG2_AAC_LSF_2048 = 0x13, 
IEC61937_MPEG2_AAC_LSF_4096 = 0x13 | 0x20, 
IEC61937_EAC3 = 0x15, 
IEC61937_TRUEHD = 0x16, 
};
static const uint16_t spdif_mpeg_pkt_offset[2][3] = {
{ 3072, 9216, 4608 }, 
{ 1536, 4608, 4608 }, 
};
void ff_spdif_bswap_buf16(uint16_t *dst, const uint16_t *src, int w);
int ff_spdif_read_packet(AVFormatContext *s, AVPacket *pkt);
int ff_spdif_probe(const uint8_t *p_buf, int buf_size, enum AVCodecID *codec);
