






















#if !defined(AVCODEC_H264_H)
#define AVCODEC_H264_H

#define QP_MAX_NUM (51 + 6*6) 





enum {
H264_NAL_UNSPECIFIED = 0,
H264_NAL_SLICE = 1,
H264_NAL_DPA = 2,
H264_NAL_DPB = 3,
H264_NAL_DPC = 4,
H264_NAL_IDR_SLICE = 5,
H264_NAL_SEI = 6,
H264_NAL_SPS = 7,
H264_NAL_PPS = 8,
H264_NAL_AUD = 9,
H264_NAL_END_SEQUENCE = 10,
H264_NAL_END_STREAM = 11,
H264_NAL_FILLER_DATA = 12,
H264_NAL_SPS_EXT = 13,
H264_NAL_PREFIX = 14,
H264_NAL_SUB_SPS = 15,
H264_NAL_DPS = 16,
H264_NAL_RESERVED17 = 17,
H264_NAL_RESERVED18 = 18,
H264_NAL_AUXILIARY_SLICE = 19,
H264_NAL_EXTEN_SLICE = 20,
H264_NAL_DEPTH_EXTEN_SLICE = 21,
H264_NAL_RESERVED22 = 22,
H264_NAL_RESERVED23 = 23,
H264_NAL_UNSPECIFIED24 = 24,
H264_NAL_UNSPECIFIED25 = 25,
H264_NAL_UNSPECIFIED26 = 26,
H264_NAL_UNSPECIFIED27 = 27,
H264_NAL_UNSPECIFIED28 = 28,
H264_NAL_UNSPECIFIED29 = 29,
H264_NAL_UNSPECIFIED30 = 30,
H264_NAL_UNSPECIFIED31 = 31,
};


enum {

H264_MAX_SPS_COUNT = 32,

H264_MAX_PPS_COUNT = 256,


H264_MAX_DPB_FRAMES = 16,


H264_MAX_REFS = 2 * H264_MAX_DPB_FRAMES,




H264_MAX_RPLM_COUNT = H264_MAX_REFS + 1,







H264_MAX_MMCO_COUNT = H264_MAX_REFS * 2 + 3,



H264_MAX_SLICE_GROUPS = 8,


H264_MAX_CPB_CNT = 32,


H264_MAX_MB_PIC_SIZE = 139264,



H264_MAX_MB_WIDTH = 1055,
H264_MAX_MB_HEIGHT = 1055,
H264_MAX_WIDTH = H264_MAX_MB_WIDTH * 16,
H264_MAX_HEIGHT = H264_MAX_MB_HEIGHT * 16,
};


#endif 
