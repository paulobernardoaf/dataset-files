






















#if !defined(AVCODEC_VP9_H)
#define AVCODEC_VP9_H

enum TxfmMode {
TX_4X4,
TX_8X8,
TX_16X16,
TX_32X32,
N_TXFM_SIZES,
TX_SWITCHABLE = N_TXFM_SIZES,
N_TXFM_MODES
};

enum TxfmType {
DCT_DCT,
DCT_ADST,
ADST_DCT,
ADST_ADST,
N_TXFM_TYPES
};

enum IntraPredMode {
VERT_PRED,
HOR_PRED,
DC_PRED,
DIAG_DOWN_LEFT_PRED,
DIAG_DOWN_RIGHT_PRED,
VERT_RIGHT_PRED,
HOR_DOWN_PRED,
VERT_LEFT_PRED,
HOR_UP_PRED,
TM_VP8_PRED,
LEFT_DC_PRED,
TOP_DC_PRED,
DC_128_PRED,
DC_127_PRED,
DC_129_PRED,
N_INTRA_PRED_MODES
};

enum FilterMode {
FILTER_8TAP_SMOOTH,
FILTER_8TAP_REGULAR,
FILTER_8TAP_SHARP,
FILTER_BILINEAR,
N_FILTERS,
FILTER_SWITCHABLE = N_FILTERS,
};

#endif 
