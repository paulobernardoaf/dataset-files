#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_charset.h>
#include "substext.h"
#include "cea708.h"
#if 0
#define Debug(code) code
#else
#define Debug(code)
#endif
static int Open ( vlc_object_t * );
static void Close( vlc_object_t * );
#define OPAQUE_TEXT N_("Opacity")
#define OPAQUE_LONGTEXT N_("Setting to true " "makes the text to be boxed and maybe easier to read." )
vlc_module_begin ()
set_shortname( N_("CC 608/708"))
set_description( N_("Closed Captions decoder") )
set_capability( "spu decoder", 50 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_SCODEC )
set_callbacks( Open, Close )
add_bool( "cc-opaque", true,
OPAQUE_TEXT, OPAQUE_LONGTEXT, false )
vlc_module_end ()
typedef enum
{
EIA608_MODE_POPUP = 0,
EIA608_MODE_ROLLUP_2 = 1,
EIA608_MODE_ROLLUP_3 = 2,
EIA608_MODE_ROLLUP_4 = 3,
EIA608_MODE_PAINTON = 4,
EIA608_MODE_TEXT = 5
} eia608_mode_t;
typedef enum
{
EIA608_COLOR_WHITE = 0,
EIA608_COLOR_GREEN = 1,
EIA608_COLOR_BLUE = 2,
EIA608_COLOR_CYAN = 3,
EIA608_COLOR_RED = 4,
EIA608_COLOR_YELLOW = 5,
EIA608_COLOR_MAGENTA = 6,
EIA608_COLOR_USERDEFINED = 7
} eia608_color_t;
typedef enum
{
EIA608_FONT_REGULAR = 0x00,
EIA608_FONT_ITALICS = 0x01,
EIA608_FONT_UNDERLINE = 0x02,
EIA608_FONT_UNDERLINE_ITALICS = EIA608_FONT_UNDERLINE | EIA608_FONT_ITALICS
} eia608_font_t;
#define EIA608_SCREEN_ROWS 15
#define EIA608_SCREEN_COLUMNS 32
#define EIA608_MARGIN 0.10
#define EIA608_VISIBLE (1.0 - EIA608_MARGIN * 2)
#define FONT_TO_LINE_HEIGHT_RATIO 1.06
struct eia608_screen 
{
uint8_t characters[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1];
eia608_color_t colors[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1];
eia608_font_t fonts[EIA608_SCREEN_ROWS][EIA608_SCREEN_COLUMNS+1]; 
int row_used[EIA608_SCREEN_ROWS]; 
};
typedef struct eia608_screen eia608_screen;
typedef enum
{
EIA608_STATUS_DEFAULT = 0x00,
EIA608_STATUS_CHANGED = 0x01, 
EIA608_STATUS_CAPTION_ENDED = 0x02, 
EIA608_STATUS_CAPTION_CLEARED = 0x04, 
EIA608_STATUS_DISPLAY = EIA608_STATUS_CAPTION_CLEARED | EIA608_STATUS_CAPTION_ENDED,
} eia608_status_t;
static const struct {
eia608_color_t i_color;
eia608_font_t i_font;
int i_column;
} pac2_attribs[]= {
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_GREEN, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_GREEN, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_BLUE, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_BLUE, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_CYAN, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_CYAN, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_RED, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_RED, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_YELLOW, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_YELLOW, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_MAGENTA, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_MAGENTA, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_WHITE, EIA608_FONT_ITALICS, 0 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE_ITALICS, 0 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 0 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 0 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 4 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 4 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 8 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 8 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 12 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 12 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 16 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 16 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 20 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 20 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 24 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 24 },
{ EIA608_COLOR_WHITE, EIA608_FONT_REGULAR, 28 },
{ EIA608_COLOR_WHITE, EIA608_FONT_UNDERLINE, 28 } ,
};
#define EIA608_COLOR_DEFAULT EIA608_COLOR_WHITE
static const int rgi_eia608_colors[] = {
0xffffff, 
0x00ff00, 
0x0000ff, 
0x00ffff, 
0xff0000, 
0xffff00, 
0xff00ff, 
0xffffff, 
};
typedef struct
{
int i_channel;
int i_screen; 
eia608_screen screen[2];
struct
{
int i_row;
int i_column;
} cursor;
eia608_mode_t mode;
eia608_color_t color;
eia608_font_t font;
int i_row_rollup;
struct
{
uint8_t d1;
uint8_t d2;
} last;
} eia608_t;
static void Eia608Init( eia608_t * );
static eia608_status_t Eia608Parse( eia608_t *h, int i_channel_selected, const uint8_t data[2] );
static void Eia608FillUpdaterRegions( subtext_updater_sys_t *p_updater, eia608_t *h );
#define CC_MAX_REORDER_SIZE (64)
typedef struct
{
int i_queue;
block_t *p_queue;
int i_field;
int i_channel;
int i_reorder_depth;
cea708_demux_t *p_dtvcc;
cea708_t *p_cea708;
eia608_t *p_eia608;
bool b_opaque;
} decoder_sys_t;
static int Decode( decoder_t *, block_t * );
static void Flush( decoder_t * );
static void DTVCC_ServiceData_Handler( void *priv, uint8_t i_sid, vlc_tick_t i_time,
const uint8_t *p_data, size_t i_data )
{
decoder_t *p_dec = priv;
decoder_sys_t *p_sys = p_dec->p_sys;
if( i_sid == 1 + p_dec->fmt_in.subs.cc.i_channel )
CEA708_Decoder_Push( p_sys->p_cea708, i_time, p_data, i_data );
}
static int Open( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
if( ( p_dec->fmt_in.i_codec != VLC_CODEC_CEA608 ||
p_dec->fmt_in.subs.cc.i_channel > 3 ) &&
( p_dec->fmt_in.i_codec != VLC_CODEC_CEA708 ||
p_dec->fmt_in.subs.cc.i_channel > 63 ) )
return VLC_EGENERIC;
p_dec->pf_decode = Decode;
p_dec->pf_flush = Flush;
p_dec->p_sys = p_sys = calloc( 1, sizeof( *p_sys ) );
if( p_sys == NULL )
return VLC_ENOMEM;
if( p_dec->fmt_in.i_codec == VLC_CODEC_CEA608 )
{
p_sys->i_field = p_dec->fmt_in.subs.cc.i_channel >> 1;
p_sys->i_channel = 1 + (p_dec->fmt_in.subs.cc.i_channel & 1);
p_sys->p_eia608 = malloc(sizeof(*p_sys->p_eia608));
if( !p_sys->p_eia608 )
{
free( p_sys );
return VLC_ENOMEM;
}
Eia608Init( p_sys->p_eia608 );
}
else
{
p_sys->p_dtvcc = CEA708_DTVCC_Demuxer_New( p_dec, DTVCC_ServiceData_Handler );
if( !p_sys->p_dtvcc )
{
free( p_sys );
return VLC_ENOMEM;
}
p_sys->p_cea708 = CEA708_Decoder_New( p_dec );
if( !p_sys->p_cea708 )
{
CEA708_DTVCC_Demuxer_Release( p_sys->p_dtvcc );
free( p_sys );
return VLC_ENOMEM;
}
p_sys->i_channel = p_dec->fmt_in.subs.cc.i_channel;
}
p_sys->b_opaque = var_InheritBool( p_dec, "cc-opaque" );
p_sys->i_reorder_depth = p_dec->fmt_in.subs.cc.i_reorder_depth;
p_dec->fmt_out.i_codec = VLC_CODEC_TEXT;
return VLC_SUCCESS;
}
static void Flush( decoder_t *p_dec )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_sys->p_eia608 )
{
Eia608Init( p_sys->p_eia608 );
}
else
{
CEA708_DTVCC_Demuxer_Flush( p_sys->p_dtvcc );
CEA708_Decoder_Flush( p_sys->p_cea708 );
}
block_ChainRelease( p_sys->p_queue );
p_sys->p_queue = NULL;
p_sys->i_queue = 0;
}
static void Push( decoder_t *, block_t * );
static block_t *Pop( decoder_t *, bool );
static void Convert( decoder_t *, vlc_tick_t, const uint8_t *, size_t );
static bool DoDecode( decoder_t *p_dec, bool b_drain )
{
block_t *p_block = Pop( p_dec, b_drain );
if( !p_block )
return false;
Convert( p_dec, p_block->i_pts, p_block->p_buffer, p_block->i_buffer );
block_Release( p_block );
return true;
}
static int Decode( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_block )
{
if( p_block->i_flags & (BLOCK_FLAG_DISCONTINUITY | BLOCK_FLAG_CORRUPTED) )
{
for( ; DoDecode( p_dec, true ) ; );
if( p_sys->p_eia608 )
{
Eia608Init( p_sys->p_eia608 );
}
else
{
CEA708_DTVCC_Demuxer_Flush( p_sys->p_dtvcc );
CEA708_Decoder_Flush( p_sys->p_cea708 );
}
if( (p_block->i_flags & BLOCK_FLAG_CORRUPTED) || p_block->i_buffer < 1 )
{
block_Release( p_block );
return VLCDEC_SUCCESS;
}
}
if( p_sys->i_reorder_depth == 0 )
{
if( (p_block->i_flags & BLOCK_FLAG_TYPE_B) == 0 )
for( ; DoDecode( p_dec, true ); );
}
Push( p_dec, p_block );
}
const bool b_no_reorder = (p_dec->fmt_in.subs.cc.i_reorder_depth < 0);
for( ; DoDecode( p_dec, (p_block == NULL) || b_no_reorder ); );
return VLCDEC_SUCCESS;
}
static void Close( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
free( p_sys->p_eia608 );
if( p_sys->p_cea708 )
{
CEA708_Decoder_Release( p_sys->p_cea708 );
CEA708_DTVCC_Demuxer_Release( p_sys->p_dtvcc );
}
block_ChainRelease( p_sys->p_queue );
free( p_sys );
}
static void Push( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
if( p_sys->i_queue >= CC_MAX_REORDER_SIZE )
{
block_Release( Pop( p_dec, true ) );
msg_Warn( p_dec, "Trashing a CC entry" );
}
block_t **pp_block;
for( pp_block = &p_sys->p_queue; *pp_block ; pp_block = &((*pp_block)->p_next) )
{
if( p_block->i_pts == VLC_TICK_INVALID || (*pp_block)->i_pts == VLC_TICK_INVALID )
continue;
if( p_block->i_pts < (*pp_block)->i_pts )
{
if( p_sys->i_reorder_depth > 0 &&
p_sys->i_queue < p_sys->i_reorder_depth &&
pp_block == &p_sys->p_queue )
{
msg_Info( p_dec, "Increasing reorder depth to %d", ++p_sys->i_reorder_depth );
}
break;
}
}
p_block->p_next = *pp_block ? *pp_block : NULL;
*pp_block = p_block;
p_sys->i_queue++;
}
static block_t *Pop( decoder_t *p_dec, bool b_forced )
{
decoder_sys_t *p_sys = p_dec->p_sys;
block_t *p_block;
if( p_sys->i_queue == 0 )
return NULL;
if( !b_forced && p_sys->i_queue < CC_MAX_REORDER_SIZE )
{
if( p_sys->i_queue < p_sys->i_reorder_depth || p_sys->i_reorder_depth == 0 )
return NULL;
}
p_block = p_sys->p_queue;
p_sys->p_queue = p_block->p_next;
p_block->p_next = NULL;
p_sys->i_queue--;
return p_block;
}
static subpicture_t *Subtitle( decoder_t *p_dec, eia608_t *h, vlc_tick_t i_pts )
{
subpicture_t *p_spu = NULL;
if( i_pts == VLC_TICK_INVALID )
return NULL;
p_spu = decoder_NewSubpictureText( p_dec );
if( !p_spu )
return NULL;
p_spu->i_start = i_pts;
p_spu->i_stop = i_pts + VLC_TICK_FROM_SEC(10); 
p_spu->b_ephemer = true;
p_spu->b_absolute = false;
subtext_updater_sys_t *p_spu_sys = p_spu->updater.p_sys;
decoder_sys_t *p_dec_sys = p_dec->p_sys;
p_spu_sys->region.align = SUBPICTURE_ALIGN_TOP|SUBPICTURE_ALIGN_LEFT;
p_spu_sys->region.inner_align = SUBPICTURE_ALIGN_BOTTOM|SUBPICTURE_ALIGN_LEFT;
p_spu_sys->region.flags = UPDT_REGION_IGNORE_BACKGROUND | UPDT_REGION_USES_GRID_COORDINATES;
p_spu_sys->p_default_style->i_style_flags |= STYLE_MONOSPACED;
if( p_dec_sys->b_opaque )
{
p_spu_sys->p_default_style->i_background_alpha = STYLE_ALPHA_OPAQUE;
p_spu_sys->p_default_style->i_features |= STYLE_HAS_BACKGROUND_ALPHA;
p_spu_sys->p_default_style->i_style_flags |= STYLE_BACKGROUND;
}
p_spu_sys->margin_ratio = EIA608_MARGIN;
p_spu_sys->p_default_style->i_font_color = rgi_eia608_colors[EIA608_COLOR_DEFAULT];
p_spu_sys->p_default_style->f_font_relsize = EIA608_VISIBLE * 100 / EIA608_SCREEN_ROWS /
FONT_TO_LINE_HEIGHT_RATIO;
p_spu_sys->p_default_style->i_features |= (STYLE_HAS_FONT_COLOR | STYLE_HAS_FLAGS);
Eia608FillUpdaterRegions( p_spu_sys, h );
return p_spu;
}
static void Convert( decoder_t *p_dec, vlc_tick_t i_pts,
const uint8_t *p_buffer, size_t i_buffer )
{
decoder_sys_t *p_sys = p_dec->p_sys;
unsigned i_ticks = 0;
while( i_buffer >= 3 )
{
if( (p_buffer[0] & 0x04) )
{
const vlc_tick_t i_spupts = i_pts + vlc_tick_from_samples(i_ticks, 1200/3);
if ( p_sys->p_eia608 &&
(p_buffer[0] & 0x03) == p_sys->i_field )
{
eia608_status_t i_status = Eia608Parse( p_sys->p_eia608,
p_sys->i_channel, &p_buffer[1] );
if( i_status & (EIA608_STATUS_DISPLAY | EIA608_STATUS_CHANGED) )
{
Debug(printf("\n"));
subpicture_t *p_spu = Subtitle( p_dec, p_sys->p_eia608, i_spupts );
if( p_spu )
decoder_QueueSub( p_dec, p_spu );
}
}
else if( p_sys->p_cea708 && (p_buffer[0] & 0x03) >= 2 )
{
CEA708_DTVCC_Demuxer_Push( p_sys->p_dtvcc, i_spupts, p_buffer );
}
}
i_ticks++;
i_buffer -= 3;
p_buffer += 3;
}
}
static void Eia608Cursor( eia608_t *h, int dx )
{
h->cursor.i_column += dx;
if( h->cursor.i_column < 0 )
h->cursor.i_column = 0;
else if( h->cursor.i_column > EIA608_SCREEN_COLUMNS-1 )
h->cursor.i_column = EIA608_SCREEN_COLUMNS-1;
}
static void Eia608ClearScreenRowX( eia608_t *h, int i_screen, int i_row, int x )
{
eia608_screen *screen = &h->screen[i_screen];
if( x == 0 )
{
screen->row_used[i_row] = false;
}
else
{
screen->row_used[i_row] = false;
for( int i = 0; i < x; i++ )
{
if( screen->characters[i_row][i] != ' ' ||
screen->colors[i_row][i] != EIA608_COLOR_DEFAULT ||
screen->fonts[i_row][i] != EIA608_FONT_REGULAR )
{
screen->row_used[i_row] = true;
break;
}
}
}
for( ; x < EIA608_SCREEN_COLUMNS+1; x++ )
{
screen->characters[i_row][x] = x < EIA608_SCREEN_COLUMNS ? ' ' : '\0';
screen->colors[i_row][x] = EIA608_COLOR_DEFAULT;
screen->fonts[i_row][x] = EIA608_FONT_REGULAR;
}
}
static void Eia608ClearScreenRow( eia608_t *h, int i_screen, int i_row )
{
Eia608ClearScreenRowX( h, i_screen, i_row, 0 );
}
static void Eia608ClearScreen( eia608_t *h, int i_screen )
{
for( int i = 0; i < EIA608_SCREEN_ROWS; i++ )
Eia608ClearScreenRow( h, i_screen, i );
}
static int Eia608GetWritingScreenIndex( eia608_t *h )
{
switch( h->mode )
{
case EIA608_MODE_POPUP: 
return 1 - h->i_screen;
case EIA608_MODE_ROLLUP_2: 
case EIA608_MODE_ROLLUP_3:
case EIA608_MODE_ROLLUP_4:
case EIA608_MODE_PAINTON:
return h->i_screen;
default:
vlc_assert_unreachable();
return 0;
}
}
static void Eia608EraseScreen( eia608_t *h, bool b_displayed )
{
Eia608ClearScreen( h, b_displayed ? h->i_screen : (1-h->i_screen) );
}
static void Eia608Write( eia608_t *h, const uint8_t c )
{
const int i_row = h->cursor.i_row;
const int i_column = h->cursor.i_column;
eia608_screen *screen;
if( h->mode == EIA608_MODE_TEXT )
return;
screen = &h->screen[Eia608GetWritingScreenIndex( h )];
screen->characters[i_row][i_column] = c;
screen->colors[i_row][i_column] = h->color;
screen->fonts[i_row][i_column] = h->font;
screen->row_used[i_row] = true;
Eia608Cursor( h, 1 );
}
static void Eia608Erase( eia608_t *h )
{
const int i_row = h->cursor.i_row;
const int i_column = h->cursor.i_column - 1;
eia608_screen *screen;
if( h->mode == EIA608_MODE_TEXT )
return;
if( i_column < 0 )
return;
screen = &h->screen[Eia608GetWritingScreenIndex( h )];
screen->characters[i_row][i_column] = ' ';
Eia608Cursor( h, -1 );
}
static void Eia608EraseToEndOfRow( eia608_t *h )
{
if( h->mode == EIA608_MODE_TEXT )
return;
Eia608ClearScreenRowX( h, Eia608GetWritingScreenIndex( h ), h->cursor.i_row, h->cursor.i_column );
}
static void Eia608RollUp( eia608_t *h )
{
if( h->mode == EIA608_MODE_TEXT )
return;
const int i_screen = Eia608GetWritingScreenIndex( h );
eia608_screen *screen = &h->screen[i_screen];
int keep_lines;
if( h->mode == EIA608_MODE_ROLLUP_2 )
keep_lines = 2;
else if( h->mode == EIA608_MODE_ROLLUP_3 )
keep_lines = 3;
else if( h->mode == EIA608_MODE_ROLLUP_4 )
keep_lines = 4;
else
return;
h->cursor.i_column = 0;
for( int i = 0; i < h->cursor.i_row - keep_lines; i++ )
Eia608ClearScreenRow( h, i_screen, i );
for( int i = 0; i < keep_lines-1; i++ )
{
const int i_row = h->cursor.i_row - keep_lines + i + 1;
if( i_row < 0 )
continue;
assert( i_row+1 < EIA608_SCREEN_ROWS );
memcpy( screen->characters[i_row], screen->characters[i_row+1], sizeof(*screen->characters) );
memcpy( screen->colors[i_row], screen->colors[i_row+1], sizeof(*screen->colors) );
memcpy( screen->fonts[i_row], screen->fonts[i_row+1], sizeof(*screen->fonts) );
screen->row_used[i_row] = screen->row_used[i_row+1];
}
Eia608ClearScreenRow( h, i_screen, h->cursor.i_row );
}
static void Eia608ParseChannel( eia608_t *h, const uint8_t d[2] )
{
static const int p4[16] = {
0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};
if( p4[d[0] & 0xf] == p4[d[0] >> 4] ||
p4[d[1] & 0xf] == p4[ d[1] >> 4] )
{
h->i_channel = -1;
return;
}
const int d1 = d[0] & 0x7f;
if( d1 >= 0x10 && d1 <= 0x1f )
h->i_channel = 1 + ((d1 & 0x08) != 0);
else if( d1 < 0x10 )
h->i_channel = 3;
}
static eia608_status_t Eia608ParseTextAttribute( eia608_t *h, uint8_t d2 )
{
const int i_index = d2 - 0x20;
assert( d2 >= 0x20 && d2 <= 0x2f );
Debug(printf("[TA %d]", i_index));
h->color = pac2_attribs[i_index].i_color;
h->font = pac2_attribs[i_index].i_font;
Eia608Cursor( h, 1 );
return EIA608_STATUS_DEFAULT;
}
static eia608_status_t Eia608ParseSingle( eia608_t *h, const uint8_t dx )
{
assert( dx >= 0x20 );
Eia608Write( h, dx );
return EIA608_STATUS_CHANGED;
}
static eia608_status_t Eia608ParseDouble( eia608_t *h, uint8_t d2 )
{
assert( d2 >= 0x30 && d2 <= 0x3f );
Debug(printf("\033[0;33m%s\033[0m", d2 + 0x50));
Eia608Write( h, d2 + 0x50 ); 
return EIA608_STATUS_CHANGED;
}
static eia608_status_t Eia608ParseExtended( eia608_t *h, uint8_t d1, uint8_t d2 )
{
assert( d2 >= 0x20 && d2 <= 0x3f );
assert( d1 == 0x12 || d1 == 0x13 );
if( d1 == 0x12 )
d2 += 0x70; 
else
d2 += 0x90; 
Debug(printf("[EXT %x->'%c']", d2, d2));
Eia608Cursor( h, -1 );
Eia608Write( h, d2 );
return EIA608_STATUS_CHANGED;
}
static eia608_status_t Eia608ParseCommand0x14( eia608_t *h, uint8_t d2 )
{
eia608_status_t i_status = EIA608_STATUS_DEFAULT;
eia608_mode_t proposed_mode;
switch( d2 )
{
case 0x20: 
Debug(printf("[RCL]"));
h->mode = EIA608_MODE_POPUP;
break;
case 0x21: 
Debug(printf("[BS]"));
Eia608Erase( h );
i_status = EIA608_STATUS_CHANGED;
break;
case 0x22: 
case 0x23:
Debug(printf("[ALARM %d]", d2 - 0x22));
break;
case 0x24: 
Debug(printf("[DER]"));
Eia608EraseToEndOfRow( h );
break;
case 0x25: 
case 0x26: 
case 0x27: 
Debug(printf("[RU%d]", d2 - 0x23));
if( h->mode == EIA608_MODE_POPUP || h->mode == EIA608_MODE_PAINTON )
{
Eia608EraseScreen( h, true );
Eia608EraseScreen( h, false );
i_status = EIA608_STATUS_CHANGED | EIA608_STATUS_CAPTION_CLEARED;
}
if( d2 == 0x25 )
proposed_mode = EIA608_MODE_ROLLUP_2;
else if( d2 == 0x26 )
proposed_mode = EIA608_MODE_ROLLUP_3;
else
proposed_mode = EIA608_MODE_ROLLUP_4;
if ( proposed_mode != h->mode )
{
h->mode = proposed_mode;
h->cursor.i_column = 0;
h->cursor.i_row = h->i_row_rollup;
}
break;
case 0x28: 
Debug(printf("[FON]"));
break;
case 0x29: 
Debug(printf("[RDC]"));
h->mode = EIA608_MODE_PAINTON;
break;
case 0x2a: 
Debug(printf("[TR]"));
break;
case 0x2b: 
Debug(printf("[RTD]"));
h->mode = EIA608_MODE_TEXT;
break;
case 0x2c: 
Debug(printf("[EDM]"));
Eia608EraseScreen( h, true );
i_status = EIA608_STATUS_CHANGED | EIA608_STATUS_CAPTION_CLEARED;
break;
case 0x2d: 
Debug(printf("[CR]"));
Eia608RollUp(h);
i_status = EIA608_STATUS_CHANGED;
break;
case 0x2e: 
Debug(printf("[ENM]"));
Eia608EraseScreen( h, false );
break;
case 0x2f: 
Debug(printf("[EOC]"));
if( h->mode != EIA608_MODE_PAINTON )
h->i_screen = 1 - h->i_screen;
h->mode = EIA608_MODE_POPUP;
h->cursor.i_column = 0;
h->cursor.i_row = 0;
h->color = EIA608_COLOR_DEFAULT;
h->font = EIA608_FONT_REGULAR;
i_status = EIA608_STATUS_CHANGED | EIA608_STATUS_CAPTION_ENDED;
break;
}
return i_status;
}
static bool Eia608ParseCommand0x17( eia608_t *h, uint8_t d2 )
{
switch( d2 )
{
case 0x21: 
case 0x22: 
case 0x23: 
Debug(printf("[TO%d]", d2 - 0x20));
Eia608Cursor( h, d2 - 0x20 );
break;
}
return false;
}
static bool Eia608ParsePac( eia608_t *h, uint8_t d1, uint8_t d2 )
{
static const int pi_row[] = {
11, -1, 1, 2, 3, 4, 12, 13, 14, 15, 5, 6, 7, 8, 9, 10
};
const int i_row_index = ( (d1<<1) & 0x0e) | ( (d2>>5) & 0x01 );
Debug(printf("[PAC,%d]", i_row_index));
assert( d2 >= 0x40 && d2 <= 0x7f );
if( pi_row[i_row_index] <= 0 )
return false;
if( h->mode != EIA608_MODE_TEXT )
h->cursor.i_row = pi_row[i_row_index] - 1;
h->i_row_rollup = pi_row[i_row_index] - 1;
if( d2 >= 0x60 )
d2 -= 0x60;
else if( d2 >= 0x40 )
d2 -= 0x40;
h->cursor.i_column = pac2_attribs[d2].i_column;
h->color = pac2_attribs[d2].i_color;
h->font = pac2_attribs[d2].i_font;
return false;
}
static eia608_status_t Eia608ParseData( eia608_t *h, uint8_t d1, uint8_t d2 )
{
eia608_status_t i_status = EIA608_STATUS_DEFAULT;
if( d1 >= 0x18 && d1 <= 0x1f )
d1 -= 8;
#define ON( d2min, d2max, cmd ) do { if( d2 >= d2min && d2 <= d2max ) i_status = cmd; } while(0)
switch( d1 )
{
case 0x11:
ON( 0x20, 0x2f, Eia608ParseTextAttribute( h, d2 ) );
ON( 0x30, 0x3f, Eia608ParseDouble( h, d2 ) );
break;
case 0x12: case 0x13:
ON( 0x20, 0x3f, Eia608ParseExtended( h, d1, d2 ) );
break;
case 0x14: case 0x15:
ON( 0x20, 0x2f, Eia608ParseCommand0x14( h, d2 ) );
break;
case 0x17:
ON( 0x21, 0x23, Eia608ParseCommand0x17( h, d2 ) );
ON( 0x2e, 0x2f, Eia608ParseTextAttribute( h, d2 ) );
break;
}
if( d1 == 0x10 )
ON( 0x40, 0x5f, Eia608ParsePac( h, d1, d2 ) );
else if( d1 >= 0x11 && d1 <= 0x17 )
ON( 0x40, 0x7f, Eia608ParsePac( h, d1, d2 ) );
#undef ON
if( d1 >= 0x20 )
{
Debug(printf("\033[0;33m%c", d1));
i_status = Eia608ParseSingle( h, d1 );
if( d2 >= 0x20 )
{
Debug(printf("%c", d2));
i_status |= Eia608ParseSingle( h, d2 );
}
Debug(printf("\033[0m"));
}
if( h->mode == EIA608_MODE_POPUP && i_status == EIA608_STATUS_CHANGED )
i_status = EIA608_STATUS_DEFAULT;
return i_status;
}
static void Eia608TextUtf8( char *psz_utf8, uint8_t c ) 
{
#define E1(c,u) { c, { u, '\0' } }
#define E2(c,u1,u2) { c, { u1, u2, '\0' } }
#define E3(c,u1,u2,u3) { c, { u1, u2, u3, '\0' } }
static const struct {
uint8_t c;
char utf8[3+1];
} c2utf8[] = {
E2( 0x2a, 0xc3,0xa1), 
E2( 0x5c, 0xc3,0xa9), 
E2( 0x5e, 0xc3,0xad), 
E2( 0x5f, 0xc3,0xb3), 
E2( 0x60, 0xc3,0xba), 
E2( 0x7b, 0xc3,0xa7), 
E2( 0x7c, 0xc3,0xb7), 
E2( 0x7d, 0xc3,0x91), 
E2( 0x7e, 0xc3,0xb1), 
E2( 0x80, 0xc2,0xae), 
E2( 0x81, 0xc2,0xb0), 
E2( 0x82, 0xc2,0xbd), 
E2( 0x83, 0xc2,0xbf), 
E3( 0x84, 0xe2,0x84,0xa2), 
E2( 0x85, 0xc2,0xa2), 
E2( 0x86, 0xc2,0xa3), 
E3( 0x87, 0xe2,0x99,0xaa), 
E2( 0x88, 0xc3,0xa0), 
E2( 0x89, 0xc2,0xa0), 
E2( 0x8a, 0xc3,0xa8), 
E2( 0x8b, 0xc3,0xa2), 
E2( 0x8c, 0xc3,0xaa), 
E2( 0x8d, 0xc3,0xae), 
E2( 0x8e, 0xc3,0xb4), 
E2( 0x8f, 0xc3,0xbb), 
E2( 0x90, 0xc3,0x81), 
E2( 0x91, 0xc3,0x89), 
E2( 0x92, 0xc3,0x93), 
E2( 0x93, 0xc3,0x9a), 
E2( 0x94, 0xc3,0x9c), 
E2( 0x95, 0xc3,0xbc), 
E1( 0x96, 0x27), 
E2( 0x97, 0xc2,0xa1), 
E1( 0x98, 0x2a), 
E1( 0x99, 0x27), 
E1( 0x9a, 0x2d), 
E2( 0x9b, 0xc2,0xa9), 
E3( 0x9c, 0xe2,0x84,0xa0), 
E1( 0x9d, 0x2e), 
E3( 0x9e, 0xe2,0x80,0x9c), 
E3( 0x9f, 0xe2,0x80,0x9d), 
E2( 0xa0, 0xc3,0x80), 
E2( 0xa1, 0xc3,0x82), 
E2( 0xa2, 0xc3,0x87), 
E2( 0xa3, 0xc3,0x88), 
E2( 0xa4, 0xc3,0x8a), 
E2( 0xa5, 0xc3,0x8b), 
E2( 0xa6, 0xc3,0xab), 
E2( 0xa7, 0xc3,0x8e), 
E2( 0xa8, 0xc3,0x8f), 
E2( 0xa9, 0xc3,0xaf), 
E2( 0xaa, 0xc3,0x94), 
E2( 0xab, 0xc3,0x99), 
E2( 0xac, 0xc3,0xb9), 
E2( 0xad, 0xc3,0x9b), 
E2( 0xae, 0xc2,0xab), 
E2( 0xaf, 0xc2,0xbb), 
E2( 0xb0, 0xc3,0x83), 
E2( 0xb1, 0xc3,0xa3), 
E2( 0xb2, 0xc3,0x8d), 
E2( 0xb3, 0xc3,0x8c), 
E2( 0xb4, 0xc3,0xac), 
E2( 0xb5, 0xc3,0x92), 
E2( 0xb6, 0xc3,0xb2), 
E2( 0xb7, 0xc3,0x95), 
E2( 0xb8, 0xc3,0xb5), 
E1( 0xb9, 0x7b), 
E1( 0xba, 0x7d), 
E1( 0xbb, 0x5c), 
E1( 0xbc, 0x5e), 
E1( 0xbd, 0x5f), 
E2( 0xbe, 0xc2,0xa6), 
E1( 0xbf, 0x7e), 
E2( 0xc0, 0xc3,0x84), 
E2( 0xc1, 0xc3,0xa4), 
E2( 0xc2, 0xc3,0x96), 
E2( 0xc3, 0xc3,0xb6), 
E2( 0xc4, 0xc3,0x9f), 
E2( 0xc5, 0xc2,0xa5), 
E2( 0xc6, 0xc2,0xa4), 
E1( 0xc7, 0x7c), 
E2( 0xc8, 0xc3,0x85), 
E2( 0xc9, 0xc3,0xa5), 
E2( 0xca, 0xc3,0x98), 
E2( 0xcb, 0xc3,0xb8), 
E3( 0xcc, 0xe2,0x8c,0x9c), 
E3( 0xcd, 0xe2,0x8c,0x9d), 
E3( 0xce, 0xe2,0x8c,0x9e), 
E3( 0xcf, 0xe2,0x8c,0x9f), 
E1(0,0)
};
#undef E3
#undef E2
#undef E1
for( size_t i = 0; i < ARRAY_SIZE(c2utf8) ; i++ )
if( c2utf8[i].c == c ) {
strcpy( psz_utf8, c2utf8[i].utf8 );
return;
}
psz_utf8[0] = c < 0x80 ? c : '?'; 
psz_utf8[1] = '\0';
}
static void Eia608Strlcat( char *d, const char *s, int i_max )
{
if( i_max > 1 )
strncat( d, s, i_max-1 - strnlen(d, i_max-1));
if( i_max > 0 )
d[i_max-1] = '\0';
}
#define CAT(t) Eia608Strlcat( psz_text, t, i_text_max )
static text_segment_t * Eia608TextLine( struct eia608_screen *screen, int i_row )
{
const uint8_t *p_char = screen->characters[i_row];
const eia608_color_t *p_color = screen->colors[i_row];
const eia608_font_t *p_font = screen->fonts[i_row];
int i_start;
int i_end;
int x;
eia608_color_t prev_color = EIA608_COLOR_DEFAULT;
eia608_font_t prev_font = EIA608_FONT_REGULAR;
char utf8[4];
const unsigned i_text_max = 4 * EIA608_SCREEN_COLUMNS + 1;
char psz_text[i_text_max + 1];
psz_text[0] = '\0';
i_start = 0;
while( i_start < EIA608_SCREEN_COLUMNS && p_char[i_start] == ' ' ) {
Eia608TextUtf8( utf8, 0x89 );
CAT( utf8 );
i_start++;
}
i_end = EIA608_SCREEN_COLUMNS-1;
while( i_end > i_start && p_char[i_end] == ' ' )
i_end--;
if( i_start > i_end ) 
return NULL;
text_segment_t *p_segment, *p_segments_head = p_segment = text_segment_New( NULL );
if(!p_segment)
return NULL;
p_segment->style = text_style_Create( STYLE_NO_DEFAULTS );
if(!p_segment->style)
{
text_segment_Delete(p_segment);
return NULL;
}
p_segment->style->i_style_flags |= STYLE_MONOSPACED;
for( x = i_start; x <= i_end; x++ )
{
eia608_color_t color = p_color[x];
eia608_font_t font = p_font[x];
if(font != prev_font || color != prev_color)
{
EnsureUTF8(psz_text);
p_segment->psz_text = strdup(psz_text);
psz_text[0] = '\0';
p_segment->p_next = text_segment_New( NULL );
p_segment = p_segment->p_next;
if(!p_segment)
return p_segments_head;
p_segment->style = text_style_Create( STYLE_NO_DEFAULTS );
if(!p_segment->style)
{
text_segment_Delete(p_segment);
return p_segments_head;
}
p_segment->style->i_style_flags |= STYLE_MONOSPACED;
if(font & EIA608_FONT_ITALICS)
{
p_segment->style->i_style_flags |= STYLE_ITALIC;
p_segment->style->i_features |= STYLE_HAS_FLAGS;
}
if(font & EIA608_FONT_UNDERLINE)
{
p_segment->style->i_style_flags |= STYLE_UNDERLINE;
p_segment->style->i_features |= STYLE_HAS_FLAGS;
}
if(color != EIA608_COLOR_DEFAULT)
{
p_segment->style->i_font_color = rgi_eia608_colors[color];
p_segment->style->i_features |= STYLE_HAS_FONT_COLOR;
}
}
Eia608TextUtf8( utf8, p_char[x] );
CAT( utf8 );
prev_font = font;
prev_color = color;
}
#undef CAT
if( p_segment )
{
assert(!p_segment->psz_text); 
EnsureUTF8(psz_text);
p_segment->psz_text = strdup(psz_text);
}
return p_segments_head;
}
static void Eia608FillUpdaterRegions( subtext_updater_sys_t *p_updater, eia608_t *h )
{
struct eia608_screen *screen = &h->screen[h->i_screen];
substext_updater_region_t *p_region = &p_updater->region;
text_segment_t **pp_last = &p_region->p_segments;
bool b_newregion = false;
for( int i = 0; i < EIA608_SCREEN_ROWS; i++ )
{
if( !screen->row_used[i] )
continue;
text_segment_t *p_segments = Eia608TextLine( screen, i );
if( p_segments )
{
if( b_newregion )
{
substext_updater_region_t *p_newregion;
p_newregion = SubpictureUpdaterSysRegionNew();
if( !p_newregion )
{
text_segment_ChainDelete( p_segments );
return;
}
p_newregion->align = p_region->align;
p_newregion->inner_align = p_region->inner_align;
p_newregion->flags = p_region->flags;
SubpictureUpdaterSysRegionAdd( p_region, p_newregion );
p_region = p_newregion;
pp_last = &p_region->p_segments;
b_newregion = false;
}
if( p_region->p_segments == NULL ) 
{
p_region->origin.y = (float) i 
/ (EIA608_SCREEN_ROWS * FONT_TO_LINE_HEIGHT_RATIO);
p_region->flags |= UPDT_REGION_ORIGIN_Y_IS_RATIO;
}
else 
{
*pp_last = text_segment_New( "\n" );
if( *pp_last )
pp_last = &((*pp_last)->p_next);
}
*pp_last = p_segments;
do { pp_last = &((*pp_last)->p_next); } while ( *pp_last != NULL );
}
else
{
b_newregion = !!p_region->p_segments;
}
}
}
static void Eia608Init( eia608_t *h )
{
memset( h, 0, sizeof(*h) );
h->i_channel = -1;
h->i_screen = 0;
Eia608ClearScreen( h, 0 );
Eia608ClearScreen( h, 1 );
h->cursor.i_column = 0;
h->cursor.i_row = 0;
h->last.d1 = 0x00;
h->last.d2 = 0x00;
h->mode = EIA608_MODE_POPUP;
h->color = EIA608_COLOR_DEFAULT;
h->font = EIA608_FONT_REGULAR;
h->i_row_rollup = EIA608_SCREEN_ROWS-1;
}
static eia608_status_t Eia608Parse( eia608_t *h, int i_channel_selected, const uint8_t data[2] )
{
const uint8_t d1 = data[0] & 0x7f; 
const uint8_t d2 = data[1] & 0x7f;
eia608_status_t i_screen_status = EIA608_STATUS_DEFAULT;
if( d1 == 0 && d2 == 0 )
return EIA608_STATUS_DEFAULT; 
Eia608ParseChannel( h, data );
if( h->i_channel != i_channel_selected )
return false;
if( d1 >= 0x10 )
{
if( d1 >= 0x20 ||
d1 != h->last.d1 || d2 != h->last.d2 ) 
i_screen_status = Eia608ParseData( h, d1,d2 );
h->last.d1 = d1;
h->last.d2 = d2;
}
else if( ( d1 >= 0x01 && d1 <= 0x0E ) || d1 == 0x0F )
{
}
return i_screen_status;
}
