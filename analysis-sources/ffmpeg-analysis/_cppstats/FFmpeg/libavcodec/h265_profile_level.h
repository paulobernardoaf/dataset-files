#include <stdint.h>
#include "cbs_h265.h"
typedef struct H265LevelDescriptor {
const char *name;
uint8_t level_idc;
uint32_t max_luma_ps;
uint32_t max_cpb_main;
uint32_t max_cpb_high;
uint16_t max_slice_segments_per_picture;
uint8_t max_tile_rows;
uint8_t max_tile_cols;
uint32_t max_luma_sr;
uint32_t max_br_main;
uint32_t max_br_high;
uint8_t min_cr_base_main;
uint8_t min_cr_base_high;
} H265LevelDescriptor;
typedef struct H265ProfileDescriptor {
const char *name;
uint8_t profile_idc;
uint8_t high_throughput;
uint8_t max_14bit;
uint8_t max_12bit;
uint8_t max_10bit;
uint8_t max_8bit;
uint8_t max_422chroma;
uint8_t max_420chroma;
uint8_t max_monochrome;
uint8_t intra;
uint8_t one_picture_only;
uint8_t lower_bit_rate;
uint16_t cpb_vcl_factor;
uint16_t cpb_nal_factor;
float format_capability_factor;
float min_cr_scale_factor;
uint8_t max_dpb_pic_buf;
} H265ProfileDescriptor;
const H265LevelDescriptor *ff_h265_get_level(int level_idc);
const H265ProfileDescriptor *ff_h265_get_profile(const H265RawProfileTierLevel *ptl);
const H265LevelDescriptor *ff_h265_guess_level(const H265RawProfileTierLevel *ptl,
int64_t bitrate,
int width, int height,
int slice_segments,
int tile_rows, int tile_cols,
int max_dec_pic_buffering);
