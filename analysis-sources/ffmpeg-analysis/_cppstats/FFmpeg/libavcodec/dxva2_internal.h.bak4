





















#if !defined(AVCODEC_DXVA2_INTERNAL_H)
#define AVCODEC_DXVA2_INTERNAL_H

#define COBJMACROS

#include "config.h"


#include <objbase.h>

#if CONFIG_DXVA2
#include "dxva2.h"
#include "libavutil/hwcontext_dxva2.h"
#endif
#if CONFIG_D3D11VA
#include "d3d11va.h"
#include "libavutil/hwcontext_d3d11va.h"
#endif
#if HAVE_DXVA_H


#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#undef _CRT_BUILD_DESKTOP_APP
#define _CRT_BUILD_DESKTOP_APP 0
#include <dxva.h>
#endif

#include "libavutil/hwcontext.h"

#include "avcodec.h"
#include "internal.h"

typedef void DECODER_BUFFER_DESC;

typedef union {
#if CONFIG_D3D11VA
struct AVD3D11VAContext d3d11va;
#endif
#if CONFIG_DXVA2
struct dxva_context dxva2;
#endif
} AVDXVAContext;

typedef struct FFDXVASharedContext {
AVBufferRef *decoder_ref;


uint64_t workaround;


enum AVPixelFormat pix_fmt;

AVHWDeviceContext *device_ctx;

#if CONFIG_D3D11VA
ID3D11VideoDecoder *d3d11_decoder;
D3D11_VIDEO_DECODER_CONFIG d3d11_config;
ID3D11VideoDecoderOutputView **d3d11_views;
int nb_d3d11_views;
ID3D11Texture2D *d3d11_texture;
#endif

#if CONFIG_DXVA2
IDirectXVideoDecoder *dxva2_decoder;
IDirectXVideoDecoderService *dxva2_service;
DXVA2_ConfigPictureDecode dxva2_config;
#endif



AVDXVAContext ctx;
} FFDXVASharedContext;

#define DXVA_SHARED_CONTEXT(avctx) ((FFDXVASharedContext *)((avctx)->internal->hwaccel_priv_data))

#define DXVA_CONTEXT(avctx) (AVDXVAContext *)((avctx)->hwaccel_context ? (avctx)->hwaccel_context : (&(DXVA_SHARED_CONTEXT(avctx)->ctx)))

#define D3D11VA_CONTEXT(ctx) (&ctx->d3d11va)
#define DXVA2_CONTEXT(ctx) (&ctx->dxva2)

#if CONFIG_D3D11VA && CONFIG_DXVA2
#define DXVA_CONTEXT_WORKAROUND(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? ctx->d3d11va.workaround : ctx->dxva2.workaround)
#define DXVA_CONTEXT_COUNT(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? ctx->d3d11va.surface_count : ctx->dxva2.surface_count)
#define DXVA_CONTEXT_DECODER(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? (void *)ctx->d3d11va.decoder : (void *)ctx->dxva2.decoder)
#define DXVA_CONTEXT_REPORT_ID(avctx, ctx) (*(ff_dxva2_is_d3d11(avctx) ? &ctx->d3d11va.report_id : &ctx->dxva2.report_id))
#define DXVA_CONTEXT_CFG(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? (void *)ctx->d3d11va.cfg : (void *)ctx->dxva2.cfg)
#define DXVA_CONTEXT_CFG_BITSTREAM(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? ctx->d3d11va.cfg->ConfigBitstreamRaw : ctx->dxva2.cfg->ConfigBitstreamRaw)
#define DXVA_CONTEXT_CFG_INTRARESID(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? ctx->d3d11va.cfg->ConfigIntraResidUnsigned : ctx->dxva2.cfg->ConfigIntraResidUnsigned)
#define DXVA_CONTEXT_CFG_RESIDACCEL(avctx, ctx) (ff_dxva2_is_d3d11(avctx) ? ctx->d3d11va.cfg->ConfigResidDiffAccelerator : ctx->dxva2.cfg->ConfigResidDiffAccelerator)
#define DXVA_CONTEXT_VALID(avctx, ctx) (DXVA_CONTEXT_DECODER(avctx, ctx) && DXVA_CONTEXT_CFG(avctx, ctx) && (ff_dxva2_is_d3d11(avctx) || ctx->dxva2.surface_count))


#elif CONFIG_DXVA2
#define DXVA_CONTEXT_WORKAROUND(avctx, ctx) (ctx->dxva2.workaround)
#define DXVA_CONTEXT_COUNT(avctx, ctx) (ctx->dxva2.surface_count)
#define DXVA_CONTEXT_DECODER(avctx, ctx) (ctx->dxva2.decoder)
#define DXVA_CONTEXT_REPORT_ID(avctx, ctx) (*(&ctx->dxva2.report_id))
#define DXVA_CONTEXT_CFG(avctx, ctx) (ctx->dxva2.cfg)
#define DXVA_CONTEXT_CFG_BITSTREAM(avctx, ctx) (ctx->dxva2.cfg->ConfigBitstreamRaw)
#define DXVA_CONTEXT_CFG_INTRARESID(avctx, ctx) (ctx->dxva2.cfg->ConfigIntraResidUnsigned)
#define DXVA_CONTEXT_CFG_RESIDACCEL(avctx, ctx) (ctx->dxva2.cfg->ConfigResidDiffAccelerator)
#define DXVA_CONTEXT_VALID(avctx, ctx) (ctx->dxva2.decoder && ctx->dxva2.cfg && ctx->dxva2.surface_count)
#elif CONFIG_D3D11VA
#define DXVA_CONTEXT_WORKAROUND(avctx, ctx) (ctx->d3d11va.workaround)
#define DXVA_CONTEXT_COUNT(avctx, ctx) (ctx->d3d11va.surface_count)
#define DXVA_CONTEXT_DECODER(avctx, ctx) (ctx->d3d11va.decoder)
#define DXVA_CONTEXT_REPORT_ID(avctx, ctx) (*(&ctx->d3d11va.report_id))
#define DXVA_CONTEXT_CFG(avctx, ctx) (ctx->d3d11va.cfg)
#define DXVA_CONTEXT_CFG_BITSTREAM(avctx, ctx) (ctx->d3d11va.cfg->ConfigBitstreamRaw)
#define DXVA_CONTEXT_CFG_INTRARESID(avctx, ctx) (ctx->d3d11va.cfg->ConfigIntraResidUnsigned)
#define DXVA_CONTEXT_CFG_RESIDACCEL(avctx, ctx) (ctx->d3d11va.cfg->ConfigResidDiffAccelerator)
#define DXVA_CONTEXT_VALID(avctx, ctx) (ctx->d3d11va.decoder && ctx->d3d11va.cfg)
#endif

unsigned ff_dxva2_get_surface_index(const AVCodecContext *avctx,
const AVDXVAContext *,
const AVFrame *frame);

int ff_dxva2_commit_buffer(AVCodecContext *, AVDXVAContext *,
DECODER_BUFFER_DESC *,
unsigned type, const void *data, unsigned size,
unsigned mb_count);


int ff_dxva2_common_end_frame(AVCodecContext *, AVFrame *,
const void *pp, unsigned pp_size,
const void *qm, unsigned qm_size,
int (*commit_bs_si)(AVCodecContext *,
DECODER_BUFFER_DESC *bs,
DECODER_BUFFER_DESC *slice));

int ff_dxva2_decode_init(AVCodecContext *avctx);

int ff_dxva2_decode_uninit(AVCodecContext *avctx);

int ff_dxva2_common_frame_params(AVCodecContext *avctx,
AVBufferRef *hw_frames_ctx);

int ff_dxva2_is_d3d11(const AVCodecContext *avctx);

#endif 
