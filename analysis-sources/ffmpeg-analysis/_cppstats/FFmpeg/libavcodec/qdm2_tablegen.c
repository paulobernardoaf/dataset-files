#include <stdlib.h>
#include "tableprint_vlc.h"
#define CONFIG_HARDCODED_TABLES 0
#include "qdm2_tablegen.h"
int main(void)
{
softclip_table_init();
rnd_table_init();
init_noise_samples();
write_fileheader();
WRITE_ARRAY("static const", uint16_t, softclip_table);
WRITE_ARRAY("static const", float, noise_table);
WRITE_ARRAY("static const", float, noise_samples);
WRITE_2D_ARRAY("static const", uint8_t, random_dequant_index);
WRITE_2D_ARRAY("static const", uint8_t, random_dequant_type24);
qdm2_init_vlc();
WRITE_2D_ARRAY("static const", VLC_TYPE, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_level, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_diff, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_run, qdm2_table);
WRITE_VLC_TYPE("static const", fft_level_exp_alt_vlc, qdm2_table);
WRITE_VLC_TYPE("static const", fft_level_exp_vlc, qdm2_table);
WRITE_VLC_TYPE("static const", fft_stereo_exp_vlc, qdm2_table);
WRITE_VLC_TYPE("static const", fft_stereo_phase_vlc, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_tone_level_idx_hi1, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_tone_level_idx_mid, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_tone_level_idx_hi2, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_type30, qdm2_table);
WRITE_VLC_TYPE("static const", vlc_tab_type34, qdm2_table);
WRITE_VLC_ARRAY("static const", vlc_tab_fft_tone_offset, qdm2_table);
return 0;
}
