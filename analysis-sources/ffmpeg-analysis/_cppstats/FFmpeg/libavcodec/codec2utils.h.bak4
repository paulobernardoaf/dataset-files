




















#if !defined(AVCODEC_CODEC2UTILS_H)
#define AVCODEC_CODEC2UTILS_H

#include <stdint.h>




#define AVPRIV_CODEC2_MODE_MAX 8 





#define AVPRIV_CODEC2_AVOPTIONS(desc, classname, min_val, default_val, option_flags) { "mode", desc, offsetof(classname, mode), AV_OPT_TYPE_INT, {.i64 = default_val}, min_val, AVPRIV_CODEC2_MODE_MAX, .flags=option_flags, .unit="codec2_mode"},{ "3200", "3200", 0, AV_OPT_TYPE_CONST, {.i64 = 0}, .flags=option_flags, .unit="codec2_mode"},{ "2400", "2400", 0, AV_OPT_TYPE_CONST, {.i64 = 1}, .flags=option_flags, .unit="codec2_mode"},{ "1600", "1600", 0, AV_OPT_TYPE_CONST, {.i64 = 2}, .flags=option_flags, .unit="codec2_mode"},{ "1400", "1400", 0, AV_OPT_TYPE_CONST, {.i64 = 3}, .flags=option_flags, .unit="codec2_mode"},{ "1300", "1300", 0, AV_OPT_TYPE_CONST, {.i64 = 4}, .flags=option_flags, .unit="codec2_mode"},{ "1200", "1200", 0, AV_OPT_TYPE_CONST, {.i64 = 5}, .flags=option_flags, .unit="codec2_mode"},{ "700", "700", 0, AV_OPT_TYPE_CONST, {.i64 = 6}, .flags=option_flags, .unit="codec2_mode"},{ "700B", "700B", 0, AV_OPT_TYPE_CONST, {.i64 = 7}, .flags=option_flags, .unit="codec2_mode"},{ "700C", "700C", 0, AV_OPT_TYPE_CONST, {.i64 = 8}, .flags=option_flags, .unit="codec2_mode"}

















int avpriv_codec2_mode_bit_rate(void *logctx, int mode);


int avpriv_codec2_mode_frame_size(void *logctx, int mode);


int avpriv_codec2_mode_block_align(void *logctx, int mode);

#define AVPRIV_CODEC2_EXTRADATA_SIZE 4


static inline void avpriv_codec2_make_extradata(uint8_t *ptr, int mode) {

ptr[0] = 0; 
ptr[1] = 8; 
ptr[2] = mode; 
ptr[3] = 0; 
}


static inline uint16_t avpriv_codec2_version_from_extradata(uint8_t *ptr) {
return (ptr[0] << 8) + ptr[1];
}

static inline uint8_t avpriv_codec2_mode_from_extradata(uint8_t *ptr) {
return ptr[2];
}

#endif 
