#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_charset.h>
#include <jpeglib.h>
#include <setjmp.h>
#define JPEG_SYS_COMMON_MEMBERS struct jpeg_error_mgr err; jmp_buf setjmp_buffer; vlc_object_t *p_obj; 
#define ENC_CFG_PREFIX "sout-jpeg-"
#define ENC_QUALITY_TEXT N_("Quality level")
#define ENC_QUALITY_LONGTEXT N_("Quality level " "for encoding (this can enlarge or reduce output image size).")
struct jpeg_sys_t
{
JPEG_SYS_COMMON_MEMBERS
};
typedef struct jpeg_sys_t jpeg_sys_t;
typedef struct
{
JPEG_SYS_COMMON_MEMBERS
JSAMPARRAY p_row_pointers;
struct jpeg_decompress_struct p_jpeg;
} decoder_sys_t;
static int OpenDecoder(vlc_object_t *);
static void CloseDecoder(vlc_object_t *);
static int DecodeBlock(decoder_t *, block_t *);
typedef struct
{
JPEG_SYS_COMMON_MEMBERS
struct jpeg_compress_struct p_jpeg;
int i_blocksize;
int i_quality;
} encoder_sys_t;
static const char * const ppsz_enc_options[] = {
"quality",
NULL
};
static int OpenEncoder(vlc_object_t *);
static void CloseEncoder(vlc_object_t *);
static block_t *EncodeBlock(encoder_t *, picture_t *);
vlc_module_begin()
set_category(CAT_INPUT)
set_subcategory(SUBCAT_INPUT_VCODEC)
set_description(N_("JPEG image decoder"))
set_capability("video decoder", 1000)
set_callbacks(OpenDecoder, CloseDecoder)
add_shortcut("jpeg")
add_submodule()
add_shortcut("jpeg")
set_section(N_("Encoding"), NULL)
set_description(N_("JPEG image encoder"))
set_capability("encoder", 1000)
set_callbacks(OpenEncoder, CloseEncoder)
add_integer_with_range(ENC_CFG_PREFIX "quality", 95, 0, 100,
ENC_QUALITY_TEXT, ENC_QUALITY_LONGTEXT, true)
vlc_module_end()
static void user_error_exit(j_common_ptr p_jpeg)
{
jpeg_sys_t *p_sys = (jpeg_sys_t *)p_jpeg->err;
p_sys->err.output_message(p_jpeg);
longjmp(p_sys->setjmp_buffer, 1);
}
static void user_error_message(j_common_ptr p_jpeg)
{
char error_msg[JMSG_LENGTH_MAX];
jpeg_sys_t *p_sys = (jpeg_sys_t *)p_jpeg->err;
p_sys->err.format_message(p_jpeg, error_msg);
msg_Err(p_sys->p_obj, "%s", error_msg);
}
static int OpenDecoder(vlc_object_t *p_this)
{
decoder_t *p_dec = (decoder_t *)p_this;
if (p_dec->fmt_in.i_codec != VLC_CODEC_JPEG)
{
return VLC_EGENERIC;
}
decoder_sys_t *p_sys = malloc(sizeof(decoder_sys_t));
if (p_sys == NULL)
{
return VLC_ENOMEM;
}
p_dec->p_sys = p_sys;
p_sys->p_obj = p_this;
p_sys->p_jpeg.err = jpeg_std_error(&p_sys->err);
p_sys->err.error_exit = user_error_exit;
p_sys->err.output_message = user_error_message;
p_dec->pf_decode = DecodeBlock;
p_dec->fmt_out.video.i_chroma =
p_dec->fmt_out.i_codec = VLC_CODEC_RGB24;
p_dec->fmt_out.video.transfer = TRANSFER_FUNC_SRGB;
p_dec->fmt_out.video.space = COLOR_SPACE_SRGB;
p_dec->fmt_out.video.primaries = COLOR_PRIMARIES_SRGB;
p_dec->fmt_out.video.color_range = COLOR_RANGE_FULL;
video_format_FixRgb(&p_dec->fmt_out.video);
return VLC_SUCCESS;
}
#define G_LITTLE_ENDIAN 1234
#define G_BIG_ENDIAN 4321
typedef unsigned int uint;
typedef unsigned short ushort;
LOCAL( unsigned short )
de_get16( void * ptr, uint endian ) {
unsigned short val;
memcpy( &val, ptr, sizeof( val ) );
if ( endian == G_BIG_ENDIAN )
{
#if !defined(WORDS_BIGENDIAN)
val = vlc_bswap16( val );
#endif
}
else
{
#if defined(WORDS_BIGENDIAN)
val = vlc_bswap16( val );
#endif
}
return val;
}
LOCAL( unsigned int )
de_get32( void * ptr, uint endian ) {
unsigned int val;
memcpy( &val, ptr, sizeof( val ) );
if ( endian == G_BIG_ENDIAN )
{
#if !defined(WORDS_BIGENDIAN)
val = vlc_bswap32( val );
#endif
}
else
{
#if defined(WORDS_BIGENDIAN)
val = vlc_bswap32( val );
#endif
}
return val;
}
static bool getRDFFloat(const char *psz_rdf, float *out, const char *psz_var)
{
char *p_start = strcasestr(psz_rdf, psz_var);
if (p_start == NULL)
return false;
size_t varlen = strlen(psz_var);
p_start += varlen;
char *p_end = NULL;
if (p_start[0] == '>')
{
p_start += 1;
p_end = strchr(p_start, '<');
}
else if (p_start[0] == '=' && p_start[1] == '"')
{
p_start += 2;
p_end = strchr(p_start, '"');
}
if (unlikely(p_end == NULL || p_end == p_start + 1))
return false;
*out = us_strtof(p_start, NULL);
return true;
}
#define EXIF_JPEG_MARKER 0xE1
#define EXIF_XMP_STRING "http://ns.adobe.com/xap/1.0/\000"
static void jpeg_GetProjection(j_decompress_ptr cinfo, video_format_t *fmt)
{
jpeg_saved_marker_ptr xmp_marker = NULL;
jpeg_saved_marker_ptr cmarker = cinfo->marker_list;
while (cmarker)
{
if (cmarker->marker == EXIF_JPEG_MARKER)
{
if(cmarker->data_length >= 32 &&
!memcmp(cmarker->data, EXIF_XMP_STRING, 29))
{
xmp_marker = cmarker;
break;
}
}
cmarker = cmarker->next;
}
if (xmp_marker == NULL)
return;
char *psz_rdf = malloc(xmp_marker->data_length - 29 + 1);
if (unlikely(psz_rdf == NULL))
return;
memcpy(psz_rdf, xmp_marker->data + 29, xmp_marker->data_length - 29);
psz_rdf[xmp_marker->data_length - 29] = '\0';
if (strcasestr(psz_rdf, "ProjectionType=\"equirectangular\"") ||
strcasestr(psz_rdf, "ProjectionType>equirectangular"))
fmt->projection_mode = PROJECTION_MODE_EQUIRECTANGULAR;
float value;
if (getRDFFloat(psz_rdf, &value, "PoseHeadingDegrees"))
fmt->pose.yaw = value;
if (getRDFFloat(psz_rdf, &value, "PosePitchDegrees"))
fmt->pose.pitch = value;
if (getRDFFloat(psz_rdf, &value, "PoseRollDegrees"))
fmt->pose.roll = value;
if (getRDFFloat(psz_rdf, &value, "InitialViewHeadingDegrees"))
fmt->pose.yaw = value;
if (getRDFFloat(psz_rdf, &value, "InitialViewPitchDegrees"))
fmt->pose.pitch = value;
if (getRDFFloat(psz_rdf, &value, "InitialViewRollDegrees"))
fmt->pose.roll = value;
if (getRDFFloat(psz_rdf, &value, "InitialHorizontalFOVDegrees"))
fmt->pose.fov = value;
free(psz_rdf);
}
LOCAL( int )
jpeg_GetOrientation( j_decompress_ptr cinfo )
{
uint i; 
ushort tag_type; 
uint ret; 
uint offset; 
uint tags; 
uint type; 
uint count; 
uint tiff = 0; 
uint endian = 0; 
jpeg_saved_marker_ptr exif_marker; 
jpeg_saved_marker_ptr cmarker; 
const char leth[] = { 0x49, 0x49, 0x2a, 0x00 }; 
const char beth[] = { 0x4d, 0x4d, 0x00, 0x2a }; 
#define EXIF_IDENT_STRING "Exif\000\000"
#define EXIF_ORIENT_TAGID 0x112
exif_marker = NULL;
cmarker = cinfo->marker_list;
while ( cmarker )
{
if ( cmarker->data_length >= 32 &&
cmarker->marker == EXIF_JPEG_MARKER )
{
if ( !memcmp( cmarker->data, EXIF_IDENT_STRING, 6 ) )
{
exif_marker = cmarker;
}
}
cmarker = cmarker->next;
}
if ( exif_marker == NULL )
return 0;
i = 0;
while ( i < 16 )
{
if ( memcmp( &exif_marker->data[i], leth, 4 ) == 0 )
{
endian = G_LITTLE_ENDIAN;
}
else
if ( memcmp( &exif_marker->data[i], beth, 4 ) == 0 )
{
endian = G_BIG_ENDIAN;
}
else
{
i++;
continue;
}
tiff = i;
break;
}
if ( tiff == 0 )
return 0;
offset = de_get32( &exif_marker->data[i] + 4, endian );
i = i + offset;
if ( i > exif_marker->data_length - 2 )
return 0;
tags = de_get16( &exif_marker->data[i], endian );
i = i + 2;
if ( tags * 12U > exif_marker->data_length - i )
return 0;
while ( tags-- )
{
tag_type = de_get16( &exif_marker->data[i], endian );
if ( tag_type == EXIF_ORIENT_TAGID )
{
type = de_get16( &exif_marker->data[i + 2], endian );
count = de_get32( &exif_marker->data[i + 4], endian );
if ( type != 3 || count != 1 )
return 0;
ret = de_get16( &exif_marker->data[i + 8], endian );
return ( ret <= 8 ) ? ret : 0;
}
i = i + 12;
}
return 0; 
}
static int DecodeBlock(decoder_t *p_dec, block_t *p_block)
{
decoder_sys_t *p_sys = p_dec->p_sys;
picture_t *p_pic = 0;
p_sys->p_row_pointers = NULL;
if (!p_block) 
return VLCDEC_SUCCESS;
if (p_block->i_flags & BLOCK_FLAG_CORRUPTED )
{
block_Release(p_block);
return VLCDEC_SUCCESS;
}
if (setjmp(p_sys->setjmp_buffer))
{
goto error;
}
jpeg_create_decompress(&p_sys->p_jpeg);
jpeg_mem_src(&p_sys->p_jpeg, p_block->p_buffer, p_block->i_buffer);
jpeg_save_markers( &p_sys->p_jpeg, EXIF_JPEG_MARKER, 0xffff );
jpeg_read_header(&p_sys->p_jpeg, TRUE);
p_sys->p_jpeg.out_color_space = JCS_RGB;
jpeg_start_decompress(&p_sys->p_jpeg);
p_dec->fmt_out.video.i_visible_width = p_dec->fmt_out.video.i_width = p_sys->p_jpeg.output_width;
p_dec->fmt_out.video.i_visible_height = p_dec->fmt_out.video.i_height = p_sys->p_jpeg.output_height;
p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;
int i_otag; 
i_otag = jpeg_GetOrientation( &p_sys->p_jpeg );
if ( i_otag > 1 )
{
msg_Dbg( p_dec, "Jpeg orientation is %d", i_otag );
p_dec->fmt_out.video.orientation = ORIENT_FROM_EXIF( i_otag );
}
jpeg_GetProjection(&p_sys->p_jpeg, &p_dec->fmt_out.video);
if (decoder_UpdateVideoFormat(p_dec))
{
goto error;
}
p_pic = decoder_NewPicture(p_dec);
if (!p_pic)
{
goto error;
}
p_sys->p_row_pointers = vlc_alloc(p_sys->p_jpeg.output_height, sizeof(JSAMPROW));
if (!p_sys->p_row_pointers)
{
goto error;
}
for (unsigned i = 0; i < p_sys->p_jpeg.output_height; i++) {
p_sys->p_row_pointers[i] = p_pic->p->p_pixels + p_pic->p->i_pitch * i;
}
while (p_sys->p_jpeg.output_scanline < p_sys->p_jpeg.output_height)
{
jpeg_read_scanlines(&p_sys->p_jpeg,
p_sys->p_row_pointers + p_sys->p_jpeg.output_scanline,
p_sys->p_jpeg.output_height - p_sys->p_jpeg.output_scanline);
}
jpeg_finish_decompress(&p_sys->p_jpeg);
jpeg_destroy_decompress(&p_sys->p_jpeg);
free(p_sys->p_row_pointers);
p_pic->date = p_block->i_pts != VLC_TICK_INVALID ? p_block->i_pts : p_block->i_dts;
block_Release(p_block);
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
error:
jpeg_destroy_decompress(&p_sys->p_jpeg);
free(p_sys->p_row_pointers);
block_Release(p_block);
return VLCDEC_SUCCESS;
}
static void CloseDecoder(vlc_object_t *p_this)
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
free(p_sys);
}
static int OpenEncoder(vlc_object_t *p_this)
{
encoder_t *p_enc = (encoder_t *)p_this;
config_ChainParse(p_enc, ENC_CFG_PREFIX, ppsz_enc_options, p_enc->p_cfg);
if (p_enc->fmt_out.i_codec != VLC_CODEC_JPEG)
{
return VLC_EGENERIC;
}
encoder_sys_t *p_sys = malloc(sizeof(encoder_sys_t));
if (p_sys == NULL)
{
return VLC_ENOMEM;
}
p_enc->p_sys = p_sys;
p_sys->p_obj = p_this;
p_sys->p_jpeg.err = jpeg_std_error(&p_sys->err);
p_sys->err.error_exit = user_error_exit;
p_sys->err.output_message = user_error_message;
p_sys->i_quality = var_GetInteger(p_enc, ENC_CFG_PREFIX "quality");
p_sys->i_blocksize = 3 * p_enc->fmt_in.video.i_visible_width * p_enc->fmt_in.video.i_visible_height;
p_enc->fmt_in.i_codec = VLC_CODEC_J420;
p_enc->pf_encode_video = EncodeBlock;
return VLC_SUCCESS;
}
static block_t *EncodeBlock(encoder_t *p_enc, picture_t *p_pic)
{
encoder_sys_t *p_sys = p_enc->p_sys;
if (unlikely(!p_pic))
{
return NULL;
}
block_t *p_block = block_Alloc(p_sys->i_blocksize);
if (p_block == NULL)
{
return NULL;
}
JSAMPIMAGE p_row_pointers = NULL;
unsigned long size = p_block->i_buffer;
if (setjmp(p_sys->setjmp_buffer))
{
goto error;
}
jpeg_create_compress(&p_sys->p_jpeg);
jpeg_mem_dest(&p_sys->p_jpeg, &p_block->p_buffer, &size);
p_sys->p_jpeg.image_width = p_enc->fmt_in.video.i_visible_width;
p_sys->p_jpeg.image_height = p_enc->fmt_in.video.i_visible_height;
p_sys->p_jpeg.input_components = 3;
p_sys->p_jpeg.in_color_space = JCS_YCbCr;
jpeg_set_defaults(&p_sys->p_jpeg);
jpeg_set_colorspace(&p_sys->p_jpeg, JCS_YCbCr);
p_sys->p_jpeg.raw_data_in = TRUE;
#if JPEG_LIB_VERSION >= 70
p_sys->p_jpeg.do_fancy_downsampling = FALSE;
#endif
jpeg_set_quality(&p_sys->p_jpeg, p_sys->i_quality, TRUE);
jpeg_start_compress(&p_sys->p_jpeg, TRUE);
p_row_pointers = vlc_alloc(p_pic->i_planes, sizeof(JSAMPARRAY));
if (p_row_pointers == NULL)
{
goto error;
}
for (int i = 0; i < p_pic->i_planes; i++)
{
p_row_pointers[i] = vlc_alloc(p_sys->p_jpeg.comp_info[i].v_samp_factor, sizeof(JSAMPROW) * DCTSIZE);
}
while (p_sys->p_jpeg.next_scanline < p_sys->p_jpeg.image_height)
{
for (int i = 0; i < p_pic->i_planes; i++)
{
int i_offset = p_sys->p_jpeg.next_scanline * p_sys->p_jpeg.comp_info[i].v_samp_factor / p_sys->p_jpeg.max_v_samp_factor;
for (int j = 0; j < p_sys->p_jpeg.comp_info[i].v_samp_factor * DCTSIZE; j++)
{
p_row_pointers[i][j] = p_pic->p[i].p_pixels + p_pic->p[i].i_pitch * (i_offset + j);
}
}
jpeg_write_raw_data(&p_sys->p_jpeg, p_row_pointers, p_sys->p_jpeg.max_v_samp_factor * DCTSIZE);
}
jpeg_finish_compress(&p_sys->p_jpeg);
jpeg_destroy_compress(&p_sys->p_jpeg);
for (int i = 0; i < p_pic->i_planes; i++)
{
free(p_row_pointers[i]);
}
free(p_row_pointers);
p_block->i_buffer = size;
p_block->i_dts = p_block->i_pts = p_pic->date;
return p_block;
error:
jpeg_destroy_compress(&p_sys->p_jpeg);
if (p_row_pointers != NULL)
{
for (int i = 0; i < p_pic->i_planes; i++)
{
free(p_row_pointers[i]);
}
}
free(p_row_pointers);
block_Release(p_block);
return NULL;
}
static void CloseEncoder(vlc_object_t *p_this)
{
encoder_t *p_enc = (encoder_t *)p_this;
encoder_sys_t *p_sys = p_enc->p_sys;
free(p_sys);
}
