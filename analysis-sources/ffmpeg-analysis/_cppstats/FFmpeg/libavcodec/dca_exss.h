#include "libavutil/common.h"
#include "avcodec.h"
#include "get_bits.h"
typedef struct DCAExssAsset {
int asset_offset; 
int asset_size; 
int asset_index; 
int pcm_bit_res; 
int max_sample_rate; 
int nchannels_total; 
int one_to_one_map_ch_to_spkr; 
int embedded_stereo; 
int embedded_6ch; 
int spkr_mask_enabled; 
int spkr_mask; 
int representation_type; 
int coding_mode; 
int extension_mask; 
int core_offset; 
int core_size; 
int xbr_offset; 
int xbr_size; 
int xxch_offset; 
int xxch_size; 
int x96_offset; 
int x96_size; 
int lbr_offset; 
int lbr_size; 
int xll_offset; 
int xll_size; 
int xll_sync_present; 
int xll_delay_nframes; 
int xll_sync_offset; 
int hd_stream_id; 
} DCAExssAsset;
typedef struct DCAExssParser {
AVCodecContext *avctx;
GetBitContext gb;
int exss_index; 
int exss_size_nbits; 
int exss_size; 
int static_fields_present; 
int npresents; 
int nassets; 
int mix_metadata_enabled; 
int nmixoutconfigs; 
int nmixoutchs[4]; 
DCAExssAsset assets[1]; 
} DCAExssParser;
int ff_dca_exss_parse(DCAExssParser *s, const uint8_t *data, int size);
