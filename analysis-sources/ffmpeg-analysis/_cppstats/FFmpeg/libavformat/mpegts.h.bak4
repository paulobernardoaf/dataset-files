




















#if !defined(AVFORMAT_MPEGTS_H)
#define AVFORMAT_MPEGTS_H

#include "avformat.h"

#define TS_FEC_PACKET_SIZE 204
#define TS_DVHS_PACKET_SIZE 192
#define TS_PACKET_SIZE 188
#define TS_MAX_PACKET_SIZE 204

#define NB_PID_MAX 8192
#define USUAL_SECTION_SIZE 1024 
#define MAX_SECTION_SIZE 4096


#define PAT_PID 0x0000 
#define CAT_PID 0x0001 
#define TSDT_PID 0x0002 
#define IPMP_PID 0x0003

#define NIT_PID 0x0010 
#define SDT_PID 0x0011 
#define BAT_PID 0x0011 
#define EIT_PID 0x0012 
#define RST_PID 0x0013 
#define TDT_PID 0x0014 
#define TOT_PID 0x0014
#define NET_SYNC_PID 0x0015
#define RNT_PID 0x0016 




#define LINK_LOCAL_PID 0x001C
#define MEASUREMENT_PID 0x001D
#define DIT_PID 0x001E 
#define SIT_PID 0x001F 


#define FIRST_OTHER_PID 0x0020
#define LAST_OTHER_PID 0x1FFA



#define NULL_PID 0x1FFF 


#define PAT_TID 0x00 
#define CAT_TID 0x01 
#define PMT_TID 0x02 
#define TSDT_TID 0x03 

#define M4OD_TID 0x05
#define NIT_TID 0x40 
#define ONIT_TID 0x41 
#define SDT_TID 0x42 

#define OSDT_TID 0x46 

#define BAT_TID 0x4A 
#define UNT_TID 0x4B 
#define DFI_TID 0x4C 

#define EIT_TID 0x4E 
#define OEIT_TID 0x4F 
#define EITS_START_TID 0x50 
#define EITS_END_TID 0x5F 
#define OEITS_START_TID 0x60 
#define OEITS_END_TID 0x6F 
#define TDT_TID 0x70 
#define RST_TID 0x71 
#define ST_TID 0x72 
#define TOT_TID 0x73 
#define AIT_TID 0x74 
#define CT_TID 0x75 
#define RCT_TID 0x76 
#define CIT_TID 0x77 
#define MPE_FEC_TID 0x78 
#define RPNT_TID 0x79 
#define MPE_IFEC_TID 0x7A 
#define PROTMT_TID 0x7B 

#define DIT_TID 0x7E 
#define SIT_TID 0x7F 



#define STREAM_TYPE_VIDEO_MPEG1 0x01
#define STREAM_TYPE_VIDEO_MPEG2 0x02
#define STREAM_TYPE_AUDIO_MPEG1 0x03
#define STREAM_TYPE_AUDIO_MPEG2 0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA 0x06
#define STREAM_TYPE_AUDIO_AAC 0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM 0x11
#define STREAM_TYPE_VIDEO_MPEG4 0x10
#define STREAM_TYPE_METADATA 0x15
#define STREAM_TYPE_VIDEO_H264 0x1b
#define STREAM_TYPE_VIDEO_HEVC 0x24
#define STREAM_TYPE_VIDEO_CAVS 0x42
#define STREAM_TYPE_VIDEO_VC1 0xea
#define STREAM_TYPE_VIDEO_DIRAC 0xd1

#define STREAM_TYPE_AUDIO_AC3 0x81
#define STREAM_TYPE_AUDIO_DTS 0x82
#define STREAM_TYPE_AUDIO_TRUEHD 0x83
#define STREAM_TYPE_AUDIO_EAC3 0x87

typedef struct MpegTSContext MpegTSContext;

MpegTSContext *avpriv_mpegts_parse_open(AVFormatContext *s);
int avpriv_mpegts_parse_packet(MpegTSContext *ts, AVPacket *pkt,
const uint8_t *buf, int len);
void avpriv_mpegts_parse_close(MpegTSContext *ts);

typedef struct SLConfigDescr {
int use_au_start;
int use_au_end;
int use_rand_acc_pt;
int use_padding;
int use_timestamps;
int use_idle;
int timestamp_res;
int timestamp_len;
int ocr_len;
int au_len;
int inst_bitrate_len;
int degr_prior_len;
int au_seq_num_len;
int packet_seq_num_len;
} SLConfigDescr;

typedef struct Mp4Descr {
int es_id;
int dec_config_descr_len;
uint8_t *dec_config_descr;
SLConfigDescr sl;
} Mp4Descr;










int ff_parse_mpeg2_descriptor(AVFormatContext *fc, AVStream *st, int stream_type,
const uint8_t **pp, const uint8_t *desc_list_end,
Mp4Descr *mp4_descr, int mp4_descr_count, int pid,
MpegTSContext *ts);





int ff_check_h264_startcode(AVFormatContext *s, const AVStream *st, const AVPacket *pkt);

#endif 
