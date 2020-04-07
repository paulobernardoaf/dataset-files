
















#pragma once

#include "../util/c99defs.h"

#if defined(__cplusplus)
extern "C" {
#endif





EXPORT void compress_uyvx_to_i420(const uint8_t *input, uint32_t in_linesize,
uint32_t start_y, uint32_t end_y,
uint8_t *output[],
const uint32_t out_linesize[]);

EXPORT void compress_uyvx_to_nv12(const uint8_t *input, uint32_t in_linesize,
uint32_t start_y, uint32_t end_y,
uint8_t *output[],
const uint32_t out_linesize[]);

EXPORT void convert_uyvx_to_i444(const uint8_t *input, uint32_t in_linesize,
uint32_t start_y, uint32_t end_y,
uint8_t *output[],
const uint32_t out_linesize[]);

EXPORT void decompress_nv12(const uint8_t *const input[],
const uint32_t in_linesize[], uint32_t start_y,
uint32_t end_y, uint8_t *output,
uint32_t out_linesize);

EXPORT void decompress_420(const uint8_t *const input[],
const uint32_t in_linesize[], uint32_t start_y,
uint32_t end_y, uint8_t *output,
uint32_t out_linesize);

EXPORT void decompress_422(const uint8_t *input, uint32_t in_linesize,
uint32_t start_y, uint32_t end_y, uint8_t *output,
uint32_t out_linesize, bool leading_lum);

#if defined(__cplusplus)
}
#endif
