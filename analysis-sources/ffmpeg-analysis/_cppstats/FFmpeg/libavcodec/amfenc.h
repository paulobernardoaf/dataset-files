#include <AMF/core/Factory.h>
#include <AMF/components/VideoEncoderVCE.h>
#include <AMF/components/VideoEncoderHEVC.h>
#include "libavutil/fifo.h"
#include "avcodec.h"
typedef struct AmfTraceWriter {
AMFTraceWriterVtbl *vtbl;
AVCodecContext *avctx;
} AmfTraceWriter;
typedef struct AmfContext {
AVClass *avclass;
amf_handle library; 
AMFFactory *factory; 
AMFDebug *debug; 
AMFTrace *trace; 
amf_uint64 version; 
AmfTraceWriter tracer; 
AMFContext *context; 
AMFComponent *encoder; 
amf_bool eof; 
AMF_SURFACE_FORMAT format; 
AVBufferRef *hw_device_ctx; 
AVBufferRef *hw_frames_ctx; 
int hwsurfaces_in_queue;
int hwsurfaces_in_queue_max;
int delayed_drain;
AMFSurface *delayed_surface;
AVFrame *delayed_frame;
AVFifoBuffer *timestamp_list;
int64_t dts_delay;
int log_to_dbg;
int usage;
int profile;
int level;
int preanalysis;
int quality;
int b_frame_delta_qp;
int ref_b_frame_delta_qp;
int rate_control_mode;
int enforce_hrd;
int filler_data;
int enable_vbaq;
int skip_frame;
int qp_i;
int qp_p;
int qp_b;
int max_au_size;
int header_spacing;
int b_frame_ref;
int intra_refresh_mb;
int coding_mode;
int me_half_pel;
int me_quarter_pel;
int aud;
int gops_per_idr;
int header_insertion_mode;
int min_qp_i;
int max_qp_i;
int min_qp_p;
int max_qp_p;
int tier;
} AmfContext;
int ff_amf_encode_init(AVCodecContext *avctx);
int ff_amf_encode_close(AVCodecContext *avctx);
int ff_amf_send_frame(AVCodecContext *avctx, const AVFrame *frame);
int ff_amf_receive_packet(AVCodecContext *avctx, AVPacket *avpkt);
extern const enum AVPixelFormat ff_amf_pix_fmts[];
#define AMF_RETURN_IF_FALSE(avctx, exp, ret_value, ...) if (!(exp)) { av_log(avctx, AV_LOG_ERROR, __VA_ARGS__); return ret_value; }
