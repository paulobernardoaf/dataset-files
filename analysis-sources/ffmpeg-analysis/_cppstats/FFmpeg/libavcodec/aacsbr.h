#include "get_bits.h"
#include "aac.h"
#include "sbr.h"
#define ENVELOPE_ADJUSTMENT_OFFSET 2
#define NOISE_FLOOR_OFFSET 6
enum {
T_HUFFMAN_ENV_1_5DB,
F_HUFFMAN_ENV_1_5DB,
T_HUFFMAN_ENV_BAL_1_5DB,
F_HUFFMAN_ENV_BAL_1_5DB,
T_HUFFMAN_ENV_3_0DB,
F_HUFFMAN_ENV_3_0DB,
T_HUFFMAN_ENV_BAL_3_0DB,
F_HUFFMAN_ENV_BAL_3_0DB,
T_HUFFMAN_NOISE_3_0DB,
T_HUFFMAN_NOISE_BAL_3_0DB,
};
enum {
FIXFIX,
FIXVAR,
VARFIX,
VARVAR,
};
enum {
EXTENSION_ID_PS = 2,
};
static const int8_t vlc_sbr_lav[10] =
{ 60, 60, 24, 24, 31, 31, 12, 12, 31, 12 };
#define SBR_INIT_VLC_STATIC(num, size) INIT_VLC_STATIC(&vlc_sbr[num], 9, sbr_tmp[num].table_size / sbr_tmp[num].elem_size, sbr_tmp[num].sbr_bits , 1, 1, sbr_tmp[num].sbr_codes, sbr_tmp[num].elem_size, sbr_tmp[num].elem_size, size)
#define SBR_VLC_ROW(name) { name ##_codes, name ##_bits, sizeof(name ##_codes), sizeof(name ##_codes[0]) }
void AAC_RENAME(ff_aac_sbr_init)(void);
void AAC_RENAME(ff_aac_sbr_ctx_init)(AACContext *ac, SpectralBandReplication *sbr, int id_aac);
void AAC_RENAME(ff_aac_sbr_ctx_close)(SpectralBandReplication *sbr);
int AAC_RENAME(ff_decode_sbr_extension)(AACContext *ac, SpectralBandReplication *sbr,
GetBitContext *gb, int crc, int cnt, int id_aac);
void AAC_RENAME(ff_sbr_apply)(AACContext *ac, SpectralBandReplication *sbr, int id_aac,
INTFLOAT* L, INTFLOAT *R);
void ff_aacsbr_func_ptr_init_mips(AACSBRContext *c);
