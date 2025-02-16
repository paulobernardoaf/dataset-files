#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_block.h>
#include <vlc_bits.h>
#include <vlc_block_helper.h>
#include "packetizer_helper.h"
#include "mpeg4audio.h"
#include <assert.h>
typedef struct
{
enum mpeg4_audioObjectType i_object_type;
unsigned i_samplerate;
uint8_t i_channel_configuration;
int8_t i_sbr; 
int8_t i_ps; 
struct
{
enum mpeg4_audioObjectType i_object_type;
unsigned i_samplerate;
uint8_t i_channel_configuration;
} extension;
unsigned i_frame_length; 
} mpeg4_asc_t;
#define LATM_MAX_EXTRA_SIZE 64
typedef struct
{
uint8_t i_program;
uint8_t i_layer;
unsigned i_frame_length; 
uint8_t i_frame_length_type;
uint8_t i_frame_length_index; 
mpeg4_asc_t cfg;
size_t i_extra;
uint8_t extra[LATM_MAX_EXTRA_SIZE];
} latm_stream_t;
#define LATM_MAX_LAYER (8)
#define LATM_MAX_PROGRAM (16)
typedef struct
{
bool b_same_time_framing;
uint8_t i_sub_frames;
uint8_t i_programs;
uint8_t pi_layers[LATM_MAX_PROGRAM];
uint8_t pi_stream[LATM_MAX_PROGRAM][LATM_MAX_LAYER];
uint8_t i_streams;
latm_stream_t stream[LATM_MAX_PROGRAM*LATM_MAX_LAYER];
uint32_t i_other_data;
int16_t i_crc; 
} latm_mux_t;
typedef struct
{
int i_state;
int i_type;
block_bytestream_t bytestream;
date_t end_date;
vlc_tick_t i_pts;
bool b_discontuinity;
int i_frame_size;
unsigned int i_channels;
unsigned int i_rate, i_frame_length, i_header_size;
int i_aac_profile;
int i_input_rate;
bool b_latm_cfg;
latm_mux_t latm;
int i_warnings;
} decoder_sys_t;
enum
{
WARN_CRC_UNSUPPORTED = 1
};
#define WARN_ONCE(warn, msg) do{decoder_sys_t *p_sys = p_dec->p_sys;if( (p_sys->i_warnings & warn) == 0 ){p_sys->i_warnings |= warn;msg_Warn( p_dec, msg );}} while(0)
enum {
TYPE_UNKNOWN, 
TYPE_UNKNOWN_NONRAW, 
TYPE_RAW, 
TYPE_ADTS,
TYPE_LOAS
};
static const int pi_sample_rates[16] =
{
96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
16000, 12000, 11025, 8000, 7350, 0, 0, 0
};
static int ChannelConfigurationToVLC(uint8_t i_channel)
{
if (i_channel == 7)
i_channel = 8; 
else if (i_channel >= 8)
i_channel = -1;
return i_channel;
}
static int AOTtoAACProfile(uint8_t i_object_type)
{
switch(i_object_type)
{
case AOT_AAC_MAIN:
case AOT_AAC_LC:
case AOT_AAC_SSR:
case AOT_AAC_LTP:
case AOT_AAC_SBR:
case AOT_AAC_SC:
case AOT_ER_AAC_LD:
case AOT_AAC_PS:
case AOT_ER_AAC_ELD:
{
static_assert(AOT_AAC_MAIN == AAC_PROFILE_MAIN + 1,
"invalid profile to object mapping");
return i_object_type - 1;
}
default:
return -1;
}
}
#define ADTS_HEADER_SIZE 9
#define LOAS_HEADER_SIZE 3
static int OpenPacketizer(vlc_object_t *);
static void ClosePacketizer(vlc_object_t *);
static block_t *Packetize (decoder_t *, block_t **);
static void Flush( decoder_t * );
static int Mpeg4ReadAudioSpecificConfig(bs_t *s, mpeg4_asc_t *p_cfg, bool);
vlc_module_begin ()
set_category(CAT_SOUT)
set_subcategory(SUBCAT_SOUT_PACKETIZER)
set_description(N_("MPEG4 audio packetizer"))
set_capability("packetizer", 50)
set_callbacks(OpenPacketizer, ClosePacketizer)
vlc_module_end ()
static int OpenPacketizer(vlc_object_t *p_this)
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
if (p_dec->fmt_in.i_codec != VLC_CODEC_MP4A)
return VLC_EGENERIC;
if ((p_dec->p_sys = p_sys = (decoder_sys_t *)malloc(sizeof(decoder_sys_t))) == NULL)
return VLC_ENOMEM;
p_sys->i_state = STATE_NOSYNC;
p_sys->b_discontuinity = false;
block_BytestreamInit(&p_sys->bytestream);
p_sys->b_latm_cfg = false;
p_sys->i_warnings = 0;
p_dec->fmt_out.i_codec = VLC_CODEC_MP4A;
msg_Dbg(p_dec, "running MPEG4 audio packetizer");
switch (p_dec->fmt_in.i_original_fourcc)
{
case VLC_FOURCC('L','A','T','M'):
p_sys->i_type = TYPE_LOAS;
msg_Dbg(p_dec, "LOAS/LATM Mode");
break;
case VLC_FOURCC('A','D','T','S'):
p_sys->i_type = TYPE_ADTS;
msg_Dbg(p_dec, "ADTS Mode");
break;
case VLC_FOURCC('H','E','A','D'):
p_sys->i_type = TYPE_UNKNOWN_NONRAW;
break;
default:
p_sys->i_type = TYPE_UNKNOWN;
break;
}
p_dec->fmt_out.audio.i_rate = p_dec->fmt_in.audio.i_rate;
if(p_dec->fmt_in.i_extra)
{
mpeg4_asc_t asc;
bs_t s;
bs_init(&s, p_dec->fmt_in.p_extra, p_dec->fmt_in.i_extra);
if(Mpeg4ReadAudioSpecificConfig(&s, &asc, true) == VLC_SUCCESS)
{
p_dec->fmt_out.audio.i_rate = asc.i_samplerate;
p_dec->fmt_out.audio.i_frame_length = asc.i_frame_length;
p_dec->fmt_out.audio.i_channels =
ChannelConfigurationToVLC(asc.i_channel_configuration);
if(p_dec->fmt_out.i_profile != -1)
p_dec->fmt_out.i_profile = AOTtoAACProfile(asc.i_object_type);
msg_Dbg(p_dec, "%sAAC%s %dHz %d samples/frame",
(asc.i_sbr) ? "HE-" : "",
(asc.i_ps) ? "v2" : "",
(asc.i_sbr) ? p_dec->fmt_out.audio.i_rate << 1
: p_dec->fmt_out.audio.i_rate,
p_dec->fmt_out.audio.i_frame_length);
}
p_dec->fmt_out.p_extra = malloc(p_dec->fmt_in.i_extra);
if (!p_dec->fmt_out.p_extra)
return VLC_ENOMEM;
p_dec->fmt_out.i_extra = p_dec->fmt_in.i_extra;
memcpy(p_dec->fmt_out.p_extra, p_dec->fmt_in.p_extra,
p_dec->fmt_in.i_extra);
}
date_Init(&p_sys->end_date, p_dec->fmt_out.audio.i_rate ?
p_dec->fmt_out.audio.i_rate : 48000, 1);
p_dec->pf_packetize = Packetize;
p_dec->pf_flush = Flush;
p_dec->pf_get_cc = NULL;
return VLC_SUCCESS;
}
static void ClosePacketizer(vlc_object_t *p_this)
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
block_BytestreamRelease(&p_sys->bytestream);
free(p_sys);
}
static block_t *ForwardRawBlock(decoder_t *p_dec, block_t **pp_block)
{
decoder_sys_t *p_sys = p_dec->p_sys;
block_t *p_block;
if (!pp_block || !*pp_block)
return NULL;
p_block = *pp_block;
*pp_block = NULL; 
vlc_tick_t i_diff = 0;
if (p_block->i_pts != VLC_TICK_INVALID &&
p_block->i_pts != date_Get(&p_sys->end_date))
{
if(date_Get(&p_sys->end_date) != VLC_TICK_INVALID)
i_diff = llabs( date_Get(&p_sys->end_date) - p_block->i_pts );
date_Set(&p_sys->end_date, p_block->i_pts);
}
p_block->i_pts = p_block->i_dts = date_Get(&p_sys->end_date);
if(p_dec->fmt_out.audio.i_frame_length && p_block->i_pts != VLC_TICK_INVALID)
{
p_block->i_length = date_Increment(&p_sys->end_date,
p_dec->fmt_out.audio.i_frame_length) - p_block->i_pts;
if( i_diff > p_block->i_length )
p_sys->b_discontuinity = true;
}
return p_block;
}
static int ADTSSyncInfo(decoder_t * p_dec, const uint8_t * p_buf,
unsigned int * pi_channels,
unsigned int * pi_sample_rate,
unsigned int * pi_frame_length,
unsigned int * pi_header_size)
{
int i_profile, i_sample_rate_idx, i_frame_size;
bool b_crc;
b_crc = !(p_buf[1] & 0x01);
i_profile = p_buf[2] >> 6;
i_sample_rate_idx = (p_buf[2] >> 2) & 0x0f;
*pi_sample_rate = pi_sample_rates[i_sample_rate_idx];
*pi_channels = ((p_buf[2] & 0x01) << 2) | ((p_buf[3] >> 6) & 0x03);
if (*pi_channels == 0) 
*pi_channels = 2;
i_frame_size = ((p_buf[3] & 0x03) << 11) | (p_buf[4] << 3) |
((p_buf[5] >> 5) );
unsigned short i_raw_blocks_in_frame = p_buf[6] & 0x03;
if (!*pi_sample_rate || !i_frame_size) {
msg_Warn(p_dec, "Invalid ADTS header");
return 0;
}
*pi_frame_length = 1024;
if (i_raw_blocks_in_frame == 0) {
if (b_crc) {
WARN_ONCE(WARN_CRC_UNSUPPORTED, "ADTS CRC not supported");
}
} else {
msg_Err(p_dec, "Multiple blocks per frame in ADTS not supported");
return 0;
#if 0
int i;
const uint8_t *p_pos = p_buf + 7;
uint16_t crc_block;
uint16_t i_block_pos[3];
if (b_crc) {
for (i = 0 ; i < i_raw_blocks_in_frame ; i++) {
i_block_pos[i] = (*p_pos << 8) | *(p_pos+1);
p_pos += 2;
}
crc_block = (*p_pos << 8) | *(p_pos+1);
p_pos += 2;
}
for (i = 0 ; i <= i_raw_blocks_in_frame ; i++) {
if (b_crc) {
WARN_ONCE(WARN_CRC_UNSUPPORTED, "ADTS CRC not supported");
}
}
#endif
}
if (!p_dec->fmt_out.i_extra) {
p_dec->fmt_out.p_extra = malloc(2);
if (!p_dec->fmt_out.p_extra)
return 0;
p_dec->fmt_out.i_extra = 2;
((uint8_t *)p_dec->fmt_out.p_extra)[0] =
(i_profile + 1) << 3 | (i_sample_rate_idx >> 1);
((uint8_t *)p_dec->fmt_out.p_extra)[1] =
((i_sample_rate_idx & 0x01) << 7) | (*pi_channels <<3);
}
*pi_header_size = b_crc ? 9 : 7;
return i_frame_size - *pi_header_size;
}
static int LOASSyncInfo(uint8_t p_header[LOAS_HEADER_SIZE], unsigned int *pi_header_size)
{
*pi_header_size = 3;
return ((p_header[1] & 0x1f) << 8) + p_header[2];
}
static int Mpeg4GAProgramConfigElement(bs_t *s)
{
int i_tag = bs_read(s, 4);
if (i_tag != 0x05)
return -1;
bs_skip(s, 2 + 4); 
int i_num_front = bs_read(s, 4);
int i_num_side = bs_read(s, 4);
int i_num_back = bs_read(s, 4);
int i_num_lfe = bs_read(s, 2);
int i_num_assoc_data = bs_read(s, 3);
int i_num_valid_cc = bs_read(s, 4);
if (bs_read1(s))
bs_skip(s, 4); 
if (bs_read1(s))
bs_skip(s, 4); 
if (bs_read1(s))
bs_skip(s, 2+1); 
bs_skip(s, i_num_front * (1+4));
bs_skip(s, i_num_side * (1+4));
bs_skip(s, i_num_back * (1+4));
bs_skip(s, i_num_lfe * (4));
bs_skip(s, i_num_assoc_data * (4));
bs_skip(s, i_num_valid_cc * (5));
bs_align(s);
int i_comment = bs_read(s, 8);
bs_skip(s, i_comment * 8);
return 0;
}
static int Mpeg4GASpecificConfig(mpeg4_asc_t *p_cfg, bs_t *s)
{
p_cfg->i_frame_length = bs_read1(s) ? 960 : 1024;
if(p_cfg->i_object_type == AOT_ER_AAC_LD) 
p_cfg->i_frame_length >>= 1;
else if(p_cfg->i_object_type == AOT_AAC_SSR)
p_cfg->i_frame_length = 256;
if (bs_read1(s)) 
bs_skip(s, 14); 
int i_extension_flag = bs_read1(s);
if (p_cfg->i_channel_configuration == 0)
Mpeg4GAProgramConfigElement(s);
if (p_cfg->i_object_type == AOT_AAC_SC ||
p_cfg->i_object_type == AOT_ER_AAC_SC)
bs_skip(s, 3); 
if (i_extension_flag) {
if (p_cfg->i_object_type == AOT_ER_BSAC)
bs_skip(s, 5 + 11); 
if (p_cfg->i_object_type == AOT_ER_AAC_LC ||
p_cfg->i_object_type == AOT_ER_AAC_LTP ||
p_cfg->i_object_type == AOT_ER_AAC_SC ||
p_cfg->i_object_type == AOT_ER_AAC_LD)
bs_skip(s, 1+1+1); 
if (bs_read1(s)) 
fprintf(stderr, "Mpeg4GASpecificConfig: error 1\n");
}
return 0;
}
static int Mpeg4ELDSpecificConfig(mpeg4_asc_t *p_cfg, bs_t *s)
{
p_cfg->i_frame_length = bs_read1(s) ? 960 : 480;
bs_skip(s, 3);
if(bs_read1(s)) 
{
bs_skip(s, 2);
unsigned numSbrHeader;
switch(p_cfg->i_channel_configuration)
{
case 1: case 2:
numSbrHeader = 1;
break;
case 3:
numSbrHeader = 2;
break;
case 4: case 5: case 6:
numSbrHeader = 3;
break;
case 7:
numSbrHeader = 4;
break;
default:
numSbrHeader = 0;
break;
}
for( ; numSbrHeader; numSbrHeader-- )
{
bs_read(s, 14);
bool header_extra_1 = bs_read1(s);
bool header_extra_2 = bs_read1(s);
if(header_extra_1)
bs_read(s, 5);
if(header_extra_2)
bs_read(s, 6);
}
}
for(unsigned eldExtType = bs_read(s, 4);
eldExtType != 0x0 ;
eldExtType = bs_read(s, 4))
{
unsigned eldExtLen = bs_read(s, 4);
unsigned eldExtLenAdd = 0;
if(eldExtLen == 15)
{
eldExtLenAdd = bs_read(s, 8);
eldExtLen += eldExtLenAdd;
}
if(eldExtLenAdd == 255)
eldExtLen += bs_read(s, 16);
for(; eldExtLen; eldExtLen--)
bs_skip(s, 8);
}
return 0;
}
static enum mpeg4_audioObjectType Mpeg4ReadAudioObjectType(bs_t *s)
{
int i_type = bs_read(s, 5);
if (i_type == 31)
i_type = 32 + bs_read(s, 6);
return i_type;
}
static unsigned Mpeg4ReadAudioSamplerate(bs_t *s)
{
int i_index = bs_read(s, 4);
if (i_index != 0x0f)
return pi_sample_rates[i_index];
return bs_read(s, 24);
}
static int Mpeg4ReadAudioSpecificConfig(bs_t *s, mpeg4_asc_t *p_cfg, bool b_withext)
{
p_cfg->i_object_type = Mpeg4ReadAudioObjectType(s);
p_cfg->i_samplerate = Mpeg4ReadAudioSamplerate(s);
p_cfg->i_channel_configuration = bs_read(s, 4);
p_cfg->i_sbr = -1;
p_cfg->i_ps = -1;
p_cfg->extension.i_object_type = 0;
p_cfg->extension.i_samplerate = 0;
p_cfg->extension.i_channel_configuration = 0;
p_cfg->i_frame_length = 0;
if (p_cfg->i_object_type == AOT_AAC_SBR ||
p_cfg->i_object_type == AOT_AAC_PS) {
p_cfg->i_sbr = 1;
if (p_cfg->i_object_type == AOT_AAC_PS)
p_cfg->i_ps = 1;
p_cfg->extension.i_object_type = AOT_AAC_SBR;
p_cfg->extension.i_samplerate = Mpeg4ReadAudioSamplerate(s);
p_cfg->i_object_type = Mpeg4ReadAudioObjectType(s);
if(p_cfg->i_object_type == AOT_ER_BSAC)
p_cfg->extension.i_channel_configuration = bs_read(s, 4);
}
switch(p_cfg->i_object_type)
{
case AOT_AAC_MAIN:
case AOT_AAC_LC:
case AOT_AAC_SSR:
case AOT_AAC_LTP:
case AOT_AAC_SC:
case AOT_TWINVQ:
case AOT_ER_AAC_LC:
case AOT_ER_AAC_LTP:
case AOT_ER_AAC_SC:
case AOT_ER_TWINVQ:
case AOT_ER_BSAC:
case AOT_ER_AAC_LD:
Mpeg4GASpecificConfig(p_cfg, s);
break;
case AOT_CELP:
case AOT_HVXC:
case AOT_TTSI:
case AOT_MAIN_SYNTHETIC:
case AOT_WAVETABLES:
case AOT_GENERAL_MIDI:
case AOT_ALGORITHMIC:
case AOT_ER_CELP:
case AOT_ER_HXVC:
case AOT_ER_HILN:
case AOT_ER_Parametric:
case AOT_SSC:
case AOT_LAYER1:
case AOT_LAYER2:
case AOT_LAYER3:
case AOT_DST:
case AOT_ALS:
case AOT_SLS:
case AOT_SLS_NON_CORE:
case AOT_ER_AAC_ELD:
Mpeg4ELDSpecificConfig(p_cfg, s);
break;
case AOT_SMR_SIMPLE:
case AOT_SMR_MAIN:
default:
return VLC_EGENERIC;
}
switch(p_cfg->i_object_type)
{
case AOT_ER_AAC_LC:
case AOT_ER_AAC_LTP:
case AOT_ER_AAC_SC:
case AOT_ER_TWINVQ:
case AOT_ER_BSAC:
case AOT_ER_AAC_LD:
case AOT_ER_CELP:
case AOT_ER_HXVC:
case AOT_ER_HILN:
case AOT_ER_Parametric:
case AOT_ER_AAC_ELD:
{
int epConfig = bs_read(s, 2);
if (epConfig == 2 || epConfig == 3)
if (epConfig == 3)
if (bs_read1(s)) {
}
break;
}
default:
break;
}
if (b_withext && p_cfg->extension.i_object_type != AOT_AAC_SBR &&
bs_remain(s) >= 16 && bs_read(s, 11) == 0x2b7)
{
p_cfg->extension.i_object_type = Mpeg4ReadAudioObjectType(s);
if (p_cfg->extension.i_object_type == AOT_AAC_SBR)
{
p_cfg->i_sbr = bs_read1(s);
if (p_cfg->i_sbr == 1) {
p_cfg->extension.i_samplerate = Mpeg4ReadAudioSamplerate(s);
if (bs_remain(s) >= 12 && bs_read(s, 11) == 0x548)
p_cfg->i_ps = bs_read1(s);
}
}
else if (p_cfg->extension.i_object_type == AOT_ER_BSAC)
{
p_cfg->i_sbr = bs_read1(s);
if(p_cfg->i_sbr)
p_cfg->extension.i_samplerate = Mpeg4ReadAudioSamplerate(s);
p_cfg->extension.i_channel_configuration = bs_read(s, 4);
}
}
#if 0
static const char *ppsz_otype[] = {
"NULL",
"AAC Main", "AAC LC", "AAC SSR", "AAC LTP", "SBR", "AAC Scalable",
"TwinVQ",
"CELP", "HVXC",
"Reserved", "Reserved",
"TTSI",
"Main Synthetic", "Wavetables Synthesis", "General MIDI",
"Algorithmic Synthesis and Audio FX",
"ER AAC LC",
"Reserved",
"ER AAC LTP", "ER AAC Scalable", "ER TwinVQ", "ER BSAC", "ER AAC LD",
"ER CELP", "ER HVXC", "ER HILN", "ER Parametric",
"SSC",
"PS", "MPEG Surround", "Escape",
"Layer 1", "Layer 2", "Layer 3",
"DST", "ALS", "SLS", "SLS non-core", "ELD",
"SMR Simple", "SMR Main",
};
fprintf(stderr, "Mpeg4ReadAudioSpecificInfo: t=%s(%d)f=%d c=%d sbr=%d\n",
ppsz_otype[p_cfg->i_object_type], p_cfg->i_object_type,
p_cfg->i_samplerate, p_cfg->i_channel, p_cfg->i_sbr);
#endif
return VLC_SUCCESS;
}
static uint32_t LatmGetValue(bs_t *s)
{
uint32_t v = 0;
for (int i = 1 + bs_read(s, 2); i > 0; i--)
v = (v << 8) + bs_read(s, 8);
return v;
}
static size_t AudioSpecificConfigBitsToBytes(bs_t *s, uint32_t i_bits, uint8_t *p_data)
{
size_t i_extra = __MIN((i_bits + 7) / 8, LATM_MAX_EXTRA_SIZE);
for (size_t i = 0; i < i_extra; i++) {
const uint32_t i_read = __MIN(8, i_bits - 8*i);
p_data[i] = bs_read(s, i_read) << (8-i_read);
}
return i_extra;
}
static int LatmReadStreamMuxConfiguration(latm_mux_t *m, bs_t *s)
{
int i_mux_version;
int i_mux_versionA;
i_mux_version = bs_read(s, 1);
i_mux_versionA = 0;
if (i_mux_version)
i_mux_versionA = bs_read(s, 1);
if (i_mux_versionA != 0) 
return -1;
memset(m, 0, sizeof(*m));
if (i_mux_versionA == 0)
if (i_mux_version == 1)
LatmGetValue(s); 
m->b_same_time_framing = bs_read1(s);
m->i_sub_frames = 1 + bs_read(s, 6);
m->i_programs = 1 + bs_read(s, 4);
for (uint8_t i_program = 0; i_program < m->i_programs; i_program++) {
m->pi_layers[i_program] = 1+bs_read(s, 3);
for (uint8_t i_layer = 0; i_layer < m->pi_layers[i_program]; i_layer++) {
latm_stream_t *st = &m->stream[m->i_streams];
bool b_previous_cfg;
m->pi_stream[i_program][i_layer] = m->i_streams;
st->i_program = i_program;
st->i_layer = i_layer;
b_previous_cfg = false;
if (i_program != 0 || i_layer != 0)
b_previous_cfg = bs_read1(s);
if (b_previous_cfg) {
assert(m->i_streams > 0);
st->cfg = m->stream[m->i_streams-1].cfg;
} else {
uint32_t asc_size = 0;
if(i_mux_version > 0)
asc_size = LatmGetValue(s);
bs_t asc_bs = *s;
Mpeg4ReadAudioSpecificConfig(&asc_bs, &st->cfg, i_mux_version > 0);
if (i_mux_version == 0)
asc_size = bs_pos(&asc_bs) - bs_pos(s);
asc_bs = *s;
st->i_extra = AudioSpecificConfigBitsToBytes(&asc_bs, asc_size, st->extra);
bs_skip(s, asc_size);
}
st->i_frame_length_type = bs_read(s, 3);
switch(st->i_frame_length_type)
{
case 0:
{
bs_skip(s, 8); 
if (!m->b_same_time_framing)
if (st->cfg.i_object_type == AOT_AAC_SC ||
st->cfg.i_object_type == AOT_CELP ||
st->cfg.i_object_type == AOT_ER_AAC_SC ||
st->cfg.i_object_type == AOT_ER_CELP)
bs_skip(s, 6); 
break;
}
case 1:
st->i_frame_length = bs_read(s, 9);
break;
case 3: case 4: case 5:
st->i_frame_length_index = bs_read(s, 6); 
break;
case 6: case 7:
st->i_frame_length_index = bs_read(s, 1); 
default:
break;
}
m->i_streams++;
}
}
if (bs_read1(s)) {
if (i_mux_version == 1)
m->i_other_data = LatmGetValue(s);
else {
int b_continue;
do {
b_continue = bs_read1(s);
m->i_other_data = (m->i_other_data << 8) + bs_read(s, 8);
} while (b_continue);
}
}
m->i_crc = -1;
if (bs_read1(s))
m->i_crc = bs_read(s, 8);
return 0;
}
static int LOASParse(decoder_t *p_dec, uint8_t *p_buffer, int i_buffer)
{
decoder_sys_t *p_sys = p_dec->p_sys;
bs_t s;
int i_accumulated = 0;
bs_init(&s, p_buffer, i_buffer);
if (!bs_read1(&s) && !LatmReadStreamMuxConfiguration(&p_sys->latm, &s) &&
p_sys->latm.i_streams > 0) {
const latm_stream_t *st = &p_sys->latm.stream[0];
if(st->cfg.i_samplerate == 0 || st->cfg.i_frame_length == 0 ||
ChannelConfigurationToVLC(st->cfg.i_channel_configuration) == 0)
return 0;
p_sys->i_channels = ChannelConfigurationToVLC(st->cfg.i_channel_configuration);
p_sys->i_rate = st->cfg.i_samplerate;
p_sys->i_frame_length = st->cfg.i_frame_length;
p_sys->i_aac_profile = AOTtoAACProfile(st->cfg.i_object_type);
if (p_sys->i_channels && p_sys->i_rate && p_sys->i_frame_length > 0)
{
if((size_t)p_dec->fmt_out.i_extra != st->i_extra ||
(p_dec->fmt_out.i_extra > 0 &&
memcmp(p_dec->fmt_out.p_extra, st->extra, st->i_extra)) )
{
if(p_dec->fmt_out.i_extra)
free(p_dec->fmt_out.p_extra);
p_dec->fmt_out.p_extra = malloc(st->i_extra);
if(p_dec->fmt_out.p_extra)
{
p_dec->fmt_out.i_extra = st->i_extra;
memcpy(p_dec->fmt_out.p_extra, st->extra, st->i_extra);
p_sys->b_latm_cfg = true;
}
else
{
p_dec->fmt_out.i_extra = 0;
p_sys->b_latm_cfg = false;
}
}
}
}
if (!p_sys->b_latm_cfg)
{
if( p_dec->fmt_in.i_extra > 0 &&
p_sys->i_channels && p_sys->i_rate && p_sys->i_frame_length )
{
p_sys->b_latm_cfg = true;
}
else return 0;
}
if (p_sys->latm.i_sub_frames > 1)
msg_Err(p_dec, "latm sub frames not yet supported, please send a sample");
for (uint8_t i_sub = 0; i_sub < p_sys->latm.i_sub_frames; i_sub++) {
unsigned pi_payload[LATM_MAX_PROGRAM][LATM_MAX_LAYER];
if (p_sys->latm.b_same_time_framing) {
for (uint8_t i_program = 0; i_program < p_sys->latm.i_programs; i_program++) {
for (uint8_t i_layer = 0; i_layer < p_sys->latm.pi_layers[i_program]; i_layer++) {
latm_stream_t *st = &p_sys->latm.stream[p_sys->latm.pi_stream[i_program][i_layer]];
if (st->i_frame_length_type == 0) {
unsigned i_payload = 0;
for (;;) {
uint8_t i_tmp = bs_read(&s, 8);
i_payload += i_tmp;
if (i_tmp != 255)
break;
}
pi_payload[i_program][i_layer] = i_payload;
} else if (st->i_frame_length_type == 1) {
pi_payload[i_program][i_layer] = st->i_frame_length / 8; 
} else if ((st->i_frame_length_type == 3) ||
(st->i_frame_length_type == 5) ||
(st->i_frame_length_type == 7)) {
bs_skip(&s, 2); 
pi_payload[i_program][i_layer] = 0; 
} else {
pi_payload[i_program][i_layer] = 0; 
}
}
}
for (uint8_t i_program = 0; i_program < p_sys->latm.i_programs; i_program++) {
for (uint8_t i_layer = 0; i_layer < p_sys->latm.pi_layers[i_program]; i_layer++) {
if (i_program != 0 || i_layer != 0)
break;
if (pi_payload[i_program][i_layer] <= 0)
continue;
for (unsigned i = 0; i < pi_payload[i_program][i_layer]; i++) {
if (i_accumulated >= i_buffer)
return 0;
p_buffer[i_accumulated++] = bs_read(&s, 8);
}
}
}
} else {
const int i_chunks = bs_read(&s, 4);
#if 0
int pi_program[16];
int pi_layer[16];
#endif
msg_Err(p_dec, "latm without same time frameing not yet supported, please send a sample");
for (int i_chunk = 0; i_chunk < i_chunks; i_chunk++) {
const int streamIndex = bs_read(&s, 4);
latm_stream_t *st = &p_sys->latm.stream[streamIndex];
const int i_program = st->i_program;
const int i_layer = st->i_layer;
#if 0
pi_program[i_chunk] = i_program;
pi_layer[i_chunk] = i_layer;
#endif
if (st->i_frame_length_type == 0) {
int i_payload = 0;
for (;;) {
int i_tmp = bs_read(&s, 8);
i_payload += i_tmp;
if (i_tmp != 255)
break;
}
pi_payload[i_program][i_layer] = i_payload;
bs_skip(&s, 1); 
} else if (st->i_frame_length_type == 1) {
pi_payload[i_program][i_layer] = st->i_frame_length / 8; 
} else if ((st->i_frame_length_type == 3) ||
(st->i_frame_length_type == 5) ||
(st->i_frame_length_type == 7)) {
bs_read(&s, 2); 
}
}
#if 0
for (int i_chunk = 0; i_chunk < i_chunks; i_chunk++) {
}
#endif
}
}
#if 0
if (p_sys->latm.i_other_data > 0)
; 
#endif
bs_align(&s);
return i_accumulated;
}
static void SetupOutput(decoder_t *p_dec, block_t *p_block)
{
decoder_sys_t *p_sys = p_dec->p_sys;
if (p_dec->fmt_out.audio.i_rate != p_sys->i_rate && p_sys->i_rate > 0)
{
msg_Info(p_dec, "AAC channels: %d samplerate: %d",
p_sys->i_channels, p_sys->i_rate);
date_Change(&p_sys->end_date, p_sys->i_rate, 1);
}
p_dec->fmt_out.audio.i_rate = p_sys->i_rate;
p_dec->fmt_out.audio.i_channels = p_sys->i_channels;
p_dec->fmt_out.audio.i_bytes_per_frame = p_sys->i_frame_size;
p_dec->fmt_out.audio.i_frame_length = p_sys->i_frame_length;
p_dec->fmt_out.i_profile = p_sys->i_aac_profile;
#if 0
p_dec->fmt_out.audio.i_physical_channels = p_sys->i_channels_conf;
#endif
p_block->i_pts = p_block->i_dts = date_Get(&p_sys->end_date);
p_block->i_length =
date_Increment(&p_sys->end_date, p_sys->i_frame_length) - p_block->i_pts;
}
static void Flush(decoder_t *p_dec)
{
decoder_sys_t *p_sys = p_dec->p_sys;
p_sys->i_state = STATE_NOSYNC;
block_BytestreamEmpty(&p_sys->bytestream);
date_Set(&p_sys->end_date, VLC_TICK_INVALID);
p_sys->b_discontuinity = true;
}
static inline bool HasADTSHeader( const uint8_t *p_header )
{
return p_header[0] == 0xff && (p_header[1] & 0xf6) == 0xf0;
}
static inline bool HasLoasHeader( const uint8_t *p_header )
{
return p_header[0] == 0x56 && (p_header[1] & 0xe0) == 0xe0;
}
static block_t *PacketizeStreamBlock(decoder_t *p_dec, block_t **pp_block)
{
decoder_sys_t *p_sys = p_dec->p_sys;
uint8_t p_header[ADTS_HEADER_SIZE + LOAS_HEADER_SIZE];
block_t *p_out_buffer;
uint8_t *p_buf;
block_t *p_block = pp_block ? *pp_block : NULL;
if(p_block)
{
block_BytestreamPush(&p_sys->bytestream, p_block);
*pp_block = NULL;
}
for (;;) switch(p_sys->i_state) {
case STATE_NOSYNC:
while (block_PeekBytes(&p_sys->bytestream, p_header, 2) == VLC_SUCCESS) {
if ((p_sys->i_type == TYPE_ADTS || p_sys->i_type == TYPE_UNKNOWN_NONRAW) &&
HasADTSHeader( p_header ) )
{
if (p_sys->i_type != TYPE_ADTS)
msg_Dbg(p_dec, "detected ADTS format");
p_sys->i_state = STATE_SYNC;
p_sys->i_type = TYPE_ADTS;
break;
}
else if ((p_sys->i_type == TYPE_LOAS || p_sys->i_type == TYPE_UNKNOWN_NONRAW) &&
HasLoasHeader( p_header ) )
{
if (p_sys->i_type != TYPE_LOAS)
msg_Dbg(p_dec, "detected LOAS format");
p_sys->i_state = STATE_SYNC;
p_sys->i_type = TYPE_LOAS;
break;
}
block_SkipByte(&p_sys->bytestream);
}
if (p_sys->i_state != STATE_SYNC) {
block_BytestreamFlush(&p_sys->bytestream);
return NULL;
}
case STATE_SYNC:
p_sys->i_pts = p_sys->bytestream.p_block->i_pts;
if (p_sys->i_pts != VLC_TICK_INVALID &&
p_sys->i_pts != date_Get(&p_sys->end_date))
date_Set(&p_sys->end_date, p_sys->i_pts);
p_sys->i_state = STATE_HEADER;
break;
case STATE_HEADER:
if (p_sys->i_type == TYPE_ADTS) {
if (block_PeekBytes(&p_sys->bytestream, p_header,
ADTS_HEADER_SIZE) != VLC_SUCCESS)
return NULL; 
p_sys->i_frame_size = ADTSSyncInfo(p_dec, p_header,
&p_sys->i_channels,
&p_sys->i_rate,
&p_sys->i_frame_length,
&p_sys->i_header_size);
} else {
assert(p_sys->i_type == TYPE_LOAS);
if (block_PeekBytes(&p_sys->bytestream, p_header,
LOAS_HEADER_SIZE) != VLC_SUCCESS)
return NULL; 
p_sys->i_frame_size = LOASSyncInfo(p_header, &p_sys->i_header_size);
}
if (p_sys->i_frame_size <= 0) {
msg_Dbg(p_dec, "emulated sync word");
block_SkipByte(&p_sys->bytestream);
p_sys->i_state = STATE_NOSYNC;
break;
}
p_sys->i_state = STATE_NEXT_SYNC;
case STATE_NEXT_SYNC:
if (p_sys->bytestream.p_block == NULL) {
p_sys->i_state = STATE_NOSYNC;
block_BytestreamFlush(&p_sys->bytestream);
return NULL;
}
if (block_PeekOffsetBytes(&p_sys->bytestream, p_sys->i_frame_size
+ p_sys->i_header_size, p_header, 2) != VLC_SUCCESS)
{
if(p_block == NULL) 
{
p_sys->i_state = STATE_SEND_DATA;
break;
}
return NULL; 
}
assert((p_sys->i_type == TYPE_ADTS) || (p_sys->i_type == TYPE_LOAS));
if ( (p_sys->i_type == TYPE_ADTS && !HasADTSHeader( p_header )) ||
(p_sys->i_type == TYPE_LOAS && !HasLoasHeader( p_header )) )
{
if( block_PeekOffsetBytes(&p_sys->bytestream,
p_sys->i_frame_size + p_sys->i_header_size,
p_header, 3) == VLC_SUCCESS &&
p_header[0] == 0x00 &&
((p_sys->i_type == TYPE_ADTS && HasADTSHeader( &p_header[1] )) ||
(p_sys->i_type == TYPE_LOAS && !HasLoasHeader( &p_header[1] ))))
{
p_sys->i_state = STATE_SEND_DATA;
}
else
{
msg_Dbg(p_dec, "emulated sync word (no sync on following frame)"
" 0x%"PRIx8" 0x%"PRIx8, p_header[0], p_header[1] );
p_sys->i_state = STATE_NOSYNC;
block_SkipByte(&p_sys->bytestream);
}
break;
}
p_sys->i_state = STATE_SEND_DATA;
break;
case STATE_GET_DATA:
if (block_WaitBytes(&p_sys->bytestream, p_sys->i_frame_size +
p_sys->i_header_size) != VLC_SUCCESS)
return NULL; 
p_sys->i_state = STATE_SEND_DATA;
case STATE_SEND_DATA:
p_out_buffer = block_Alloc(p_sys->i_frame_size);
if (!p_out_buffer) {
return NULL;
}
p_buf = p_out_buffer->p_buffer;
block_SkipBytes(&p_sys->bytestream, p_sys->i_header_size);
block_GetBytes(&p_sys->bytestream, p_buf, p_sys->i_frame_size);
if (p_sys->i_type != TYPE_ADTS) { 
assert(p_sys->i_type == TYPE_LOAS);
p_out_buffer->i_buffer = LOASParse(p_dec, p_buf, p_sys->i_frame_size);
if (p_out_buffer->i_buffer <= 0)
{
if (!p_sys->b_latm_cfg)
msg_Warn(p_dec, "waiting for header");
block_Release(p_out_buffer);
p_out_buffer = NULL;
p_sys->i_state = STATE_NOSYNC;
break;
}
}
SetupOutput(p_dec, p_out_buffer);
if (p_sys->i_pts == p_sys->bytestream.p_block->i_pts)
p_sys->i_pts = p_sys->bytestream.p_block->i_pts = VLC_TICK_INVALID;
if( pp_block )
*pp_block = block_BytestreamPop(&p_sys->bytestream);
p_sys->i_state = STATE_NOSYNC;
return p_out_buffer;
}
return NULL;
}
static block_t *Packetize(decoder_t *p_dec, block_t **pp_block)
{
decoder_sys_t *p_sys = p_dec->p_sys;
block_t *p_block = pp_block ? *pp_block : NULL;
if(p_block)
{
if (p_block->i_flags & (BLOCK_FLAG_DISCONTINUITY|BLOCK_FLAG_CORRUPTED))
{
if(p_sys->i_type == TYPE_ADTS || p_sys->i_type == TYPE_LOAS)
{
block_t *p_drain = PacketizeStreamBlock(p_dec, NULL);
if(p_drain)
return p_drain;
}
Flush(p_dec);
if (p_block->i_flags & BLOCK_FLAG_CORRUPTED)
{
block_Release(p_block);
return NULL;
}
}
if ( p_block->i_pts == VLC_TICK_INVALID &&
date_Get(&p_sys->end_date) == VLC_TICK_INVALID )
{
block_Release(p_block);
return NULL;
}
}
if(p_block && p_sys->i_type == TYPE_UNKNOWN)
{
p_sys->i_type = TYPE_RAW;
if(p_block->i_buffer > 1)
{
if(p_block->p_buffer[0] == 0xff && (p_block->p_buffer[1] & 0xf6) == 0xf0)
{
p_sys->i_type = TYPE_ADTS;
}
else if(p_block->p_buffer[0] == 0x56 && (p_block->p_buffer[1] & 0xe0) == 0xe0)
{
p_sys->i_type = TYPE_LOAS;
}
}
}
if(p_sys->i_type == TYPE_RAW)
p_block = ForwardRawBlock(p_dec, pp_block);
else
p_block = PacketizeStreamBlock(p_dec, pp_block);
if(p_block && p_sys->b_discontuinity)
{
p_block->i_flags |= BLOCK_FLAG_DISCONTINUITY;
p_sys->b_discontuinity = false;
}
return p_block;
}
