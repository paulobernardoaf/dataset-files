#include "libavutil/common.h"
#include "libavcodec/h265_profile_level.h"
static const struct {
int width;
int height;
int level_idc;
} test_sizes[] = {
{ 176, 144, 30 }, 
{ 352, 288, 60 }, 
{ 640, 480, 90 }, 
{ 720, 480, 90 }, 
{ 720, 576, 90 }, 
{ 1024, 768, 93 }, 
{ 1280, 720, 93 }, 
{ 1280, 1024, 120 }, 
{ 1920, 1080, 120 }, 
{ 2048, 1080, 120 }, 
{ 2048, 1536, 150 }, 
{ 3840, 2160, 150 }, 
{ 7680, 4320, 180 }, 
{ 1, 512, 30 },
{ 1, 1024, 63 },
{ 1, 2048, 90 },
{ 1, 4096, 120 },
{ 1, 8192, 150 },
{ 1, 16384, 180 },
{ 1, 32768, 0 },
{ 512, 1, 30 },
{ 1024, 1, 63 },
{ 2048, 1, 90 },
{ 4096, 1, 120 },
{ 8192, 1, 150 },
{ 16384, 1, 180 },
{ 32768, 1, 0 },
{ 2800, 256, 93 },
{ 2816, 128, 120 },
{ 256, 4208, 120 },
{ 128, 4224, 150 },
{ 8432, 256, 150 },
{ 8448, 128, 180 },
{ 256, 16880, 180 },
{ 128, 16896, 0 },
};
static const struct {
int width;
int height;
int dpb_size;
int level_idc;
} test_dpb[] = {
{ 176, 144, 8, 30 },
{ 176, 144, 9, 60 },
{ 352, 288, 6, 60 },
{ 352, 288, 7, 63 },
{ 352, 288, 13, 90 },
{ 1280, 720, 6, 93 },
{ 1280, 720, 12, 120 },
{ 1280, 720, 16, 150 },
{ 3840, 2160, 6, 150 },
{ 3840, 2160, 7, 180 },
{ 3840, 2160, 16, 180 },
};
static const H265RawProfileTierLevel profile_main = {
.general_profile_space = 0,
.general_profile_idc = 1,
.general_tier_flag = 0,
.general_profile_compatibility_flag[1] = 1,
};
static const H265RawProfileTierLevel profile_main_12 = {
.general_profile_space = 0,
.general_profile_idc = 4,
.general_tier_flag = 0,
.general_profile_compatibility_flag[4] = 1,
.general_max_12bit_constraint_flag = 1,
.general_max_10bit_constraint_flag = 0,
.general_max_8bit_constraint_flag = 0,
.general_max_422chroma_constraint_flag = 1,
.general_max_420chroma_constraint_flag = 1,
.general_max_monochrome_constraint_flag = 0,
.general_intra_constraint_flag = 0,
.general_one_picture_only_constraint_flag = 0,
.general_lower_bit_rate_constraint_flag = 1,
};
static const H265RawProfileTierLevel profile_main_422_12_intra = {
.general_profile_space = 0,
.general_profile_idc = 4,
.general_tier_flag = 0,
.general_profile_compatibility_flag[4] = 1,
.general_max_12bit_constraint_flag = 1,
.general_max_10bit_constraint_flag = 0,
.general_max_8bit_constraint_flag = 0,
.general_max_422chroma_constraint_flag = 1,
.general_max_420chroma_constraint_flag = 0,
.general_max_monochrome_constraint_flag = 0,
.general_intra_constraint_flag = 1,
.general_one_picture_only_constraint_flag = 0,
};
static const H265RawProfileTierLevel profile_ht_444_14 = {
.general_profile_space = 0,
.general_profile_idc = 5,
.general_tier_flag = 0,
.general_profile_compatibility_flag[5] = 1,
.general_max_14bit_constraint_flag = 1,
.general_max_12bit_constraint_flag = 0,
.general_max_10bit_constraint_flag = 0,
.general_max_8bit_constraint_flag = 0,
.general_max_422chroma_constraint_flag = 0,
.general_max_420chroma_constraint_flag = 0,
.general_max_monochrome_constraint_flag = 0,
.general_intra_constraint_flag = 0,
.general_one_picture_only_constraint_flag = 0,
.general_lower_bit_rate_constraint_flag = 1,
};
static const H265RawProfileTierLevel profile_main_high_tier = {
.general_profile_space = 0,
.general_profile_idc = 1,
.general_tier_flag = 1,
.general_profile_compatibility_flag[1] = 1,
};
static const struct {
int64_t bitrate;
const H265RawProfileTierLevel *ptl;
int level_idc;
} test_bitrate[] = {
{ 4000000, &profile_main, 90 },
{ 4000000, &profile_main_12, 63 },
{ 4000000, &profile_main_422_12_intra, 60 },
{ 50000000, &profile_main, 156 },
{ 50000000, &profile_main_12, 153 },
{ 50000000, &profile_main_422_12_intra, 120 },
{ 50000000, &profile_ht_444_14, 63 },
{ 1000, &profile_main, 30 },
{ 1000, &profile_main_high_tier, 120 },
{ 40000000, &profile_main, 153 },
{ 40000000, &profile_main_high_tier, 123 },
{ 200000000, &profile_main, 186 },
{ 200000000, &profile_main_high_tier, 156 },
{ INT64_C(2700000000), &profile_ht_444_14, 183 },
{ INT64_C(4200000000), &profile_ht_444_14, 186 },
{ INT64_C(5600000000), &profile_ht_444_14, 0 },
};
static const struct {
int slice_segments;
int tile_rows;
int tile_cols;
int level_idc;
} test_fragments[] = {
{ 4, 1, 1, 30 },
{ 32, 1, 1, 93 },
{ 70, 1, 1, 120 },
{ 80, 1, 1, 150 },
{ 201, 1, 1, 180 },
{ 600, 1, 1, 180 },
{ 601, 1, 1, 0 },
{ 1, 2, 1, 90 },
{ 1, 1, 2, 90 },
{ 1, 3, 3, 93 },
{ 1, 4, 2, 120 },
{ 1, 2, 4, 120 },
{ 1, 11, 10, 150 },
{ 1, 10, 11, 180 },
{ 1, 22, 20, 180 },
{ 1, 20, 22, 0 },
};
int main(void)
{
const H265ProfileDescriptor *profile;
const H265LevelDescriptor *level;
int i;
#define CHECK(expected, format, ...) do { if (expected ? (!level || level->level_idc != expected) : !!level) { av_log(NULL, AV_LOG_ERROR, "Incorrect level for " format ": expected %d, got %d.\n", __VA_ARGS__, expected, level ? level->level_idc : -1); return 1; } } while (0)
for (i = 0; i < FF_ARRAY_ELEMS(test_sizes); i++) {
level = ff_h265_guess_level(&profile_main, 0,
test_sizes[i].width,
test_sizes[i].height,
0, 0, 0, 0);
CHECK(test_sizes[i].level_idc, "size %dx%d",
test_sizes[i].width, test_sizes[i].height);
}
for (i = 0; i < FF_ARRAY_ELEMS(test_dpb); i++) {
level = ff_h265_guess_level(&profile_main, 0,
test_dpb[i].width,
test_dpb[i].height,
0, 0, 0, test_dpb[i].dpb_size);
CHECK(test_dpb[i].level_idc, "size %dx%d dpb %d",
test_dpb[i].width, test_dpb[i].height,
test_dpb[i].dpb_size);
}
for (i = 0; i < FF_ARRAY_ELEMS(test_bitrate); i++) {
profile = ff_h265_get_profile(test_bitrate[i].ptl);
level = ff_h265_guess_level(test_bitrate[i].ptl,
test_bitrate[i].bitrate,
0, 0, 0, 0, 0, 0);
CHECK(test_bitrate[i].level_idc, "bitrate %"PRId64" profile %s",
test_bitrate[i].bitrate, profile->name);
}
for (i = 0; i < FF_ARRAY_ELEMS(test_fragments); i++) {
level = ff_h265_guess_level(&profile_main, 0, 0, 0,
test_fragments[i].slice_segments,
test_fragments[i].tile_rows,
test_fragments[i].tile_cols, 0);
CHECK(test_fragments[i].level_idc, "%d slices %dx%d tiles",
test_fragments[i].slice_segments,
test_fragments[i].tile_cols, test_fragments[i].tile_rows);
}
return 0;
}
