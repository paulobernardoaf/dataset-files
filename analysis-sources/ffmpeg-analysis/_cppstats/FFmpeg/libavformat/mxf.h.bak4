



















#if !defined(AVFORMAT_MXF_H)
#define AVFORMAT_MXF_H

#include "avformat.h"
#include "libavcodec/avcodec.h"
#include <stdint.h>

typedef uint8_t UID[16];

enum MXFMetadataSetType {
AnyType,
MaterialPackage,
SourcePackage,
SourceClip,
TimecodeComponent,
PulldownComponent,
Sequence,
MultipleDescriptor,
Descriptor,
Track,
CryptoContext,
Preface,
Identification,
ContentStorage,
SubDescriptor,
IndexTableSegment,
EssenceContainerData,
EssenceGroup,
TaggedValue,
TapeDescriptor,
AVCSubDescriptor,
};

enum MXFFrameLayout {
FullFrame = 0,
SeparateFields,
OneField,
MixedFields,
SegmentedFrame,
};

typedef struct MXFContentPackageRate {
int rate;
AVRational tb;
} MXFContentPackageRate;

typedef struct KLVPacket {
UID key;
int64_t offset;
uint64_t length;
int64_t next_klv;
} KLVPacket;

typedef enum {
NormalWrap = 0,
D10D11Wrap,
RawAWrap,
RawVWrap
} MXFWrappingIndicatorType;

typedef struct MXFCodecUL {
UID uid;
unsigned matching_len;
int id;
const char *desc;
unsigned wrapping_indicator_pos;
MXFWrappingIndicatorType wrapping_indicator_type;
} MXFCodecUL;

extern const MXFCodecUL ff_mxf_data_definition_uls[];
extern const MXFCodecUL ff_mxf_codec_uls[];
extern const MXFCodecUL ff_mxf_pixel_format_uls[];
extern const MXFCodecUL ff_mxf_codec_tag_uls[];

int ff_mxf_decode_pixel_layout(const char pixel_layout[16], enum AVPixelFormat *pix_fmt);
int ff_mxf_get_content_package_rate(AVRational time_base);


#define PRIxUID "%02x.%02x.%02x.%02x." "%02x.%02x.%02x.%02x." "%02x.%02x.%02x.%02x." "%02x.%02x.%02x.%02x"





#define UID_ARG(x) (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5], (x)[6], (x)[7], (x)[8], (x)[9], (x)[10], (x)[11], (x)[12], (x)[13], (x)[14], (x)[15] 





#if defined(DEBUG)
#define PRINT_KEY(pc, s, x) av_log(pc, AV_LOG_VERBOSE, "%s " "0x%02x,0x%02x,0x%02x,0x%02x," "0x%02x,0x%02x,0x%02x,0x%02x," "0x%02x,0x%02x,0x%02x,0x%02x," "0x%02x,0x%02x,0x%02x,0x%02x ", s, UID_ARG(x)); av_log(pc, AV_LOG_INFO, "%s " "%02x.%02x.%02x.%02x." "%02x.%02x.%02x.%02x." "%02x.%02x.%02x.%02x." "%02x.%02x.%02x.%02x\n", s, UID_ARG(x))














#else
#define PRINT_KEY(pc, s, x) do { if(0) av_log(pc, AV_LOG_VERBOSE, "%s " "0x%02x,0x%02x,0x%02x,0x%02x," "0x%02x,0x%02x,0x%02x,0x%02x," "0x%02x,0x%02x,0x%02x,0x%02x," "0x%02x,0x%02x,0x%02x,0x%02x ", s, UID_ARG(x)); }while(0)








#endif

#endif 
