



























#include "libavutil/attributes.h"
#include "avcodec.h"
#include "mpegaudiodsp.h"
#include "mpegaudio.h"

#include "mpc.h"
#include "mpcdata.h"

av_cold void ff_mpc_init(void)
{
ff_mpa_synth_init_fixed(ff_mpa_synth_window_fixed);
}




static void mpc_synth(MPCContext *c, int16_t **out, int channels)
{
int dither_state = 0;
int i, ch;

for(ch = 0; ch < channels; ch++){
for(i = 0; i < SAMPLES_PER_BAND; i++) {
ff_mpa_synth_filter_fixed(&c->mpadsp,
c->synth_buf[ch], &(c->synth_buf_offset[ch]),
ff_mpa_synth_window_fixed, &dither_state,
out[ch] + 32 * i, 1,
c->sb_samples[ch][i]);
}
}
}

void ff_mpc_dequantize_and_synth(MPCContext * c, int maxband, int16_t **out,
int channels)
{
int i, j, ch;
Band *bands = c->bands;
int off;
float mul;


memset(c->sb_samples, 0, sizeof(c->sb_samples));
off = 0;
for(i = 0; i <= maxband; i++, off += SAMPLES_PER_BAND){
for(ch = 0; ch < 2; ch++){
if(bands[i].res[ch]){
j = 0;
mul = (mpc_CC+1)[bands[i].res[ch]] * mpc_SCF[bands[i].scf_idx[ch][0] & 0xFF];
for(; j < 12; j++)
c->sb_samples[ch][j][i] = mul * c->Q[ch][j + off];
mul = (mpc_CC+1)[bands[i].res[ch]] * mpc_SCF[bands[i].scf_idx[ch][1] & 0xFF];
for(; j < 24; j++)
c->sb_samples[ch][j][i] = mul * c->Q[ch][j + off];
mul = (mpc_CC+1)[bands[i].res[ch]] * mpc_SCF[bands[i].scf_idx[ch][2] & 0xFF];
for(; j < 36; j++)
c->sb_samples[ch][j][i] = mul * c->Q[ch][j + off];
}
}
if(bands[i].msf){
int t1, t2;
for(j = 0; j < SAMPLES_PER_BAND; j++){
t1 = c->sb_samples[0][j][i];
t2 = c->sb_samples[1][j][i];
c->sb_samples[0][j][i] = t1 + t2;
c->sb_samples[1][j][i] = t1 - t2;
}
}
}

mpc_synth(c, out, channels);
}
