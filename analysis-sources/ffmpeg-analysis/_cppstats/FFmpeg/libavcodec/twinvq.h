#include <math.h>
#include <stdint.h>
#include "libavutil/common.h"
#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "fft.h"
#include "internal.h"
enum TwinVQCodec {
TWINVQ_CODEC_VQF,
TWINVQ_CODEC_METASOUND,
};
enum TwinVQFrameType {
TWINVQ_FT_SHORT = 0, 
TWINVQ_FT_MEDIUM, 
TWINVQ_FT_LONG, 
TWINVQ_FT_PPC, 
};
#define TWINVQ_PPC_SHAPE_CB_SIZE 64
#define TWINVQ_PPC_SHAPE_LEN_MAX 60
#define TWINVQ_SUB_AMP_MAX 4500.0
#define TWINVQ_MULAW_MU 100.0
#define TWINVQ_GAIN_BITS 8
#define TWINVQ_AMP_MAX 13000.0
#define TWINVQ_SUB_GAIN_BITS 5
#define TWINVQ_WINDOW_TYPE_BITS 4
#define TWINVQ_PGAIN_MU 200
#define TWINVQ_LSP_COEFS_MAX 20
#define TWINVQ_LSP_SPLIT_MAX 4
#define TWINVQ_CHANNELS_MAX 2
#define TWINVQ_SUBBLOCKS_MAX 16
#define TWINVQ_BARK_N_COEF_MAX 4
#define TWINVQ_MAX_FRAMES_PER_PACKET 2
struct TwinVQFrameMode {
uint8_t sub; 
const uint16_t *bark_tab;
uint8_t bark_env_size;
const int16_t *bark_cb; 
uint8_t bark_n_coef;
uint8_t bark_n_bit; 
const int16_t *cb0;
const int16_t *cb1;
uint8_t cb_len_read; 
};
typedef struct TwinVQFrameData {
int window_type;
enum TwinVQFrameType ftype;
uint8_t main_coeffs[1024];
uint8_t ppc_coeffs[TWINVQ_PPC_SHAPE_LEN_MAX];
uint8_t gain_bits[TWINVQ_CHANNELS_MAX];
uint8_t sub_gain_bits[TWINVQ_CHANNELS_MAX * TWINVQ_SUBBLOCKS_MAX];
uint8_t bark1[TWINVQ_CHANNELS_MAX][TWINVQ_SUBBLOCKS_MAX][TWINVQ_BARK_N_COEF_MAX];
uint8_t bark_use_hist[TWINVQ_CHANNELS_MAX][TWINVQ_SUBBLOCKS_MAX];
uint8_t lpc_idx1[TWINVQ_CHANNELS_MAX];
uint8_t lpc_idx2[TWINVQ_CHANNELS_MAX][TWINVQ_LSP_SPLIT_MAX];
uint8_t lpc_hist_idx[TWINVQ_CHANNELS_MAX];
int p_coef[TWINVQ_CHANNELS_MAX];
int g_coef[TWINVQ_CHANNELS_MAX];
} TwinVQFrameData;
typedef struct TwinVQModeTab {
struct TwinVQFrameMode fmode[3]; 
uint16_t size; 
uint8_t n_lsp; 
const float *lspcodebook;
uint8_t lsp_bit0;
uint8_t lsp_bit1;
uint8_t lsp_bit2;
uint8_t lsp_split; 
const int16_t *ppc_shape_cb; 
uint8_t ppc_period_bit;
uint8_t ppc_shape_bit; 
uint8_t ppc_shape_len; 
uint8_t pgain_bit; 
uint16_t peak_per2wid;
} TwinVQModeTab;
typedef struct TwinVQContext {
AVCodecContext *avctx;
AVFloatDSPContext *fdsp;
FFTContext mdct_ctx[3];
const TwinVQModeTab *mtab;
int is_6kbps;
float lsp_hist[2][20]; 
float bark_hist[3][2][40]; 
int16_t permut[4][4096];
uint8_t length[4][2]; 
uint8_t length_change[4];
uint8_t bits_main_spec[2][4][2]; 
int bits_main_spec_change[4];
int n_div[4];
float *spectrum;
float *curr_frame; 
float *prev_frame; 
int last_block_pos[2];
int discarded_packets;
float *cos_tabs[3];
float *tmp_buf;
int frame_size, frames_per_packet, cur_frame;
TwinVQFrameData bits[TWINVQ_MAX_FRAMES_PER_PACKET];
enum TwinVQCodec codec;
int (*read_bitstream)(AVCodecContext *avctx, struct TwinVQContext *tctx,
const uint8_t *buf, int buf_size);
void (*dec_bark_env)(struct TwinVQContext *tctx, const uint8_t *in,
int use_hist, int ch, float *out, float gain,
enum TwinVQFrameType ftype);
void (*decode_ppc)(struct TwinVQContext *tctx, int period_coef, int g_coef,
const float *shape, float *speech);
} TwinVQContext;
extern const enum TwinVQFrameType ff_twinvq_wtype_to_ftype_table[];
static inline void twinvq_memset_float(float *buf, float val, int size)
{
while (size--)
*buf++ = val;
}
static inline float twinvq_mulawinv(float y, float clip, float mu)
{
y = av_clipf(y / clip, -1, 1);
return clip * FFSIGN(y) * (exp(log(1 + mu) * fabs(y)) - 1) / mu;
}
int ff_twinvq_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt);
int ff_twinvq_decode_close(AVCodecContext *avctx);
int ff_twinvq_decode_init(AVCodecContext *avctx);
