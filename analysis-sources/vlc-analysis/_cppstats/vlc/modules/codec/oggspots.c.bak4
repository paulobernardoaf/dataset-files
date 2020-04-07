
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include <vlc_image.h>

#include <assert.h>
#include <limits.h>




typedef struct
{

bool b_packetizer;




bool b_has_headers;




image_handler_t* p_image;




vlc_tick_t i_pts;
} decoder_sys_t;




static int OpenDecoder (vlc_object_t*);
static int OpenPacketizer(vlc_object_t*);
static void CloseDecoder (vlc_object_t*);

static int DecodeVideo (decoder_t*, block_t*);
static block_t* Packetize (decoder_t*, block_t**);
static int ProcessHeader(decoder_t*);
static void* ProcessPacket(decoder_t*, block_t*);
static void Flush (decoder_t*);
static picture_t* DecodePacket (decoder_t*, block_t*);






vlc_module_begin ()
set_category(CAT_INPUT)
set_subcategory(SUBCAT_INPUT_VCODEC)
set_shortname("OggSpots")
set_description(N_("OggSpots video decoder"))
set_capability("video decoder", 10)
set_callbacks(OpenDecoder, CloseDecoder)
add_shortcut("oggspots")

add_submodule ()
set_description(N_("OggSpots video packetizer"))
set_capability("packetizer", 10)
set_callbacks(OpenPacketizer, CloseDecoder)
add_shortcut("oggspots")
vlc_module_end ()

static int OpenCommon(vlc_object_t* p_this, bool b_packetizer)
{
decoder_t* p_dec = (decoder_t*)p_this;
decoder_sys_t* p_sys;

if (p_dec->fmt_in.i_codec != VLC_CODEC_OGGSPOTS) {
return VLC_EGENERIC;
}


p_sys = malloc(sizeof(*p_sys));
if (p_sys == NULL) {
return VLC_ENOMEM;
}
p_dec->p_sys = p_sys;
p_sys->b_packetizer = b_packetizer;
p_sys->b_has_headers = false;
p_sys->i_pts = VLC_TICK_INVALID;


p_sys->p_image = image_HandlerCreate(p_dec);
if (p_sys->p_image == NULL) {
free(p_sys);
return VLC_ENOMEM;
}

if( b_packetizer )
{
p_dec->fmt_out.i_codec = VLC_CODEC_OGGSPOTS;
p_dec->pf_packetize = Packetize;
}
else
{
p_dec->fmt_out.i_codec = VLC_CODEC_RGBA;
p_dec->pf_decode = DecodeVideo;
}

p_dec->pf_flush = Flush;

return VLC_SUCCESS;
}




static int OpenDecoder(vlc_object_t* p_this)
{
return OpenCommon(p_this, false);
}

static int OpenPacketizer(vlc_object_t* p_this)
{
return OpenCommon(p_this, true);
}






static void* DecodeBlock(decoder_t* p_dec, block_t* p_block)
{
decoder_sys_t* p_sys = p_dec->p_sys;


if (!p_sys->b_has_headers) {
if (ProcessHeader(p_dec)) {
block_Release(p_block);
return NULL;
}
p_sys->b_has_headers = true;
}

return ProcessPacket(p_dec, p_block);
}

static int DecodeVideo( decoder_t *p_dec, block_t *p_block )
{
if( p_block == NULL ) 
return VLCDEC_SUCCESS;

picture_t *p_pic = DecodeBlock( p_dec, p_block );
if( p_pic != NULL )
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
}

static block_t *Packetize( decoder_t *p_dec, block_t **pp_block )
{
if( pp_block == NULL ) 
return NULL;
block_t *p_block = *pp_block; *pp_block = NULL;
if( p_block == NULL )
return NULL;
return DecodeBlock( p_dec, p_block );
}




static int ProcessHeader(decoder_t* p_dec)
{
decoder_sys_t* p_sys = p_dec->p_sys;
const uint8_t* p_extra;
int i_major;
int i_minor;
uint64_t i_granulerate_numerator;
uint64_t i_granulerate_denominator;


if (p_dec->fmt_in.i_extra != 52) {
return VLC_EGENERIC;
}
p_extra = p_dec->fmt_in.p_extra;


if ( memcmp(p_extra, "SPOTS\0\0", 8) ) {
return VLC_EGENERIC;
}


i_major = GetWLE(&p_extra[ 8]); 
i_minor = GetWLE(&p_extra[10]); 
if (i_major != 0 || i_minor != 1) {
return VLC_EGENERIC;
}


i_granulerate_numerator = GetQWLE(&p_extra[12]);
i_granulerate_denominator = GetQWLE(&p_extra[20]);
if (i_granulerate_numerator == 0 || i_granulerate_denominator == 0) {
return VLC_EGENERIC;
}




if (i_granulerate_numerator == 1 && i_granulerate_denominator == 30) {
i_granulerate_numerator = 30;
i_granulerate_denominator = 1;
}


vlc_ureduce(&p_dec->fmt_in.video.i_frame_rate,
&p_dec->fmt_in.video.i_frame_rate_base,
i_granulerate_numerator, i_granulerate_denominator, 0);


if (!p_sys->b_packetizer) {
if ( memcmp(&p_extra[32], "PNG", 3) && memcmp(&p_extra[32], "JPEG", 4) ) {
char psz_image_type[8+1];
strncpy(psz_image_type, (char*)&p_extra[32], 8);
psz_image_type[sizeof(psz_image_type)-1] = '\0';

msg_Warn(p_dec, "Unsupported image format: %s", psz_image_type);
}
}


p_dec->fmt_out.video.i_width = p_dec->fmt_out.video.i_visible_width =
GetWLE(&p_extra[40]);
p_dec->fmt_out.video.i_height = p_dec->fmt_out.video.i_visible_height =
GetWLE(&p_extra[42]);


p_dec->fmt_out.video.i_sar_num = 1;
p_dec->fmt_out.video.i_sar_den = 1;





if (p_sys->b_packetizer) {
void* p_new_extra = realloc(p_dec->fmt_out.p_extra,
p_dec->fmt_in.i_extra);
if (unlikely(p_new_extra == NULL)) {
return VLC_ENOMEM;
}
p_dec->fmt_out.p_extra = p_new_extra;
p_dec->fmt_out.i_extra = p_dec->fmt_in.i_extra;
memcpy(p_dec->fmt_out.p_extra,
p_dec->fmt_in.p_extra, p_dec->fmt_out.i_extra);
}

return VLC_SUCCESS;
}




static void Flush(decoder_t* p_dec)
{
decoder_sys_t* p_sys = p_dec->p_sys;

p_sys->i_pts = VLC_TICK_INVALID;
}




static void* ProcessPacket(decoder_t* p_dec, block_t* p_block)
{
decoder_sys_t* p_sys = p_dec->p_sys;
void* p_buf;

if ( (p_block->i_flags & BLOCK_FLAG_DISCONTINUITY) != 0 ) {
p_sys->i_pts = p_block->i_pts;
}

if ( (p_block->i_flags & BLOCK_FLAG_CORRUPTED) != 0 ) {
block_Release(p_block);
return NULL;
}


if (p_block->i_pts != VLC_TICK_INVALID && p_block->i_pts != p_sys->i_pts) {
p_sys->i_pts = p_block->i_pts;
}

if (p_sys->b_packetizer) {



p_block->i_dts = p_block->i_pts = p_sys->i_pts;

p_block->i_length = p_sys->i_pts - p_block->i_pts;

p_buf = p_block;
}
else {
p_buf = DecodePacket(p_dec, p_block);
}

return p_buf;
}




static picture_t* DecodePacket(decoder_t* p_dec, block_t* p_block)
{
decoder_sys_t* p_sys = p_dec->p_sys;
uint32_t i_img_offset;
picture_t* p_pic;

if (p_block->i_buffer < 20) {
msg_Dbg(p_dec, "Packet too short");
goto error;
}


i_img_offset = GetDWLE(p_block->p_buffer);
if (i_img_offset < 20) {
msg_Dbg(p_dec, "Invalid byte offset");
goto error;
}


if ( !memcmp(&p_block->p_buffer[4], "PNG", 3) ) {
p_dec->fmt_in.video.i_chroma = VLC_CODEC_PNG;
}
else if ( !memcmp(&p_block->p_buffer[4], "JPEG", 4) ) {
p_dec->fmt_in.video.i_chroma = VLC_CODEC_JPEG;
}
else {
char psz_image_type[8+1];
strncpy(psz_image_type, (char*)&p_block->p_buffer[4], 8);
psz_image_type[sizeof(psz_image_type)-1] = '\0';

msg_Dbg(p_dec, "Unsupported image format: %s", psz_image_type);
goto error;
}




p_block->i_buffer -= i_img_offset;
p_block->p_buffer += i_img_offset;

p_pic = image_Read(p_sys->p_image, p_block,
&p_dec->fmt_in,
&p_dec->fmt_out.video);
if (p_pic == NULL) {
return NULL;
}

p_pic->b_force = true;
p_dec->fmt_out.i_codec = p_dec->fmt_out.video.i_chroma;
decoder_UpdateVideoFormat(p_dec);

return p_pic;

error:
block_Release(p_block);
return NULL;
}




static void CloseDecoder(vlc_object_t* p_this)
{
decoder_t* p_dec = (decoder_t*)p_this;
decoder_sys_t* p_sys = p_dec->p_sys;

image_HandlerDelete(p_sys->p_image);
free(p_sys);
}
