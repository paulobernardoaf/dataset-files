































#if !defined(AVCODEC_MJPEGENC_H)
#define AVCODEC_MJPEGENC_H

#include <stdint.h>

#include "mjpeg.h"
#include "mpegvideo.h"
#include "put_bits.h"








typedef struct MJpegHuffmanCode {

uint8_t table_id; 
uint8_t code; 
uint16_t mant; 
} MJpegHuffmanCode;




typedef struct MJpegContext {

uint8_t huff_size_dc_luminance[12]; 
uint16_t huff_code_dc_luminance[12]; 
uint8_t huff_size_dc_chrominance[12]; 
uint16_t huff_code_dc_chrominance[12]; 

uint8_t huff_size_ac_luminance[256]; 
uint16_t huff_code_ac_luminance[256]; 
uint8_t huff_size_ac_chrominance[256]; 
uint16_t huff_code_ac_chrominance[256]; 


uint8_t uni_ac_vlc_len[64 * 64 * 2];

uint8_t uni_chroma_ac_vlc_len[64 * 64 * 2];


uint8_t bits_dc_luminance[17]; 
uint8_t val_dc_luminance[12]; 
uint8_t bits_dc_chrominance[17]; 
uint8_t val_dc_chrominance[12]; 


uint8_t bits_ac_luminance[17]; 
uint8_t val_ac_luminance[256]; 
uint8_t bits_ac_chrominance[17]; 
uint8_t val_ac_chrominance[256]; 

size_t huff_ncode; 
MJpegHuffmanCode *huff_buffer; 
} MJpegContext;




enum HuffmanTableOption {
HUFFMAN_TABLE_DEFAULT = 0, 
HUFFMAN_TABLE_OPTIMAL = 1, 
NB_HUFFMAN_TABLE_OPTION = 2
};

static inline void put_marker(PutBitContext *p, enum JpegMarker code)
{
put_bits(p, 8, 0xff);
put_bits(p, 8, code);
}

int ff_mjpeg_encode_init(MpegEncContext *s);
void ff_mjpeg_encode_close(MpegEncContext *s);
void ff_mjpeg_encode_mb(MpegEncContext *s, int16_t block[12][64]);

#endif 
