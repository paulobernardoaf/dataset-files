#include "blockdsp.h"
#include "get_bits.h"
#include "idctdsp.h"
#include "intrax8dsp.h"
#include "wmv2dsp.h"
#include "mpegpicture.h"
typedef struct IntraX8Context {
VLC *j_ac_vlc[4]; 
VLC *j_orient_vlc;
VLC *j_dc_vlc[3];
int use_quant_matrix;
uint8_t *prediction_table; 
ScanTable scantable[3];
WMV2DSPContext wdsp;
uint8_t idct_permutation[64];
AVCodecContext *avctx;
int *block_last_index; 
int16_t (*block)[64];
IntraX8DSPContext dsp;
IDCTDSPContext idsp;
BlockDSPContext bdsp;
int quant;
int dquant;
int qsum;
int loopfilter;
AVFrame *frame;
GetBitContext *gb;
int quant_dc_chroma;
int divide_quant_dc_luma;
int divide_quant_dc_chroma;
uint8_t *dest[3];
uint8_t scratchpad[42]; 
int edges;
int flat_dc;
int predicted_dc;
int raw_orient;
int chroma_orient;
int orient;
int est_run;
int mb_x, mb_y;
int mb_width, mb_height;
} IntraX8Context;
int ff_intrax8_common_init(AVCodecContext *avctx,
IntraX8Context *w, IDCTDSPContext *idsp,
int16_t (*block)[64],
int block_last_index[12],
int mb_width, int mb_height);
void ff_intrax8_common_end(IntraX8Context *w);
int ff_intrax8_decode_picture(IntraX8Context *w, Picture *pict,
GetBitContext *gb, int *mb_x, int *mb_y,
int quant, int halfpq,
int loopfilter, int lowdelay);
