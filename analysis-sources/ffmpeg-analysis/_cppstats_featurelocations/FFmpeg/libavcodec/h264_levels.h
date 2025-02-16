

















#if !defined(AVCODEC_H264_LEVELS_H)
#define AVCODEC_H264_LEVELS_H


#include <stdint.h>

typedef struct H264LevelDescriptor {
const char *name;
uint8_t level_idc;
uint8_t constraint_set3_flag;
uint32_t max_mbps;
uint32_t max_fs;
uint32_t max_dpb_mbs;
uint32_t max_br;
uint32_t max_cpb;
uint16_t max_v_mv_r;
uint8_t min_cr;
uint8_t max_mvs_per_2mb;
} H264LevelDescriptor;

const H264LevelDescriptor *ff_h264_get_level(int level_idc,
int constraint_set3_flag);






const H264LevelDescriptor *ff_h264_guess_level(int profile_idc,
int64_t bitrate,
int framerate,
int width, int height,
int max_dec_frame_buffering);


#endif 
