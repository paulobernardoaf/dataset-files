




















#if !defined(AVCODEC_AC3TAB_H)
#define AVCODEC_AC3TAB_H

#include <stdint.h>

#include "libavutil/internal.h"
#include "ac3.h"
#include "internal.h"

extern const uint16_t ff_ac3_frame_size_tab[38][3];
extern const uint8_t ff_ac3_channels_tab[8];
extern av_export_avcodec const uint16_t avpriv_ac3_channel_layout_tab[8];
extern const uint8_t ff_ac3_enc_channel_map[8][2][6];
extern const uint8_t ff_ac3_dec_channel_map[8][2][6];
extern const uint16_t ff_ac3_sample_rate_tab[3];
extern const uint16_t ff_ac3_bitrate_tab[19];
extern const uint8_t ff_ac3_rematrix_band_tab[5];
extern const uint8_t ff_eac3_default_cpl_band_struct[18];
extern const int16_t ff_ac3_window[AC3_WINDOW_SIZE/2];
extern const uint8_t ff_ac3_log_add_tab[260];
extern const uint16_t ff_ac3_hearing_threshold_tab[AC3_CRITICAL_BANDS][3];
extern const uint8_t ff_ac3_bap_tab[64];
extern const uint8_t ff_ac3_slow_decay_tab[4];
extern const uint8_t ff_ac3_fast_decay_tab[4];
extern const uint16_t ff_ac3_slow_gain_tab[4];
extern const uint16_t ff_ac3_db_per_bit_tab[4];
extern const int16_t ff_ac3_floor_tab[8];
extern const uint16_t ff_ac3_fast_gain_tab[8];
extern const uint16_t ff_eac3_default_chmap[8];
extern const uint8_t ff_ac3_band_start_tab[AC3_CRITICAL_BANDS+1];
extern const uint8_t ff_ac3_bin_to_band_tab[253];
extern const uint64_t ff_eac3_custom_channel_map_locations[16][2];







enum CustomChannelMapLocation{
AC3_CHMAP_L= 1<<(15-0),
AC3_CHMAP_C= 1<<(15-1),
AC3_CHMAP_R= 1<<(15-2),
AC3_CHMAP_L_SUR= 1<<(15-3),
AC3_CHMAP_R_SUR = 1<<(15-4),
AC3_CHMAP_C_SUR= 1<<(15-7),
AC3_CHMAP_LFE = 1<<(15-15)
};

#endif 
