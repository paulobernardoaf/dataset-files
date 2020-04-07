




















#include "libavresample/avresample.h"
#include "libavutil/aarch64/neontest.h"

wrap(avresample_convert(AVAudioResampleContext *avr, uint8_t **output,
int out_plane_size, int out_samples, uint8_t **input,
int in_plane_size, int in_samples))
{
testneonclobbers(avresample_convert, avr, output, out_plane_size,
out_samples, input, in_plane_size, in_samples);
}
