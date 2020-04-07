


















#include <vlc_aout.h>

struct CoreAudio_layout_s
{
uint32_t i_channels_layout_tag;
uint32_t i_channels_bitmap;
uint32_t i_channels_description_count;
struct
{
uint32_t i_channel_label;
uint32_t i_channel_flags;
float f_coordinates[3];
} *p_descriptions;
};

static inline void CoreAudio_Layout_Clean(struct CoreAudio_layout_s *c)
{
free( c->p_descriptions );
}


enum
{
CoreAudio_Bitmap_LEFT = (1<<0),
CoreAudio_Bitmap_RIGHT = (1<<1),
CoreAudio_Bitmap_CENTER = (1<<2),
CoreAudio_Bitmap_LFESCREEN = (1<<3),
CoreAudio_Bitmap_BACKLEFT = (1<<4),
CoreAudio_Bitmap_BACKRIGHT = (1<<5),
CoreAudio_Bitmap_LEFTCENTER = (1<<6),
CoreAudio_Bitmap_RIGHTCENTER = (1<<7),
CoreAudio_Bitmap_BACKCENTER = (1<<8),
CoreAudio_Bitmap_SIDELEFT = (1<<9),
CoreAudio_Bitmap_SIDERIGHT = (1<<10),
CoreAudio_Bitmap_TOPCENTER = (1<<11),
CoreAudio_Bitmap_TOPFRONTLEFT = (1<<12),
CoreAudio_Bitmap_TOPFRONTENTER = (1<<13),
CoreAudio_Bitmap_TOPFRONTRIGHT = (1<<14),
CoreAudio_Bitmap_TOPBACKLEFT = (1<<15),
CoreAudio_Bitmap_TOPBACKCENTER = (1<<16),
CoreAudio_Bitmap_TOPBACKRIGHT = (1<<17),
};

static const uint32_t pi_vlc_chan_order_CoreAudio[] =
{
AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT, AOUT_CHAN_CENTER,
AOUT_CHAN_LFE,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARRIGHT,
AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT, AOUT_CHAN_REARCENTER,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
0
};

static const struct
{
uint32_t i_bitmap;
uint32_t i_vlc_bitmap;
} CoreAudio_Bitmap_mapping[] = {
{ CoreAudio_Bitmap_LEFT, AOUT_CHAN_LEFT },
{ CoreAudio_Bitmap_RIGHT, AOUT_CHAN_RIGHT },
{ CoreAudio_Bitmap_CENTER, AOUT_CHAN_CENTER },
{ CoreAudio_Bitmap_LFESCREEN, AOUT_CHAN_LFE },
{ CoreAudio_Bitmap_BACKLEFT, AOUT_CHAN_REARLEFT },
{ CoreAudio_Bitmap_BACKRIGHT, AOUT_CHAN_REARRIGHT },
{ CoreAudio_Bitmap_LEFTCENTER, AOUT_CHAN_LEFT },
{ CoreAudio_Bitmap_RIGHTCENTER, AOUT_CHAN_RIGHT },
{ CoreAudio_Bitmap_BACKCENTER, AOUT_CHAN_REARCENTER },
{ CoreAudio_Bitmap_SIDELEFT, AOUT_CHAN_MIDDLELEFT },
{ CoreAudio_Bitmap_SIDERIGHT, AOUT_CHAN_MIDDLERIGHT },
{ CoreAudio_Bitmap_TOPCENTER, 0 },
{ CoreAudio_Bitmap_TOPFRONTLEFT, 0 },
{ CoreAudio_Bitmap_TOPFRONTENTER,0 },
{ CoreAudio_Bitmap_TOPFRONTRIGHT,0 },
{ CoreAudio_Bitmap_TOPBACKLEFT, 0 },
{ CoreAudio_Bitmap_TOPBACKCENTER,0 },
{ CoreAudio_Bitmap_TOPBACKRIGHT, 0 },
};

enum CoreAudio_Layout
{
CoreAudio_Layout_DESC = 0,
CoreAudio_Layout_BITMAP = (1<<16),

CoreAudio_Layout_Mono = (100<<16) | 1,
CoreAudio_Layout_Stereo = (101<<16) | 2,
CoreAudio_Layout_StereoHeadphones = (102<<16) | 2,
CoreAudio_Layout_MatrixStereo = (103<<16) | 2,
CoreAudio_Layout_MidSide = (104<<16) | 2,
CoreAudio_Layout_XY = (105<<16) | 2,
CoreAudio_Layout_Binaural = (106<<16) | 2,
CoreAudio_Layout_Ambisonic_B_Format = (107<<16) | 4,

CoreAudio_Layout_Quadraphonic = (108<<16) | 4,
CoreAudio_Layout_Pentagonal = (109<<16) | 5,
CoreAudio_Layout_Hexagonal = (110<<16) | 6,
CoreAudio_Layout_Octagonal = (111<<16) | 8,
CoreAudio_Layout_Cube = (112<<16) | 8,

CoreAudio_Layout_MPEG_1_0 = CoreAudio_Layout_Mono,
CoreAudio_Layout_MPEG_2_0 = CoreAudio_Layout_Stereo, 
CoreAudio_Layout_MPEG_3_0_A = (113<<16) | 3, 
CoreAudio_Layout_MPEG_3_0_B = (114<<16) | 3, 
CoreAudio_Layout_MPEG_4_0_A = (115<<16) | 4, 
CoreAudio_Layout_MPEG_4_0_B = (116<<16) | 4, 
CoreAudio_Layout_MPEG_5_0_A = (117<<16) | 5, 
CoreAudio_Layout_MPEG_5_0_B = (118<<16) | 5, 
CoreAudio_Layout_MPEG_5_0_C = (119<<16) | 5, 
CoreAudio_Layout_MPEG_5_0_D = (120<<16) | 5, 
CoreAudio_Layout_MPEG_5_1_A = (121<<16) | 6, 
CoreAudio_Layout_MPEG_5_1_B = (122<<16) | 6, 
CoreAudio_Layout_MPEG_5_1_C = (123<<16) | 6, 
CoreAudio_Layout_MPEG_5_1_D = (124<<16) | 6, 
CoreAudio_Layout_MPEG_6_1_A = (125<<16) | 7, 
CoreAudio_Layout_MPEG_7_1_A = (126<<16) | 8, 
CoreAudio_Layout_MPEG_7_1_B = (127<<16) | 8, 
CoreAudio_Layout_MPEG_7_1_C = (128<<16) | 8, 
CoreAudio_Layout_Emagic_Default_7_1 = (129<<16) | 8, 
CoreAudio_Layout_SMPTE_DTV = (130<<16) | 8, 

CoreAudio_Layout_ITU_1_0 = CoreAudio_Layout_Mono,
CoreAudio_Layout_ITU_2_0 = CoreAudio_Layout_Stereo, 

CoreAudio_Layout_ITU_2_1 = (131<<16) | 3, 
CoreAudio_Layout_ITU_2_2 = (132<<16) | 4, 
CoreAudio_Layout_ITU_3_0 = CoreAudio_Layout_MPEG_3_0_A,
CoreAudio_Layout_ITU_3_1 = CoreAudio_Layout_MPEG_4_0_A,

CoreAudio_Layout_ITU_3_2 = CoreAudio_Layout_MPEG_5_0_A,
CoreAudio_Layout_ITU_3_2_1 = CoreAudio_Layout_MPEG_5_1_A,
CoreAudio_Layout_ITU_3_4_1 = CoreAudio_Layout_MPEG_7_1_C,

CoreAudio_Layout_DVD_0 = CoreAudio_Layout_Mono,
CoreAudio_Layout_DVD_1 = CoreAudio_Layout_Stereo, 
CoreAudio_Layout_DVD_2 = CoreAudio_Layout_ITU_2_1,
CoreAudio_Layout_DVD_3 = CoreAudio_Layout_ITU_2_2,
CoreAudio_Layout_DVD_4 = (133<<16) | 3, 
CoreAudio_Layout_DVD_5 = (134<<16) | 4, 
CoreAudio_Layout_DVD_6 = (135<<16) | 5, 
CoreAudio_Layout_DVD_7 = CoreAudio_Layout_MPEG_3_0_A,
CoreAudio_Layout_DVD_8 = CoreAudio_Layout_MPEG_4_0_A,
CoreAudio_Layout_DVD_9 = CoreAudio_Layout_MPEG_5_0_A,
CoreAudio_Layout_DVD_10 = (136<<16) | 4, 
CoreAudio_Layout_DVD_11 = (137<<16) | 5, 
CoreAudio_Layout_DVD_12 = CoreAudio_Layout_MPEG_5_1_A,
CoreAudio_Layout_DVD_13 = CoreAudio_Layout_DVD_8, 
CoreAudio_Layout_DVD_14 = CoreAudio_Layout_DVD_9, 
CoreAudio_Layout_DVD_15 = CoreAudio_Layout_DVD_10, 
CoreAudio_Layout_DVD_16 = CoreAudio_Layout_DVD_11, 
CoreAudio_Layout_DVD_17 = CoreAudio_Layout_DVD_12, 
CoreAudio_Layout_DVD_18 = (138<<16) | 5, 
CoreAudio_Layout_DVD_19 = CoreAudio_Layout_MPEG_5_0_B,
CoreAudio_Layout_DVD_20 = CoreAudio_Layout_MPEG_5_1_B,

CoreAudio_Layout_AudioUnit_4 = CoreAudio_Layout_Quadraphonic,
CoreAudio_Layout_AudioUnit_5 = CoreAudio_Layout_Pentagonal,
CoreAudio_Layout_AudioUnit_6 = CoreAudio_Layout_Hexagonal,
CoreAudio_Layout_AudioUnit_8 = CoreAudio_Layout_Octagonal,

CoreAudio_Layout_AudioUnit_5_0 = CoreAudio_Layout_MPEG_5_0_B,
CoreAudio_Layout_AudioUnit_6_0 = (139<<16) | 6, 
CoreAudio_Layout_AudioUnit_7_0 = (140<<16) | 7, 
CoreAudio_Layout_AudioUnit_7_0_Front= (148<<16) | 7, 
CoreAudio_Layout_AudioUnit_5_1 = CoreAudio_Layout_MPEG_5_1_A,
CoreAudio_Layout_AudioUnit_6_1 = CoreAudio_Layout_MPEG_6_1_A,
CoreAudio_Layout_AudioUnit_7_1 = CoreAudio_Layout_MPEG_7_1_C,
CoreAudio_Layout_AudioUnit_7_1_Front= CoreAudio_Layout_MPEG_7_1_A,

CoreAudio_Layout_AAC_3_0 = CoreAudio_Layout_MPEG_3_0_B,
CoreAudio_Layout_AAC_Quadraphonic = CoreAudio_Layout_Quadraphonic, 
CoreAudio_Layout_AAC_4_0 = CoreAudio_Layout_MPEG_4_0_B,
CoreAudio_Layout_AAC_5_0 = CoreAudio_Layout_MPEG_5_0_D,
CoreAudio_Layout_AAC_5_1 = CoreAudio_Layout_MPEG_5_1_D,
CoreAudio_Layout_AAC_6_0 = (141<<16) | 6, 
CoreAudio_Layout_AAC_6_1 = (142<<16) | 7, 
CoreAudio_Layout_AAC_7_0 = (143<<16) | 7, 
CoreAudio_Layout_AAC_7_1 = CoreAudio_Layout_MPEG_7_1_B,
CoreAudio_Layout_AAC_7_1_B = (183<<16) | 8, 
CoreAudio_Layout_AAC_Octagonal = (144<<16) | 8, 

CoreAudio_Layout_TMH_10_2_std = (145<<16) | 16, 
CoreAudio_Layout_TMH_10_2_full = (146<<16) | 21, 

CoreAudio_Layout_AC3_1_0_1 = (149<<16) | 2, 
CoreAudio_Layout_AC3_3_0 = (150<<16) | 3, 
CoreAudio_Layout_AC3_3_1 = (151<<16) | 4, 
CoreAudio_Layout_AC3_3_0_1 = (152<<16) | 4, 
CoreAudio_Layout_AC3_2_1_1 = (153<<16) | 4, 
CoreAudio_Layout_AC3_3_1_1 = (154<<16) | 5, 

CoreAudio_Layout_EAC_6_0_A = (155<<16) | 6, 
CoreAudio_Layout_EAC_7_0_A = (156<<16) | 7, 

CoreAudio_Layout_EAC3_6_1_A = (157<<16) | 7, 
CoreAudio_Layout_EAC3_6_1_B = (158<<16) | 7, 
CoreAudio_Layout_EAC3_6_1_C = (159<<16) | 7, 
CoreAudio_Layout_EAC3_7_1_A = (160<<16) | 8, 
CoreAudio_Layout_EAC3_7_1_B = (161<<16) | 8, 
CoreAudio_Layout_EAC3_7_1_C = (162<<16) | 8, 
CoreAudio_Layout_EAC3_7_1_D = (163<<16) | 8, 
CoreAudio_Layout_EAC3_7_1_E = (164<<16) | 8, 

CoreAudio_Layout_EAC3_7_1_F = (165<<16) | 8,
CoreAudio_Layout_EAC3_7_1_G = (166<<16) | 8,
CoreAudio_Layout_EAC3_7_1_H = (167<<16) | 8,

CoreAudio_Layout_DTS_3_1 = (168<<16) | 4,
CoreAudio_Layout_DTS_4_1 = (169<<16) | 5,
CoreAudio_Layout_DTS_6_0_A = (170<<16) | 6,
CoreAudio_Layout_DTS_6_0_B = (171<<16) | 6,
CoreAudio_Layout_DTS_6_0_C = (172<<16) | 6,
CoreAudio_Layout_DTS_6_1_A = (173<<16) | 7,
CoreAudio_Layout_DTS_6_1_B = (174<<16) | 7,
CoreAudio_Layout_DTS_6_1_C = (175<<16) | 7,
CoreAudio_Layout_DTS_7_0 = (176<<16) | 7,
CoreAudio_Layout_DTS_7_1 = (177<<16) | 8,
CoreAudio_Layout_DTS_8_0_A = (178<<16) | 8,
CoreAudio_Layout_DTS_8_0_B = (179<<16) | 8,
CoreAudio_Layout_DTS_8_1_A = (180<<16) | 9,
CoreAudio_Layout_DTS_8_1_B = (181<<16) | 9,
CoreAudio_Layout_DTS_6_1_D = (182<<16) | 7,

CoreAudio_Layout_DiscreteInOrder = (147<<16) | 0,
CoreAudio_Layout_Unknown = 0xFFFF0000
};

static inline int CoreAudio_Bitmap_to_vlc_bitmap( const struct CoreAudio_layout_s *c,
uint16_t *pi_mapping,
uint8_t *pi_channels,
const uint32_t **pp_chans_order )
{
*pp_chans_order = pi_vlc_chan_order_CoreAudio;
*pi_mapping = 0;
*pi_channels = 0;
for (uint8_t i=0;i<ARRAY_SIZE(CoreAudio_Bitmap_mapping);i++)
{
if ( CoreAudio_Bitmap_mapping[i].i_bitmap & c->i_channels_bitmap )
{
if ( (CoreAudio_Bitmap_mapping[i].i_vlc_bitmap & *pi_mapping) ||
*pi_channels >= AOUT_CHAN_MAX )
{

*pi_mapping = 0;
*pi_channels = 0;
return VLC_EGENERIC;
}
*pi_mapping |= CoreAudio_Bitmap_mapping[i].i_vlc_bitmap;
}
}
return VLC_SUCCESS;
}

static const uint32_t pi_vlc_chan_order_C[] = {
AOUT_CHAN_LEFT, AOUT_CHAN_CENTER, AOUT_CHAN_RIGHT,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARCENTER, AOUT_CHAN_REARRIGHT,
AOUT_CHAN_LFE,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
0
};

static const uint32_t pi_vlc_chan_order_B[] = {
AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARRIGHT,
AOUT_CHAN_CENTER,
AOUT_CHAN_LFE,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
AOUT_CHAN_REARCENTER,
0
};

static const uint32_t pi_vlc_chan_order_AAC[] = {
AOUT_CHAN_CENTER, AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARRIGHT, AOUT_CHAN_REARCENTER,
AOUT_CHAN_LFE, 0
};

static const uint32_t pi_vlc_chan_order_EAC[] = {
AOUT_CHAN_LEFT, AOUT_CHAN_CENTER, AOUT_CHAN_RIGHT,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
AOUT_CHAN_LFE,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARCENTER, AOUT_CHAN_REARRIGHT,
0
};

static const uint32_t pi_vlc_chan_order_DTS[] = {
AOUT_CHAN_LEFT, AOUT_CHAN_CENTER, AOUT_CHAN_RIGHT,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARCENTER, AOUT_CHAN_REARRIGHT,
AOUT_CHAN_LFE,
0
};

static const uint32_t pi_vlc_chan_order_DTS_C[] = {
AOUT_CHAN_CENTER, AOUT_CHAN_REARCENTER,
AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT,
AOUT_CHAN_REARLEFT, AOUT_CHAN_REARRIGHT,
AOUT_CHAN_LFE,
0
};

static const struct
{
enum CoreAudio_Layout layout;
const uint32_t *p_chans_order;
uint16_t i_vlc_bitmap;
} CoreAudio_Layout_mapping[] = {
{ CoreAudio_Layout_Mono, NULL, AOUT_CHAN_CENTER },
{ CoreAudio_Layout_Stereo, NULL, AOUT_CHANS_STEREO },
{ CoreAudio_Layout_StereoHeadphones, NULL, AOUT_CHANS_STEREO },
{ CoreAudio_Layout_Binaural, NULL, AOUT_CHANS_STEREO },


{ CoreAudio_Layout_Quadraphonic, NULL, AOUT_CHANS_4_0_MIDDLE },
{ CoreAudio_Layout_Pentagonal, pi_vlc_chan_order_B, AOUT_CHANS_4_0 | AOUT_CHAN_CENTER },




{ CoreAudio_Layout_MPEG_3_0_A, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_3_0 },
{ CoreAudio_Layout_MPEG_3_0_B, NULL, AOUT_CHANS_3_0 },
{ CoreAudio_Layout_MPEG_4_0_A, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_3_0 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_MPEG_4_0_B, NULL, AOUT_CHANS_3_0 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_MPEG_5_0_A, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_5_0 },
{ CoreAudio_Layout_MPEG_5_0_B, pi_vlc_chan_order_B, AOUT_CHANS_5_0 },
{ CoreAudio_Layout_MPEG_5_0_C, pi_vlc_chan_order_C, AOUT_CHANS_5_0 },
{ CoreAudio_Layout_MPEG_5_0_D, NULL, AOUT_CHANS_5_0 },
{ CoreAudio_Layout_MPEG_5_1_A, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_5_1 },
{ CoreAudio_Layout_MPEG_5_1_B, pi_vlc_chan_order_B, AOUT_CHANS_5_1 },
{ CoreAudio_Layout_MPEG_5_1_C, pi_vlc_chan_order_C, AOUT_CHANS_5_1 },
{ CoreAudio_Layout_MPEG_5_1_D, NULL, AOUT_CHANS_5_1 },
{ CoreAudio_Layout_MPEG_6_1_A, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_5_1 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_MPEG_7_1_A, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_7_1 },
{ CoreAudio_Layout_MPEG_7_1_B, pi_vlc_chan_order_AAC, AOUT_CHANS_7_1 },
{ CoreAudio_Layout_MPEG_7_1_C, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_7_1 },
{ CoreAudio_Layout_Emagic_Default_7_1, pi_vlc_chan_order_B, AOUT_CHANS_7_1 },

{ CoreAudio_Layout_ITU_2_1, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_3_0 },
{ CoreAudio_Layout_ITU_2_2, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_4_0 },

{ CoreAudio_Layout_DVD_4, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_2_1 },
{ CoreAudio_Layout_DVD_5, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_2_1 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_DVD_6, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_2_1 | AOUT_CHANS_REAR },
{ CoreAudio_Layout_DVD_10, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_3_1 },
{ CoreAudio_Layout_DVD_11, pi_vlc_chan_order_CoreAudio, AOUT_CHANS_3_1 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_DVD_18, pi_vlc_chan_order_B, AOUT_CHANS_5_1 },

{ CoreAudio_Layout_AudioUnit_6_0, pi_vlc_chan_order_B, AOUT_CHANS_5_0 | AOUT_CHAN_REARCENTER },

{ CoreAudio_Layout_AudioUnit_7_0_Front, pi_vlc_chan_order_B, AOUT_CHANS_5_0 | AOUT_CHANS_MIDDLE },

{ CoreAudio_Layout_AAC_6_0, pi_vlc_chan_order_AAC, AOUT_CHANS_5_0 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_AAC_6_1, pi_vlc_chan_order_AAC, AOUT_CHANS_5_0 | AOUT_CHAN_REARCENTER | AOUT_CHAN_LFE },
{ CoreAudio_Layout_AAC_7_0, pi_vlc_chan_order_AAC, AOUT_CHANS_5_0 | AOUT_CHANS_MIDDLE },
{ CoreAudio_Layout_AAC_7_1_B, pi_vlc_chan_order_AAC, AOUT_CHANS_7_1 },
{ CoreAudio_Layout_AAC_Octagonal, pi_vlc_chan_order_AAC, AOUT_CHANS_7_0 | AOUT_CHAN_REARCENTER },




{ CoreAudio_Layout_AC3_1_0_1, pi_vlc_chan_order_C, AOUT_CHAN_CENTER | AOUT_CHAN_LFE },
{ CoreAudio_Layout_AC3_3_0, pi_vlc_chan_order_C, AOUT_CHANS_3_0 },
{ CoreAudio_Layout_AC3_3_1, pi_vlc_chan_order_C, AOUT_CHANS_3_0 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_AC3_3_0_1, pi_vlc_chan_order_C, AOUT_CHANS_3_0 | AOUT_CHAN_LFE },
{ CoreAudio_Layout_AC3_2_1_1, pi_vlc_chan_order_C, AOUT_CHANS_2_1 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_AC3_3_1_1, pi_vlc_chan_order_C, AOUT_CHANS_3_1 | AOUT_CHAN_REARCENTER },

{ CoreAudio_Layout_EAC_6_0_A, pi_vlc_chan_order_C, AOUT_CHANS_4_0 | AOUT_CHAN_CENTER | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_EAC_7_0_A, pi_vlc_chan_order_C, AOUT_CHANS_6_0 | AOUT_CHAN_CENTER },

{ CoreAudio_Layout_EAC3_6_1_A, pi_vlc_chan_order_EAC, AOUT_CHANS_4_0 | AOUT_CHAN_CENTER | AOUT_CHAN_REARCENTER | AOUT_CHAN_LFE },



{ CoreAudio_Layout_EAC3_7_1_B, pi_vlc_chan_order_EAC, AOUT_CHANS_7_1 },








{ CoreAudio_Layout_DTS_3_1, NULL, AOUT_CHANS_3_1 },
{ CoreAudio_Layout_DTS_4_1, NULL, AOUT_CHANS_3_1 | AOUT_CHAN_REARCENTER },
{ CoreAudio_Layout_DTS_6_0_A, NULL, AOUT_CHANS_6_0 },

{ CoreAudio_Layout_DTS_6_0_C, pi_vlc_chan_order_DTS_C, AOUT_CHANS_6_0 },
{ CoreAudio_Layout_DTS_6_1_A, NULL, AOUT_CHANS_6_1_MIDDLE },

{ CoreAudio_Layout_DTS_6_1_C, pi_vlc_chan_order_DTS_C, AOUT_CHANS_6_1_MIDDLE },
{ CoreAudio_Layout_DTS_7_0, pi_vlc_chan_order_DTS, AOUT_CHANS_6_0 | AOUT_CHAN_CENTER },
{ CoreAudio_Layout_DTS_7_1, pi_vlc_chan_order_DTS, AOUT_CHANS_7_1 },

{ CoreAudio_Layout_DTS_8_0_B, pi_vlc_chan_order_DTS, AOUT_CHANS_5_0 | AOUT_CHANS_MIDDLE | AOUT_CHAN_REARCENTER },



};

static inline int CoreAudio_Layout_to_vlc( const struct CoreAudio_layout_s *c,
uint16_t *pi_bitmap,
uint8_t *pi_channels,
const uint32_t **pp_chans_order )
{
if( c->i_channels_layout_tag == CoreAudio_Layout_BITMAP )
return CoreAudio_Bitmap_to_vlc_bitmap( c, pi_bitmap, pi_channels, pp_chans_order );

for (size_t i=0;i<ARRAY_SIZE(CoreAudio_Layout_mapping);i++)
{
if(CoreAudio_Layout_mapping[i].layout == c->i_channels_layout_tag )
{
*pi_bitmap = CoreAudio_Layout_mapping[i].i_vlc_bitmap;
*pp_chans_order = CoreAudio_Layout_mapping[i].p_chans_order;
*pi_channels = c->i_channels_layout_tag & 0xFF;
break;
}
}
return VLC_SUCCESS;
}
