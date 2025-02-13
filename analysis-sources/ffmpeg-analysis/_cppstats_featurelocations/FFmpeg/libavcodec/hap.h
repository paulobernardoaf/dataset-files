





















#if !defined(AVCODEC_HAP_H)
#define AVCODEC_HAP_H

#include <stdint.h>

#include "libavutil/opt.h"

#include "bytestream.h"
#include "texturedsp.h"

enum HapTextureFormat {
HAP_FMT_RGBDXT1 = 0x0B,
HAP_FMT_RGBADXT5 = 0x0E,
HAP_FMT_YCOCGDXT5 = 0x0F,
HAP_FMT_RGTC1 = 0x01,
};

enum HapCompressor {
HAP_COMP_NONE = 0xA0,
HAP_COMP_SNAPPY = 0xB0,
HAP_COMP_COMPLEX = 0xC0,
};

enum HapSectionType {
HAP_ST_DECODE_INSTRUCTIONS = 0x01,
HAP_ST_COMPRESSOR_TABLE = 0x02,
HAP_ST_SIZE_TABLE = 0x03,
HAP_ST_OFFSET_TABLE = 0x04,
};

typedef struct HapChunk {
enum HapCompressor compressor;
int compressed_offset;
size_t compressed_size;
int uncompressed_offset;
size_t uncompressed_size;
} HapChunk;

typedef struct HapContext {
AVClass *class;

TextureDSPContext dxtc;
GetByteContext gbc;

enum HapTextureFormat opt_tex_fmt; 
int opt_chunk_count; 
int opt_compressor; 

int chunk_count;
HapChunk *chunks;
int *chunk_results; 

int tex_rat; 
int tex_rat2; 
const uint8_t *tex_data; 
uint8_t *tex_buf; 
size_t tex_size; 

size_t max_snappy; 

int slice_count; 

int texture_count; 
int texture_section_size; 
int uncompress_pix_size; 


int (*tex_fun)(uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
int (*tex_fun2)(uint8_t *dst, ptrdiff_t stride, const uint8_t *block);
} HapContext;






int ff_hap_set_chunk_count(HapContext *ctx, int count, int first_in_frame);




av_cold void ff_hap_free_context(HapContext *ctx);




int ff_hap_parse_section_header(GetByteContext *gbc, int *section_size,
enum HapSectionType *section_type);

#endif 
