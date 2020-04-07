






















#include "avformat.h"
#include "internal.h"
#include "isom.h"
#include "libavcodec/mpeg4audio.h"
#include "libavcodec/mpegaudiodata.h"
#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"



const AVCodecTag ff_mp4_obj_type[] = {
{ AV_CODEC_ID_MOV_TEXT , 0x08 },
{ AV_CODEC_ID_MPEG4 , 0x20 },
{ AV_CODEC_ID_H264 , 0x21 },
{ AV_CODEC_ID_HEVC , 0x23 },
{ AV_CODEC_ID_AAC , 0x40 },
{ AV_CODEC_ID_MP4ALS , 0x40 }, 
{ AV_CODEC_ID_MPEG2VIDEO , 0x61 }, 
{ AV_CODEC_ID_MPEG2VIDEO , 0x60 }, 
{ AV_CODEC_ID_MPEG2VIDEO , 0x62 }, 
{ AV_CODEC_ID_MPEG2VIDEO , 0x63 }, 
{ AV_CODEC_ID_MPEG2VIDEO , 0x64 }, 
{ AV_CODEC_ID_MPEG2VIDEO , 0x65 }, 
{ AV_CODEC_ID_AAC , 0x66 }, 
{ AV_CODEC_ID_AAC , 0x67 }, 
{ AV_CODEC_ID_AAC , 0x68 }, 
{ AV_CODEC_ID_MP3 , 0x69 }, 
{ AV_CODEC_ID_MP2 , 0x69 }, 
{ AV_CODEC_ID_MPEG1VIDEO , 0x6A }, 
{ AV_CODEC_ID_MP3 , 0x6B }, 
{ AV_CODEC_ID_MJPEG , 0x6C }, 
{ AV_CODEC_ID_PNG , 0x6D },
{ AV_CODEC_ID_JPEG2000 , 0x6E }, 
{ AV_CODEC_ID_VC1 , 0xA3 },
{ AV_CODEC_ID_DIRAC , 0xA4 },
{ AV_CODEC_ID_AC3 , 0xA5 },
{ AV_CODEC_ID_EAC3 , 0xA6 },
{ AV_CODEC_ID_DTS , 0xA9 }, 
{ AV_CODEC_ID_OPUS , 0xAD }, 
{ AV_CODEC_ID_VP9 , 0xB1 }, 
{ AV_CODEC_ID_FLAC , 0xC1 }, 
{ AV_CODEC_ID_TSCC2 , 0xD0 }, 
{ AV_CODEC_ID_EVRC , 0xD1 }, 
{ AV_CODEC_ID_VORBIS , 0xDD }, 
{ AV_CODEC_ID_DVD_SUBTITLE, 0xE0 }, 
{ AV_CODEC_ID_QCELP , 0xE1 },
{ AV_CODEC_ID_MPEG4SYSTEMS, 0x01 },
{ AV_CODEC_ID_MPEG4SYSTEMS, 0x02 },
{ AV_CODEC_ID_NONE , 0 },
};

const AVCodecTag ff_codec_movvideo_tags[] = {


{ AV_CODEC_ID_RAWVIDEO, MKTAG('r', 'a', 'w', ' ') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('y', 'u', 'v', '2') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('2', 'v', 'u', 'y') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('y', 'u', 'v', 's') }, 

{ AV_CODEC_ID_RAWVIDEO, MKTAG('L', '5', '5', '5') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('L', '5', '6', '5') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('B', '5', '6', '5') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('2', '4', 'B', 'G') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('B', 'G', 'R', 'A') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', 'G', 'B', 'A') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('A', 'B', 'G', 'R') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('b', '1', '6', 'g') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('b', '4', '8', 'r') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('b', '6', '4', 'a') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('b', 'x', 'b', 'g') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('b', 'x', 'r', 'g') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('b', 'x', 'y', 'v') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('N', 'O', '1', '6') },
{ AV_CODEC_ID_RAWVIDEO, MKTAG('D', 'V', 'O', 'O') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', '4', '2', '0') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('R', '4', '1', '1') }, 

{ AV_CODEC_ID_R10K, MKTAG('R', '1', '0', 'k') }, 
{ AV_CODEC_ID_R10K, MKTAG('R', '1', '0', 'g') }, 
{ AV_CODEC_ID_R210, MKTAG('r', '2', '1', '0') }, 
{ AV_CODEC_ID_AVUI, MKTAG('A', 'V', 'U', 'I') }, 
{ AV_CODEC_ID_AVRP, MKTAG('A', 'V', 'r', 'p') }, 
{ AV_CODEC_ID_AVRP, MKTAG('S', 'U', 'D', 'S') }, 
{ AV_CODEC_ID_V210, MKTAG('v', '2', '1', '0') }, 
{ AV_CODEC_ID_V210, MKTAG('b', 'x', 'y', '2') }, 
{ AV_CODEC_ID_V308, MKTAG('v', '3', '0', '8') }, 
{ AV_CODEC_ID_V408, MKTAG('v', '4', '0', '8') }, 
{ AV_CODEC_ID_V410, MKTAG('v', '4', '1', '0') }, 
{ AV_CODEC_ID_Y41P, MKTAG('Y', '4', '1', 'P') }, 
{ AV_CODEC_ID_YUV4, MKTAG('y', 'u', 'v', '4') }, 
{ AV_CODEC_ID_TARGA_Y216, MKTAG('Y', '2', '1', '6') },

{ AV_CODEC_ID_MJPEG, MKTAG('j', 'p', 'e', 'g') }, 
{ AV_CODEC_ID_MJPEG, MKTAG('m', 'j', 'p', 'a') }, 
{ AV_CODEC_ID_AVRN , MKTAG('A', 'V', 'D', 'J') }, 

{ AV_CODEC_ID_MJPEG, MKTAG('d', 'm', 'b', '1') }, 
{ AV_CODEC_ID_MJPEGB, MKTAG('m', 'j', 'p', 'b') }, 

{ AV_CODEC_ID_SVQ1, MKTAG('S', 'V', 'Q', '1') }, 
{ AV_CODEC_ID_SVQ1, MKTAG('s', 'v', 'q', '1') }, 
{ AV_CODEC_ID_SVQ1, MKTAG('s', 'v', 'q', 'i') }, 
{ AV_CODEC_ID_SVQ3, MKTAG('S', 'V', 'Q', '3') }, 

{ AV_CODEC_ID_MPEG4, MKTAG('m', 'p', '4', 'v') },
{ AV_CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', 'X') }, 
{ AV_CODEC_ID_MPEG4, MKTAG('X', 'V', 'I', 'D') },
{ AV_CODEC_ID_MPEG4, MKTAG('3', 'I', 'V', '2') }, 

{ AV_CODEC_ID_H263, MKTAG('h', '2', '6', '3') }, 
{ AV_CODEC_ID_H263, MKTAG('s', '2', '6', '3') }, 

{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'c', 'p') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'c', ' ') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'p', 'p') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', '5', 'p') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', '5', 'n') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('A', 'V', 'd', 'v') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('A', 'V', 'd', '1') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', 'q') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', 'p') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '1') },
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '2') },
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '4') },
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '5') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '6') }, 
{ AV_CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', '3') }, 

{ AV_CODEC_ID_VP3, MKTAG('V', 'P', '3', '1') }, 
{ AV_CODEC_ID_RPZA, MKTAG('r', 'p', 'z', 'a') }, 
{ AV_CODEC_ID_CINEPAK, MKTAG('c', 'v', 'i', 'd') }, 
{ AV_CODEC_ID_8BPS, MKTAG('8', 'B', 'P', 'S') }, 
{ AV_CODEC_ID_SMC, MKTAG('s', 'm', 'c', ' ') }, 
{ AV_CODEC_ID_QTRLE, MKTAG('r', 'l', 'e', ' ') }, 
{ AV_CODEC_ID_SGIRLE, MKTAG('r', 'l', 'e', '1') }, 
{ AV_CODEC_ID_MSRLE, MKTAG('W', 'R', 'L', 'E') },
{ AV_CODEC_ID_QDRAW, MKTAG('q', 'd', 'r', 'w') }, 
{ AV_CODEC_ID_CDTOONS, MKTAG('Q', 'k', 'B', 'k') }, 

{ AV_CODEC_ID_RAWVIDEO, MKTAG('W', 'R', 'A', 'W') },

{ AV_CODEC_ID_HEVC, MKTAG('h', 'e', 'v', '1') }, 
{ AV_CODEC_ID_HEVC, MKTAG('h', 'v', 'c', '1') }, 
{ AV_CODEC_ID_HEVC, MKTAG('d', 'v', 'h', 'e') }, 


{ AV_CODEC_ID_H264, MKTAG('a', 'v', 'c', '1') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'v', 'c', '2') },
{ AV_CODEC_ID_H264, MKTAG('a', 'v', 'c', '3') },
{ AV_CODEC_ID_H264, MKTAG('a', 'v', 'c', '4') },
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '5', 'p') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '5', 'q') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '2') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '3') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '5') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '5', '6') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '1', 'p') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '1', 'q') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '2') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '3') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '5') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', '1', '6') }, 
{ AV_CODEC_ID_H264, MKTAG('A', 'V', 'i', 'n') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'i', 'v', 'x') }, 
{ AV_CODEC_ID_H264, MKTAG('r', 'v', '6', '4') }, 
{ AV_CODEC_ID_H264, MKTAG('x', 'a', 'l', 'g') }, 
{ AV_CODEC_ID_H264, MKTAG('a', 'v', 'l', 'g') }, 
{ AV_CODEC_ID_H264, MKTAG('d', 'v', 'a', '1') }, 
{ AV_CODEC_ID_H264, MKTAG('d', 'v', 'a', 'v') }, 

{ AV_CODEC_ID_VP8, MKTAG('v', 'p', '0', '8') }, 
{ AV_CODEC_ID_VP9, MKTAG('v', 'p', '0', '9') }, 
{ AV_CODEC_ID_AV1, MKTAG('a', 'v', '0', '1') }, 

{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('m', '1', 'v', ' ') },
{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('m', '1', 'v', '1') }, 
{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('m', 'p', 'e', 'g') }, 
{ AV_CODEC_ID_MPEG1VIDEO, MKTAG('m', 'p', '1', 'v') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', '2', 'v', '1') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '1') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '2') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '3') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '4') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '5') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '6') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '7') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '8') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', '9') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('h', 'd', 'v', 'a') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'x', '5', 'n') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'x', '5', 'p') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'x', '4', 'n') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'x', '4', 'p') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'x', '3', 'n') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'x', '3', 'p') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', '1') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', '4') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', '5') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', '9') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', 'a') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', 'b') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', 'c') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', 'd') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', 'e') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', '5', 'f') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '1') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '2') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '3') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '4') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '5') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '6') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '7') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '8') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', '9') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', 'a') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', 'b') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', 'c') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', 'd') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', 'e') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'v', 'f') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'h', 'd') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('x', 'd', 'h', '2') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('A', 'V', 'm', 'p') }, 
{ AV_CODEC_ID_MPEG2VIDEO, MKTAG('m', 'p', '2', 'v') }, 

{ AV_CODEC_ID_JPEG2000, MKTAG('m', 'j', 'p', '2') }, 

{ AV_CODEC_ID_TARGA, MKTAG('t', 'g', 'a', ' ') }, 
{ AV_CODEC_ID_TIFF, MKTAG('t', 'i', 'f', 'f') }, 
{ AV_CODEC_ID_GIF, MKTAG('g', 'i', 'f', ' ') }, 
{ AV_CODEC_ID_PNG, MKTAG('p', 'n', 'g', ' ') },
{ AV_CODEC_ID_PNG, MKTAG('M', 'N', 'G', ' ') },

{ AV_CODEC_ID_VC1, MKTAG('v', 'c', '-', '1') }, 
{ AV_CODEC_ID_CAVS, MKTAG('a', 'v', 's', '2') },

{ AV_CODEC_ID_DIRAC, MKTAG('d', 'r', 'a', 'c') },
{ AV_CODEC_ID_DNXHD, MKTAG('A', 'V', 'd', 'n') }, 
{ AV_CODEC_ID_DNXHD, MKTAG('A', 'V', 'd', 'h') }, 
{ AV_CODEC_ID_H263, MKTAG('H', '2', '6', '3') },
{ AV_CODEC_ID_MSMPEG4V3, MKTAG('3', 'I', 'V', 'D') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('A', 'V', '1', 'x') }, 
{ AV_CODEC_ID_RAWVIDEO, MKTAG('A', 'V', 'u', 'p') },
{ AV_CODEC_ID_SGI, MKTAG('s', 'g', 'i', ' ') }, 
{ AV_CODEC_ID_DPX, MKTAG('d', 'p', 'x', ' ') }, 
{ AV_CODEC_ID_EXR, MKTAG('e', 'x', 'r', ' ') }, 

{ AV_CODEC_ID_PRORES, MKTAG('a', 'p', 'c', 'h') }, 
{ AV_CODEC_ID_PRORES, MKTAG('a', 'p', 'c', 'n') }, 
{ AV_CODEC_ID_PRORES, MKTAG('a', 'p', 'c', 's') }, 
{ AV_CODEC_ID_PRORES, MKTAG('a', 'p', 'c', 'o') }, 
{ AV_CODEC_ID_PRORES, MKTAG('a', 'p', '4', 'h') }, 
{ AV_CODEC_ID_PRORES, MKTAG('a', 'p', '4', 'x') }, 
{ AV_CODEC_ID_FLIC, MKTAG('f', 'l', 'i', 'c') },

{ AV_CODEC_ID_AIC, MKTAG('i', 'c', 'o', 'd') },

{ AV_CODEC_ID_HAP, MKTAG('H', 'a', 'p', '1') },
{ AV_CODEC_ID_HAP, MKTAG('H', 'a', 'p', '5') },
{ AV_CODEC_ID_HAP, MKTAG('H', 'a', 'p', 'Y') },
{ AV_CODEC_ID_HAP, MKTAG('H', 'a', 'p', 'A') },
{ AV_CODEC_ID_HAP, MKTAG('H', 'a', 'p', 'M') },

{ AV_CODEC_ID_DXV, MKTAG('D', 'X', 'D', '3') },
{ AV_CODEC_ID_DXV, MKTAG('D', 'X', 'D', 'I') },

{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'R', '0') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'R', 'A') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'R', 'G') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'Y', '2') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '0', 'Y', '4') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'R', 'G') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'R', 'A') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'G', '0') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', '0') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', '2') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', '4') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '8', 'Y', 'A') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '2', 'R', 'A') },
{ AV_CODEC_ID_MAGICYUV, MKTAG('M', '2', 'R', 'G') },

{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '0') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '1') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '2') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '3') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '4') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '5') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '6') },
{ AV_CODEC_ID_SHEERVIDEO, MKTAG('S', 'h', 'r', '7') },

{ AV_CODEC_ID_PIXLET, MKTAG('p', 'x', 'l', 't') },

{ AV_CODEC_ID_NONE, 0 },
};

const AVCodecTag ff_codec_movaudio_tags[] = {
{ AV_CODEC_ID_AAC, MKTAG('m', 'p', '4', 'a') },
{ AV_CODEC_ID_AC3, MKTAG('a', 'c', '-', '3') }, 
{ AV_CODEC_ID_AC3, MKTAG('s', 'a', 'c', '3') }, 
{ AV_CODEC_ID_ADPCM_IMA_QT, MKTAG('i', 'm', 'a', '4') },
{ AV_CODEC_ID_ALAC, MKTAG('a', 'l', 'a', 'c') },
{ AV_CODEC_ID_AMR_NB, MKTAG('s', 'a', 'm', 'r') }, 
{ AV_CODEC_ID_AMR_WB, MKTAG('s', 'a', 'w', 'b') }, 
{ AV_CODEC_ID_DTS, MKTAG('d', 't', 's', 'c') }, 
{ AV_CODEC_ID_DTS, MKTAG('d', 't', 's', 'h') }, 
{ AV_CODEC_ID_DTS, MKTAG('d', 't', 's', 'l') }, 
{ AV_CODEC_ID_DTS, MKTAG('d', 't', 's', 'e') }, 
{ AV_CODEC_ID_DTS, MKTAG('D', 'T', 'S', ' ') }, 
{ AV_CODEC_ID_EAC3, MKTAG('e', 'c', '-', '3') }, 
{ AV_CODEC_ID_DVAUDIO, MKTAG('v', 'd', 'v', 'a') },
{ AV_CODEC_ID_DVAUDIO, MKTAG('d', 'v', 'c', 'a') },
{ AV_CODEC_ID_GSM, MKTAG('a', 'g', 's', 'm') },
{ AV_CODEC_ID_ILBC, MKTAG('i', 'l', 'b', 'c') },
{ AV_CODEC_ID_MACE3, MKTAG('M', 'A', 'C', '3') },
{ AV_CODEC_ID_MACE6, MKTAG('M', 'A', 'C', '6') },
{ AV_CODEC_ID_MP1, MKTAG('.', 'm', 'p', '1') },
{ AV_CODEC_ID_MP2, MKTAG('.', 'm', 'p', '2') },
{ AV_CODEC_ID_MP3, MKTAG('.', 'm', 'p', '3') },
{ AV_CODEC_ID_MP3, MKTAG('m', 'p', '3', ' ') }, 
{ AV_CODEC_ID_MP3, 0x6D730055 },
{ AV_CODEC_ID_NELLYMOSER, MKTAG('n', 'm', 'o', 's') }, 
{ AV_CODEC_ID_NELLYMOSER, MKTAG('N', 'E', 'L', 'L') }, 
{ AV_CODEC_ID_PCM_ALAW, MKTAG('a', 'l', 'a', 'w') },
{ AV_CODEC_ID_PCM_F32BE, MKTAG('f', 'l', '3', '2') },
{ AV_CODEC_ID_PCM_F32LE, MKTAG('f', 'l', '3', '2') },
{ AV_CODEC_ID_PCM_F64BE, MKTAG('f', 'l', '6', '4') },
{ AV_CODEC_ID_PCM_F64LE, MKTAG('f', 'l', '6', '4') },
{ AV_CODEC_ID_PCM_MULAW, MKTAG('u', 'l', 'a', 'w') },
{ AV_CODEC_ID_PCM_S16BE, MKTAG('t', 'w', 'o', 's') },
{ AV_CODEC_ID_PCM_S16LE, MKTAG('s', 'o', 'w', 't') },
{ AV_CODEC_ID_PCM_S16BE, MKTAG('l', 'p', 'c', 'm') },
{ AV_CODEC_ID_PCM_S16LE, MKTAG('l', 'p', 'c', 'm') },
{ AV_CODEC_ID_PCM_S24BE, MKTAG('i', 'n', '2', '4') },
{ AV_CODEC_ID_PCM_S24LE, MKTAG('i', 'n', '2', '4') },
{ AV_CODEC_ID_PCM_S32BE, MKTAG('i', 'n', '3', '2') },
{ AV_CODEC_ID_PCM_S32LE, MKTAG('i', 'n', '3', '2') },
{ AV_CODEC_ID_PCM_S8, MKTAG('s', 'o', 'w', 't') },
{ AV_CODEC_ID_PCM_U8, MKTAG('r', 'a', 'w', ' ') },
{ AV_CODEC_ID_PCM_U8, MKTAG('N', 'O', 'N', 'E') },
{ AV_CODEC_ID_QCELP, MKTAG('Q', 'c', 'l', 'p') },
{ AV_CODEC_ID_QCELP, MKTAG('Q', 'c', 'l', 'q') },
{ AV_CODEC_ID_QCELP, MKTAG('s', 'q', 'c', 'p') }, 
{ AV_CODEC_ID_QDM2, MKTAG('Q', 'D', 'M', '2') },
{ AV_CODEC_ID_QDMC, MKTAG('Q', 'D', 'M', 'C') },
{ AV_CODEC_ID_SPEEX, MKTAG('s', 'p', 'e', 'x') }, 
{ AV_CODEC_ID_SPEEX, MKTAG('S', 'P', 'X', 'N') }, 
{ AV_CODEC_ID_EVRC, MKTAG('s', 'e', 'v', 'c') }, 
{ AV_CODEC_ID_SMV, MKTAG('s', 's', 'm', 'v') }, 
{ AV_CODEC_ID_FLAC, MKTAG('f', 'L', 'a', 'C') }, 
{ AV_CODEC_ID_TRUEHD, MKTAG('m', 'l', 'p', 'a') }, 
{ AV_CODEC_ID_OPUS, MKTAG('O', 'p', 'u', 's') }, 
{ AV_CODEC_ID_MPEGH_3D_AUDIO, MKTAG('m', 'h', 'm', '1') }, 
{ AV_CODEC_ID_NONE, 0 },
};

const AVCodecTag ff_codec_movsubtitle_tags[] = {
{ AV_CODEC_ID_MOV_TEXT, MKTAG('t', 'e', 'x', 't') },
{ AV_CODEC_ID_MOV_TEXT, MKTAG('t', 'x', '3', 'g') },
{ AV_CODEC_ID_EIA_608, MKTAG('c', '6', '0', '8') },
{ AV_CODEC_ID_NONE, 0 },
};

const AVCodecTag ff_codec_movdata_tags[] = {
{ AV_CODEC_ID_BIN_DATA, MKTAG('g', 'p', 'm', 'd') },
{ AV_CODEC_ID_NONE, 0 },
};





static const char mov_mdhd_language_map[][4] = {

"eng", "fra", "ger", "ita", "dut", "sve", "spa", "dan", "por", "nor",
"heb", "jpn", "ara", "fin", "gre", "ice", "mlt", "tur", "hr ", "chi",
"urd", "hin", "tha", "kor", "lit", "pol", "hun", "est", "lav", "",
"fo ", "", "rus", "chi", "", "iri", "alb", "ron", "ces", "slk",
"slv", "yid", "sr ", "mac", "bul", "ukr", "bel", "uzb", "kaz", "aze",

"aze", "arm", "geo", "mol", "kir", "tgk", "tuk", "mon", "", "pus",
"kur", "kas", "snd", "tib", "nep", "san", "mar", "ben", "asm", "guj",
"pa ", "ori", "mal", "kan", "tam", "tel", "", "bur", "khm", "lao",

"vie", "ind", "tgl", "may", "may", "amh", "tir", "orm", "som", "swa",

"", "run", "", "mlg", "epo", "", "", "", "", "",

"", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "wel", "baq",
"cat", "lat", "que", "grn", "aym", "tat", "uig", "dzo", "jav"
};

int ff_mov_iso639_to_lang(const char lang[4], int mp4)
{
int i, code = 0;


for (i = 0; lang[0] && !mp4 && i < FF_ARRAY_ELEMS(mov_mdhd_language_map); i++) {
if (!strcmp(lang, mov_mdhd_language_map[i]))
return i;
}

if (!mp4)
return -1;

if (lang[0] == '\0')
lang = "und";

for (i = 0; i < 3; i++) {
uint8_t c = lang[i];
c -= 0x60;
if (c > 0x1f)
return -1;
code <<= 5;
code |= c;
}
return code;
}

int ff_mov_lang_to_iso639(unsigned code, char to[4])
{
int i;
memset(to, 0, 4);


if (code >= 0x400 && code != 0x7fff) {
for (i = 2; i >= 0; i--) {
to[i] = 0x60 + (code & 0x1f);
code >>= 5;
}
return 1;
}

if (code >= FF_ARRAY_ELEMS(mov_mdhd_language_map))
return 0;
if (!mov_mdhd_language_map[code][0])
return 0;
memcpy(to, mov_mdhd_language_map[code], 4);
return 1;
}

int ff_mp4_read_descr_len(AVIOContext *pb)
{
int len = 0;
int count = 4;
while (count--) {
int c = avio_r8(pb);
len = (len << 7) | (c & 0x7f);
if (!(c & 0x80))
break;
}
return len;
}

int ff_mp4_read_descr(AVFormatContext *fc, AVIOContext *pb, int *tag)
{
int len;
*tag = avio_r8(pb);
len = ff_mp4_read_descr_len(pb);
av_log(fc, AV_LOG_TRACE, "MPEG-4 description: tag=0x%02x len=%d\n", *tag, len);
return len;
}

void ff_mp4_parse_es_descr(AVIOContext *pb, int *es_id)
{
int flags;
if (es_id) *es_id = avio_rb16(pb);
else avio_rb16(pb);
flags = avio_r8(pb);
if (flags & 0x80) 
avio_rb16(pb);
if (flags & 0x40) { 
int len = avio_r8(pb);
avio_skip(pb, len);
}
if (flags & 0x20) 
avio_rb16(pb);
}

static const AVCodecTag mp4_audio_types[] = {
{ AV_CODEC_ID_MP3ON4, AOT_PS }, 
{ AV_CODEC_ID_MP3ON4, AOT_L1 }, 
{ AV_CODEC_ID_MP3ON4, AOT_L2 }, 
{ AV_CODEC_ID_MP3ON4, AOT_L3 }, 
{ AV_CODEC_ID_MP4ALS, AOT_ALS }, 
{ AV_CODEC_ID_NONE, AOT_NULL },
};

int ff_mp4_read_dec_config_descr(AVFormatContext *fc, AVStream *st, AVIOContext *pb)
{
enum AVCodecID codec_id;
unsigned v;
int len, tag;
int ret;
int object_type_id = avio_r8(pb);
avio_r8(pb); 
avio_rb24(pb); 

v = avio_rb32(pb);


#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
if (v < INT32_MAX)
st->codec->rc_max_rate = v;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

st->codecpar->bit_rate = avio_rb32(pb); 

codec_id= ff_codec_get_id(ff_mp4_obj_type, object_type_id);
if (codec_id)
st->codecpar->codec_id = codec_id;
av_log(fc, AV_LOG_TRACE, "esds object type id 0x%02x\n", object_type_id);
len = ff_mp4_read_descr(fc, pb, &tag);
if (tag == MP4DecSpecificDescrTag) {
av_log(fc, AV_LOG_TRACE, "Specific MPEG-4 header len=%d\n", len);


if (object_type_id == 0x69 || object_type_id == 0x6b)
return 0;
if (!len || (uint64_t)len > (1<<30))
return AVERROR_INVALIDDATA;
if ((ret = ff_get_extradata(fc, st->codecpar, pb, len)) < 0)
return ret;
if (st->codecpar->codec_id == AV_CODEC_ID_AAC) {
MPEG4AudioConfig cfg = {0};
ret = avpriv_mpeg4audio_get_config2(&cfg, st->codecpar->extradata,
st->codecpar->extradata_size, 1, fc);
if (ret < 0)
return ret;
st->codecpar->channels = cfg.channels;
if (cfg.object_type == 29 && cfg.sampling_index < 3) 
st->codecpar->sample_rate = avpriv_mpa_freq_tab[cfg.sampling_index];
else if (cfg.ext_sample_rate)
st->codecpar->sample_rate = cfg.ext_sample_rate;
else
st->codecpar->sample_rate = cfg.sample_rate;
av_log(fc, AV_LOG_TRACE, "mp4a config channels %d obj %d ext obj %d "
"sample rate %d ext sample rate %d\n", st->codecpar->channels,
cfg.object_type, cfg.ext_object_type,
cfg.sample_rate, cfg.ext_sample_rate);
if (!(st->codecpar->codec_id = ff_codec_get_id(mp4_audio_types,
cfg.object_type)))
st->codecpar->codec_id = AV_CODEC_ID_AAC;
}
}
return 0;
}

typedef struct MovChannelLayout {
int64_t channel_layout;
uint32_t layout_tag;
} MovChannelLayout;

static const MovChannelLayout mov_channel_layout[] = {
{ AV_CH_LAYOUT_MONO, (100<<16) | 1}, 
{ AV_CH_LAYOUT_STEREO, (101<<16) | 2}, 
{ AV_CH_LAYOUT_STEREO, (102<<16) | 2}, 
{ AV_CH_LAYOUT_2_1, (131<<16) | 3}, 
{ AV_CH_LAYOUT_QUAD, (132<<16) | 4}, 
{ AV_CH_LAYOUT_2_2, (132<<16) | 4}, 
{ AV_CH_LAYOUT_QUAD, (108<<16) | 4}, 
{ AV_CH_LAYOUT_SURROUND, (113<<16) | 3}, 
{ AV_CH_LAYOUT_4POINT0, (115<<16) | 4}, 
{ AV_CH_LAYOUT_5POINT0_BACK, (117<<16) | 5}, 
{ AV_CH_LAYOUT_5POINT0, (117<<16) | 5}, 
{ AV_CH_LAYOUT_5POINT1_BACK, (121<<16) | 6}, 
{ AV_CH_LAYOUT_5POINT1, (121<<16) | 6}, 
{ AV_CH_LAYOUT_7POINT1, (128<<16) | 8}, 
{ AV_CH_LAYOUT_7POINT1_WIDE, (126<<16) | 8}, 
{ AV_CH_LAYOUT_5POINT1_BACK|AV_CH_LAYOUT_STEREO_DOWNMIX, (130<<16) | 8}, 
{ AV_CH_LAYOUT_STEREO|AV_CH_LOW_FREQUENCY, (133<<16) | 3}, 
{ AV_CH_LAYOUT_2_1|AV_CH_LOW_FREQUENCY, (134<<16) | 4}, 
{ AV_CH_LAYOUT_QUAD|AV_CH_LOW_FREQUENCY, (135<<16) | 4}, 
{ AV_CH_LAYOUT_2_2|AV_CH_LOW_FREQUENCY, (135<<16) | 4}, 
{ AV_CH_LAYOUT_SURROUND|AV_CH_LOW_FREQUENCY, (136<<16) | 4}, 
{ AV_CH_LAYOUT_4POINT0|AV_CH_LOW_FREQUENCY, (137<<16) | 5}, 
{ 0, 0},
};
#if 0
int ff_mov_read_chan(AVFormatContext *s, AVStream *st, int64_t size)
{
AVCodecContext *codec= st->codec;
uint32_t layout_tag;
AVIOContext *pb = s->pb;
const MovChannelLayout *layouts = mov_channel_layout;

if (size < 12)
return AVERROR_INVALIDDATA;

layout_tag = avio_rb32(pb);
size -= 4;
if (layout_tag == 0) { 

av_log_ask_for_sample(s, "Unimplemented container channel layout.\n");
avio_skip(pb, size);
return 0;
}
if (layout_tag == 0x10000) { 
codec->channel_layout = avio_rb32(pb);
size -= 4;
avio_skip(pb, size);
return 0;
}
while (layouts->channel_layout) {
if (layout_tag == layouts->layout_tag) {
codec->channel_layout = layouts->channel_layout;
break;
}
layouts++;
}
if (!codec->channel_layout)
av_log(s, AV_LOG_WARNING, "Unknown container channel layout.\n");
avio_skip(pb, size);

return 0;
}
#endif

void ff_mov_write_chan(AVIOContext *pb, int64_t channel_layout)
{
const MovChannelLayout *layouts;
uint32_t layout_tag = 0;

for (layouts = mov_channel_layout; layouts->channel_layout; layouts++)
if (channel_layout == layouts->channel_layout) {
layout_tag = layouts->layout_tag;
break;
}

if (layout_tag) {
avio_wb32(pb, layout_tag); 
avio_wb32(pb, 0); 
} else {
avio_wb32(pb, 0x10000); 
avio_wb32(pb, channel_layout);
}
avio_wb32(pb, 0); 
}

const struct AVCodecTag *avformat_get_mov_video_tags(void)
{
return ff_codec_movvideo_tags;
}

const struct AVCodecTag *avformat_get_mov_audio_tags(void)
{
return ff_codec_movaudio_tags;
}
