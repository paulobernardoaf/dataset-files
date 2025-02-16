



















#include "libavutil/attributes.h"

#include "wma_common.h"








av_cold int ff_wma_get_frame_len_bits(int sample_rate, int version,
unsigned int decode_flags)
{
int frame_len_bits;

if (sample_rate <= 16000)
frame_len_bits = 9;
else if (sample_rate <= 22050 || (sample_rate <= 32000 && version == 1))
frame_len_bits = 10;
else if (sample_rate <= 48000 || version < 3)
frame_len_bits = 11;
else if (sample_rate <= 96000)
frame_len_bits = 12;
else
frame_len_bits = 13;

if (version == 3) {
int tmp = decode_flags & 0x6;
if (tmp == 0x2)
++frame_len_bits;
else if (tmp == 0x4)
--frame_len_bits;
else if (tmp == 0x6)
frame_len_bits -= 2;
}

return frame_len_bits;
}
