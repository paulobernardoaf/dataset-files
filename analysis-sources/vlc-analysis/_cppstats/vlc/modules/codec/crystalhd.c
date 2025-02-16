#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_codec.h>
#include "../packetizer/h264_nal.h"
#if !defined(_WIN32) && !defined(__APPLE__)
#define __LINUX_USER__
#endif
#include <libcrystalhd/bc_dts_defs.h>
#include <libcrystalhd/bc_dts_types.h>
#if defined(HAVE_LIBCRYSTALHD_BC_DRV_IF_H) 
#include <libcrystalhd/bc_drv_if.h>
#elif defined(_WIN32)
#define USE_DL_OPENING 1
#else
#include <libcrystalhd/libcrystalhd_if.h>
#endif
#if defined(USE_DL_OPENING)
#warning DLL opening mode
#define BC_FUNC( a ) Our ##a
#define BC_FUNC_PSYS( a ) p_sys->Our ##a
#else
#define BC_FUNC( a ) a
#define BC_FUNC_PSYS( a ) a
#endif
#include <assert.h>
#define TO_BC_PTS( a ) ( MSFTIME_FROM_VLC_TICK(a) + 1 )
#define FROM_BC_PTS( a ) VLC_TICK_FROM_MSFTIME(a - 1)
static int OpenDecoder ( vlc_object_t * );
static void CloseDecoder ( vlc_object_t * );
vlc_module_begin ()
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_VCODEC )
set_description( N_("Crystal HD hardware video decoder") )
set_capability( "video decoder", 0 )
set_callbacks( OpenDecoder, CloseDecoder )
add_shortcut( "crystalhd" )
vlc_module_end ()
static int DecodeBlock ( decoder_t *p_dec, block_t *p_block );
static int crystal_insert_sps_pps(decoder_t *, uint8_t *, uint32_t);
typedef struct
{
HANDLE bcm_handle; 
uint8_t *p_sps_pps_buf; 
size_t i_sps_pps_size; 
uint8_t i_nal_size; 
picture_t *p_pic;
BC_DTS_PROC_OUT *proc_out;
#if defined(USE_DL_OPENING)
HINSTANCE p_bcm_dll;
BC_STATUS (WINAPI *OurDtsCloseDecoder)( HANDLE hDevice );
BC_STATUS (WINAPI *OurDtsDeviceClose)( HANDLE hDevice );
BC_STATUS (WINAPI *OurDtsFlushInput)( HANDLE hDevice, U32 Mode );
BC_STATUS (WINAPI *OurDtsStopDecoder)( HANDLE hDevice );
BC_STATUS (WINAPI *OurDtsGetDriverStatus)( HANDLE hDevice,
BC_DTS_STATUS *pStatus );
BC_STATUS (WINAPI *OurDtsProcInput)( HANDLE hDevice, U8 *pUserData,
U32 ulSizeInBytes, U64 timeStamp, BOOL encrypted );
BC_STATUS (WINAPI *OurDtsProcOutput)( HANDLE hDevice, U32 milliSecWait,
BC_DTS_PROC_OUT *pOut );
BC_STATUS (WINAPI *OurDtsIsEndOfStream)( HANDLE hDevice, U8* bEOS );
#endif
} decoder_sys_t;
static int OpenDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t*)p_this;
decoder_sys_t *p_sys;
uint32_t i_bcm_codec_subtype = 0;
switch ( p_dec->fmt_in.i_codec )
{
case VLC_CODEC_H264:
if( p_dec->fmt_in.i_original_fourcc == VLC_FOURCC( 'a', 'v', 'c', '1' ) )
i_bcm_codec_subtype = BC_MSUBTYPE_AVC1;
else
i_bcm_codec_subtype = BC_MSUBTYPE_H264;
break;
case VLC_CODEC_VC1:
i_bcm_codec_subtype = BC_MSUBTYPE_VC1;
break;
case VLC_CODEC_WMV3:
i_bcm_codec_subtype = BC_MSUBTYPE_WMV3;
break;
case VLC_CODEC_WMVA:
i_bcm_codec_subtype = BC_MSUBTYPE_WMVA;
break;
case VLC_CODEC_MPGV:
i_bcm_codec_subtype = BC_MSUBTYPE_MPEG2VIDEO;
break;
default:
return VLC_EGENERIC;
}
p_sys = malloc( sizeof(*p_sys) );
if( !p_sys )
return VLC_ENOMEM;
p_dec->p_sys = p_sys;
p_sys->i_nal_size = 4; 
p_sys->i_sps_pps_size = 0;
p_sys->p_sps_pps_buf = NULL;
p_sys->p_pic = NULL;
p_sys->proc_out = NULL;
#if defined(USE_DL_OPENING)
#define DLL_NAME "bcmDIL.dll"
#define PATHS_NB 3
static const WCHAR *psz_paths[PATHS_NB] = {
TEXT(DLL_NAME),
TEXT("C:\\Program Files\\Broadcom\\Broadcom CrystalHD Decoder\\" DLL_NAME),
TEXT("C:\\Program Files (x86)\\Broadcom\\Broadcom CrystalHD Decoder\\" DLL_NAME),
};
for( int i = 0; i < PATHS_NB; i++ )
{
HINSTANCE p_bcm_dll = LoadLibrary( psz_paths[i] );
if( p_bcm_dll )
{
p_sys->p_bcm_dll = p_bcm_dll;
break;
}
}
if( !p_sys->p_bcm_dll )
{
msg_Dbg( p_dec, "Couldn't load the CrystalHD dll");
return VLC_EGENERIC;
}
#define LOAD_SYM( a ) BC_FUNC( a ) = (void *)GetProcAddress( p_sys->p_bcm_dll, ( #a ) ); if( !BC_FUNC( a ) ) { msg_Err( p_dec, "missing symbol " #a ); return VLC_EGENERIC; }
#define LOAD_SYM_PSYS( a ) p_sys->BC_FUNC( a ) = (void *)GetProcAddress( p_sys->p_bcm_dll, #a ); if( !p_sys->BC_FUNC( a ) ) { msg_Err( p_dec, "missing symbol " #a ); return VLC_EGENERIC; }
BC_STATUS (WINAPI *OurDtsDeviceOpen)( HANDLE *hDevice, U32 mode );
LOAD_SYM( DtsDeviceOpen );
BC_STATUS (WINAPI *OurDtsCrystalHDVersion)( HANDLE hDevice, PBC_INFO_CRYSTAL bCrystalInfo );
LOAD_SYM( DtsCrystalHDVersion );
BC_STATUS (WINAPI *OurDtsSetColorSpace)( HANDLE hDevice, BC_OUTPUT_FORMAT Mode422 );
LOAD_SYM( DtsSetColorSpace );
BC_STATUS (WINAPI *OurDtsSetInputFormat)( HANDLE hDevice, BC_INPUT_FORMAT *pInputFormat );
LOAD_SYM( DtsSetInputFormat );
BC_STATUS (WINAPI *OurDtsOpenDecoder)( HANDLE hDevice, U32 StreamType );
LOAD_SYM( DtsOpenDecoder );
BC_STATUS (WINAPI *OurDtsStartDecoder)( HANDLE hDevice );
LOAD_SYM( DtsStartDecoder );
BC_STATUS (WINAPI *OurDtsStartCapture)( HANDLE hDevice );
LOAD_SYM( DtsStartCapture );
LOAD_SYM_PSYS( DtsCloseDecoder );
LOAD_SYM_PSYS( DtsDeviceClose );
LOAD_SYM_PSYS( DtsFlushInput );
LOAD_SYM_PSYS( DtsStopDecoder );
LOAD_SYM_PSYS( DtsGetDriverStatus );
LOAD_SYM_PSYS( DtsProcInput );
LOAD_SYM_PSYS( DtsProcOutput );
LOAD_SYM_PSYS( DtsIsEndOfStream );
#undef LOAD_SYM
#undef LOAD_SYM_PSYS
#endif 
#if defined(DEBUG_CRYSTALHD)
msg_Dbg( p_dec, "Trying to open CrystalHD HW");
#endif
if( BC_FUNC(DtsDeviceOpen)( &p_sys->bcm_handle,
(DTS_PLAYBACK_MODE | DTS_LOAD_FILE_PLAY_FW | DTS_SKIP_TX_CHK_CPB) )
!= BC_STS_SUCCESS )
{
msg_Err( p_dec, "Couldn't find and open the BCM CrystalHD device" );
free( p_sys );
return VLC_EGENERIC;
}
#if defined(DEBUG_CRYSTALHD)
BC_INFO_CRYSTAL info;
if( BC_FUNC(DtsCrystalHDVersion)( p_sys->bcm_handle, &info ) == BC_STS_SUCCESS )
{
msg_Dbg( p_dec, "Using CrystalHD Driver version: %i.%i.%i, "
"Library version: %i.%i.%i, Firmware version: %i.%i.%i",
info.drvVersion.drvRelease, info.drvVersion.drvMajor,
info.drvVersion.drvMinor,
info.dilVersion.dilRelease, info.dilVersion.dilMajor,
info.dilVersion.dilMinor,
info.fwVersion.fwRelease, info.fwVersion.fwMajor,
info.fwVersion.fwMinor );
}
#endif
if( i_bcm_codec_subtype == BC_MSUBTYPE_AVC1 )
{
if( p_dec->fmt_in.i_extra > 0 )
{
msg_Dbg( p_dec, "Parsing extra infos for avc1" );
if( crystal_insert_sps_pps( p_dec, (uint8_t*)p_dec->fmt_in.p_extra,
p_dec->fmt_in.i_extra ) != VLC_SUCCESS )
goto error;
}
else
{
msg_Err( p_dec, "Missing extra infos for avc1" );
goto error;
}
}
if( BC_FUNC(DtsSetColorSpace)( p_sys->bcm_handle, OUTPUT_MODE422_YUY2 )
!= BC_STS_SUCCESS )
{
msg_Err( p_dec, "Couldn't set the color space. Please report this!" );
goto error;
}
BC_INPUT_FORMAT p_in;
memset( &p_in, 0, sizeof(BC_INPUT_FORMAT) );
p_in.OptFlags = 0x51; 
p_in.mSubtype = i_bcm_codec_subtype;
p_in.startCodeSz = p_sys->i_nal_size;
p_in.pMetaData = p_sys->p_sps_pps_buf;
p_in.metaDataSz = p_sys->i_sps_pps_size;
p_in.width = p_dec->fmt_in.video.i_width;
p_in.height = p_dec->fmt_in.video.i_height;
p_in.Progressive = true;
if( BC_FUNC(DtsSetInputFormat)( p_sys->bcm_handle, &p_in ) != BC_STS_SUCCESS )
{
msg_Err( p_dec, "Couldn't set the color space. Please report this!" );
goto error;
}
if( BC_FUNC(DtsOpenDecoder)( p_sys->bcm_handle, BC_STREAM_TYPE_ES )
!= BC_STS_SUCCESS )
{
msg_Err( p_dec, "Couldn't open the CrystalHD decoder" );
goto error;
}
if( BC_FUNC(DtsStartDecoder)( p_sys->bcm_handle ) != BC_STS_SUCCESS )
{
msg_Err( p_dec, "Couldn't start the decoder" );
goto error;
}
if( BC_FUNC(DtsStartCapture)( p_sys->bcm_handle ) != BC_STS_SUCCESS )
{
msg_Err( p_dec, "Couldn't start the capture" );
goto error_complete;
}
p_dec->fmt_out.i_codec = VLC_CODEC_YUYV;
p_dec->fmt_out.video.i_width = p_dec->fmt_in.video.i_width;
p_dec->fmt_out.video.i_height = p_dec->fmt_in.video.i_height;
p_dec->pf_decode = DecodeBlock;
msg_Info( p_dec, "Opened CrystalHD hardware with success" );
return VLC_SUCCESS;
error_complete:
BC_FUNC_PSYS(DtsCloseDecoder)( p_sys->bcm_handle );
error:
BC_FUNC_PSYS(DtsDeviceClose)( p_sys->bcm_handle );
free( p_sys );
return VLC_EGENERIC;
}
static void CloseDecoder( vlc_object_t *p_this )
{
decoder_t *p_dec = (decoder_t *)p_this;
decoder_sys_t *p_sys = p_dec->p_sys;
if( BC_FUNC_PSYS(DtsFlushInput)( p_sys->bcm_handle, 2 ) != BC_STS_SUCCESS )
goto error;
if( BC_FUNC_PSYS(DtsStopDecoder)( p_sys->bcm_handle ) != BC_STS_SUCCESS )
goto error;
if( BC_FUNC_PSYS(DtsCloseDecoder)( p_sys->bcm_handle ) != BC_STS_SUCCESS )
goto error;
if( BC_FUNC_PSYS(DtsDeviceClose)( p_sys->bcm_handle ) != BC_STS_SUCCESS )
goto error;
error:
free( p_sys->p_sps_pps_buf );
#if defined(DEBUG_CRYSTALHD)
msg_Dbg( p_dec, "done cleaning up CrystalHD" );
#endif
free( p_sys );
}
#if defined(__KERNEL__) || defined(__LINUX_USER__)
static BC_STATUS ourCallback(void *shnd, uint32_t width, uint32_t height, uint32_t stride, void *pOut)
{
BC_DTS_PROC_OUT *proc_in = (BC_DTS_PROC_OUT*)pOut;
#else
static BC_STATUS ourCallback(void *shnd, uint32_t width, uint32_t height, uint32_t stride, BC_DTS_PROC_OUT *proc_in)
{
#endif
VLC_UNUSED(width); VLC_UNUSED(height); VLC_UNUSED(stride);
decoder_t *p_dec = (decoder_t *)shnd;
decoder_sys_t *p_sys = p_dec->p_sys;
BC_DTS_PROC_OUT *proc_out = p_sys->proc_out;
if( !(proc_in->PicInfo.flags & VDEC_FLAG_INTERLACED_SRC) ||
!(proc_in->PicInfo.flags & VDEC_FLAG_FIELDPAIR) )
{
if( !decoder_UpdateVideoFormat( p_dec ) )
p_sys->p_pic = decoder_NewPicture( p_dec );
}
picture_t *p_pic = p_sys->p_pic;
if( !p_pic )
return BC_STS_ERROR;
p_pic->b_progressive = !(proc_in->PicInfo.flags & VDEC_FLAG_INTERLACED_SRC);
p_pic->b_top_field_first = !(proc_in->PicInfo.flags & VDEC_FLAG_BOTTOM_FIRST);
p_pic->i_nb_fields = p_pic->b_progressive? 1: 2;
proc_out->Ybuff = !(proc_in->PicInfo.flags & VDEC_FLAG_FIELDPAIR) ?
&p_pic->p[0].p_pixels[0] :
&p_pic->p[0].p_pixels[p_pic->p[0].i_pitch];
proc_out->YbuffSz = 2 * p_pic->p[0].i_pitch;
proc_out->StrideSz = (proc_in->PicInfo.flags & VDEC_FLAG_INTERLACED_SRC)?
2 * (p_pic->p[0].i_pitch/2) - p_dec->fmt_out.video.i_width:
p_pic->p[0].i_pitch/2 - p_dec->fmt_out.video.i_width;
proc_out->PoutFlags |= BC_POUT_FLAGS_STRIDE; 
return BC_STS_SUCCESS;
}
static int DecodeBlock( decoder_t *p_dec, block_t *p_block )
{
decoder_sys_t *p_sys = p_dec->p_sys;
BC_DTS_PROC_OUT proc_out;
BC_DTS_STATUS driver_stat;
if( BC_FUNC_PSYS(DtsGetDriverStatus)( p_sys->bcm_handle, &driver_stat ) != BC_STS_SUCCESS )
{
block_Release( p_block );
return VLCDEC_SUCCESS;
}
if( p_block )
{
if( ( p_block->i_flags & (BLOCK_FLAG_CORRUPTED) ) == 0 )
{
BC_STATUS status = BC_FUNC_PSYS(DtsProcInput)( p_sys->bcm_handle,
p_block->p_buffer,
p_block->i_buffer,
p_block->i_pts != VLC_TICK_INVALID ? TO_BC_PTS(p_block->i_pts) : 0, false );
block_Release( p_block );
if( status != BC_STS_SUCCESS )
return VLCDEC_SUCCESS;
}
}
#if defined(DEBUG_CRYSTALHD)
else
{
if( driver_stat.ReadyListCount != 0 )
msg_Err( p_dec, " Input NULL but have pictures %u", driver_stat.ReadyListCount );
}
#endif
if( driver_stat.ReadyListCount == 0 )
return VLCDEC_SUCCESS;
memset( &proc_out, 0, sizeof(BC_DTS_PROC_OUT) );
proc_out.PicInfo.width = p_dec->fmt_out.video.i_width;
proc_out.PicInfo.height = p_dec->fmt_out.video.i_height;
proc_out.PoutFlags = BC_POUT_FLAGS_SIZE;
proc_out.AppCallBack = ourCallback;
proc_out.hnd = p_dec;
p_sys->proc_out = &proc_out;
BC_STATUS sts = BC_FUNC_PSYS(DtsProcOutput)( p_sys->bcm_handle, 128, &proc_out );
#if defined(DEBUG_CRYSTALHD)
if( sts != BC_STS_SUCCESS )
msg_Err( p_dec, "DtsProcOutput returned %i", sts );
#endif
uint8_t b_eos;
picture_t *p_pic = p_sys->p_pic;
switch( sts )
{
case BC_STS_SUCCESS:
if( !(proc_out.PoutFlags & BC_POUT_FLAGS_PIB_VALID) )
{
msg_Dbg( p_dec, "Invalid PIB" );
break;
}
if( !p_pic )
break;
if( (proc_out.PicInfo.flags & VDEC_FLAG_INTERLACED_SRC) &&
!(proc_out.PicInfo.flags & VDEC_FLAG_FIELDPAIR) )
return VLCDEC_SUCCESS;
p_pic->date = proc_out.PicInfo.timeStamp > 0 ?
FROM_BC_PTS(proc_out.PicInfo.timeStamp) : VLC_TICK_INVALID;
#if defined(DEBUG_CRYSTALHD)
msg_Dbg( p_dec, "TS Output is %"PRIu64, p_pic->date);
#endif
decoder_QueueVideo( p_dec, p_pic );
return VLCDEC_SUCCESS;
case BC_STS_DEC_NOT_OPEN:
case BC_STS_DEC_NOT_STARTED:
msg_Err( p_dec, "Decoder not opened or started" );
break;
case BC_STS_INV_ARG:
msg_Warn( p_dec, "Invalid arguments. Please report" );
break;
case BC_STS_FMT_CHANGE: 
p_dec->fmt_out.video.i_width = proc_out.PicInfo.width;
p_dec->fmt_out.video.i_height = proc_out.PicInfo.height;
if( proc_out.PicInfo.height == 1088 )
p_dec->fmt_out.video.i_height = 1080;
#define setAR( a, b, c ) case a: p_dec->fmt_out.video.i_sar_num = b; p_dec->fmt_out.video.i_sar_den = c; break;
switch( proc_out.PicInfo.aspect_ratio )
{
setAR( vdecAspectRatioSquare, 1, 1 )
setAR( vdecAspectRatio12_11, 12, 11 )
setAR( vdecAspectRatio10_11, 10, 11 )
setAR( vdecAspectRatio16_11, 16, 11 )
setAR( vdecAspectRatio40_33, 40, 33 )
setAR( vdecAspectRatio24_11, 24, 11 )
setAR( vdecAspectRatio20_11, 20, 11 )
setAR( vdecAspectRatio32_11, 32, 11 )
setAR( vdecAspectRatio80_33, 80, 33 )
setAR( vdecAspectRatio18_11, 18, 11 )
setAR( vdecAspectRatio15_11, 15, 11 )
setAR( vdecAspectRatio64_33, 64, 33 )
setAR( vdecAspectRatio160_99, 160, 99 )
setAR( vdecAspectRatio4_3, 4, 3 )
setAR( vdecAspectRatio16_9, 16, 9 )
setAR( vdecAspectRatio221_1, 221, 1 )
default: break;
}
#undef setAR
msg_Dbg( p_dec, "Format Change Detected [%i, %i], AR: %i/%i",
proc_out.PicInfo.width, proc_out.PicInfo.height,
p_dec->fmt_out.video.i_sar_num,
p_dec->fmt_out.video.i_sar_den );
break;
case BC_STS_NO_DATA:
if( BC_FUNC_PSYS(DtsIsEndOfStream)( p_sys->bcm_handle, &b_eos )
== BC_STS_SUCCESS )
if( b_eos )
msg_Dbg( p_dec, "End of Stream" );
break;
case BC_STS_TIMEOUT: 
msg_Err( p_dec, "ProcOutput timeout" );
break;
case BC_STS_IO_XFR_ERROR:
case BC_STS_IO_USER_ABORT:
case BC_STS_IO_ERROR:
msg_Err( p_dec, "ProcOutput return mode not implemented. Please report" );
break;
default:
msg_Err( p_dec, "Unknown return status. Please report %i", sts );
break;
}
if( p_pic )
picture_Release( p_pic );
return VLCDEC_SUCCESS;
}
#if 0
static void crystal_CopyPicture ( picture_t *p_pic, BC_DTS_PROC_OUT* p_out )
{
int i_dst_stride;
uint8_t *p_dst, *p_dst_end;
uint8_t *p_src = p_out->Ybuff;
p_dst = p_pic->p[0].p_pixels;
i_dst_stride = p_pic->p[0].i_pitch;
p_dst_end = p_dst + (i_dst_stride * p_out->PicInfo.height);
for( ; p_dst < p_dst_end; p_dst += i_dst_stride, p_src += (p_out->PicInfo.width * 2))
memcpy( p_dst, p_src, p_out->PicInfo.width * 2); 
}
#endif
static int crystal_insert_sps_pps( decoder_t *p_dec,
uint8_t *p_buf,
uint32_t i_buf_size)
{
decoder_sys_t *p_sys = p_dec->p_sys;
p_sys->i_sps_pps_size = 0;
p_sys->p_sps_pps_buf = h264_avcC_to_AnnexB_NAL( p_buf, i_buf_size,
&p_sys->i_sps_pps_size, &p_sys->i_nal_size );
return (p_sys->p_sps_pps_buf) ? VLC_SUCCESS : VLC_EGENERIC;
}
