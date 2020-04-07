#include <string.h>
#include "internal.h"
#include "libavcodec/codec2utils.h"
int avpriv_codec2_mode_bit_rate(void *logctx, int mode)
{
int frame_size = avpriv_codec2_mode_frame_size(logctx, mode);
int block_align = avpriv_codec2_mode_block_align(logctx, mode);
if (frame_size <= 0 || block_align <= 0) {
return 0;
}
return 8 * 8000 * block_align / frame_size;
}
int avpriv_codec2_mode_frame_size(void *logctx, int mode)
{
int frame_size_table[AVPRIV_CODEC2_MODE_MAX+1] = {
160, 
160, 
320, 
320, 
320, 
320, 
320, 
320, 
320, 
};
if (mode < 0 || mode > AVPRIV_CODEC2_MODE_MAX) {
av_log(logctx, AV_LOG_ERROR, "unknown codec2 mode %i, can't find frame_size\n", mode);
return 0;
} else {
return frame_size_table[mode];
}
}
int avpriv_codec2_mode_block_align(void *logctx, int mode)
{
int block_align_table[AVPRIV_CODEC2_MODE_MAX+1] = {
8, 
6, 
8, 
7, 
7, 
6, 
4, 
4, 
4, 
};
if (mode < 0 || mode > AVPRIV_CODEC2_MODE_MAX) {
av_log(logctx, AV_LOG_ERROR, "unknown codec2 mode %i, can't find block_align\n", mode);
return 0;
} else {
return block_align_table[mode];
}
}
