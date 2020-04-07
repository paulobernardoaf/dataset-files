#include <stddef.h>
#include "libavutil/attributes.h"
#include "libavutil/avutil.h"
#include "libavutil/buffer.h"
#include "libavutil/dict.h"
#include "libavutil/frame.h"
#include "libavutil/log.h"
#include "libavutil/samplefmt.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libavfilter/version.h"
unsigned avfilter_version(void);
const char *avfilter_configuration(void);
const char *avfilter_license(void);
typedef struct AVFilterContext AVFilterContext;
typedef struct AVFilterLink AVFilterLink;
typedef struct AVFilterPad AVFilterPad;
typedef struct AVFilterFormats AVFilterFormats;
int avfilter_pad_count(const AVFilterPad *pads);
const char *avfilter_pad_get_name(const AVFilterPad *pads, int pad_idx);
enum AVMediaType avfilter_pad_get_type(const AVFilterPad *pads, int pad_idx);
#define AVFILTER_FLAG_DYNAMIC_INPUTS (1 << 0)
#define AVFILTER_FLAG_DYNAMIC_OUTPUTS (1 << 1)
#define AVFILTER_FLAG_SLICE_THREADS (1 << 2)
#define AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC (1 << 16)
#define AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL (1 << 17)
#define AVFILTER_FLAG_SUPPORT_TIMELINE (AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC | AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL)
typedef struct AVFilter {
const char *name;
const char *description;
const AVFilterPad *inputs;
const AVFilterPad *outputs;
const AVClass *priv_class;
int flags;
int (*preinit)(AVFilterContext *ctx);
int (*init)(AVFilterContext *ctx);
int (*init_dict)(AVFilterContext *ctx, AVDictionary **options);
void (*uninit)(AVFilterContext *ctx);
int (*query_formats)(AVFilterContext *);
int priv_size; 
int flags_internal; 
struct AVFilter *next;
int (*process_command)(AVFilterContext *, const char *cmd, const char *arg, char *res, int res_len, int flags);
int (*init_opaque)(AVFilterContext *ctx, void *opaque);
int (*activate)(AVFilterContext *ctx);
} AVFilter;
#define AVFILTER_THREAD_SLICE (1 << 0)
typedef struct AVFilterInternal AVFilterInternal;
struct AVFilterContext {
const AVClass *av_class; 
const AVFilter *filter; 
char *name; 
AVFilterPad *input_pads; 
AVFilterLink **inputs; 
unsigned nb_inputs; 
AVFilterPad *output_pads; 
AVFilterLink **outputs; 
unsigned nb_outputs; 
void *priv; 
struct AVFilterGraph *graph; 
int thread_type;
AVFilterInternal *internal;
struct AVFilterCommand *command_queue;
char *enable_str; 
void *enable; 
double *var_values; 
int is_disabled; 
AVBufferRef *hw_device_ctx;
int nb_threads;
unsigned ready;
int extra_hw_frames;
};
struct AVFilterLink {
AVFilterContext *src; 
AVFilterPad *srcpad; 
AVFilterContext *dst; 
AVFilterPad *dstpad; 
enum AVMediaType type; 
int w; 
int h; 
AVRational sample_aspect_ratio; 
uint64_t channel_layout; 
int sample_rate; 
int format; 
AVRational time_base;
AVFilterFormats *in_formats;
AVFilterFormats *out_formats;
AVFilterFormats *in_samplerates;
AVFilterFormats *out_samplerates;
struct AVFilterChannelLayouts *in_channel_layouts;
struct AVFilterChannelLayouts *out_channel_layouts;
int request_samples;
enum {
AVLINK_UNINIT = 0, 
AVLINK_STARTINIT, 
AVLINK_INIT 
} init_state;
struct AVFilterGraph *graph;
int64_t current_pts;
int64_t current_pts_us;
int age_index;
AVRational frame_rate;
AVFrame *partial_buf;
int partial_buf_size;
int min_samples;
int max_samples;
int channels;
unsigned flags;
int64_t frame_count_in, frame_count_out;
void *frame_pool;
int frame_wanted_out;
AVBufferRef *hw_frames_ctx;
#if !defined(FF_INTERNAL_FIELDS)
char reserved[0xF000];
#else 
FFFrameQueue fifo;
int frame_blocked_in;
int status_in;
int64_t status_in_pts;
int status_out;
#endif 
};
int avfilter_link(AVFilterContext *src, unsigned srcpad,
AVFilterContext *dst, unsigned dstpad);
void avfilter_link_free(AVFilterLink **link);
#if FF_API_FILTER_GET_SET
attribute_deprecated
int avfilter_link_get_channels(AVFilterLink *link);
#endif
attribute_deprecated
void avfilter_link_set_closed(AVFilterLink *link, int closed);
int avfilter_config_links(AVFilterContext *filter);
#define AVFILTER_CMD_FLAG_ONE 1 
#define AVFILTER_CMD_FLAG_FAST 2 
int avfilter_process_command(AVFilterContext *filter, const char *cmd, const char *arg, char *res, int res_len, int flags);
const AVFilter *av_filter_iterate(void **opaque);
#if FF_API_NEXT
attribute_deprecated
void avfilter_register_all(void);
attribute_deprecated
int avfilter_register(AVFilter *filter);
attribute_deprecated
const AVFilter *avfilter_next(const AVFilter *prev);
#endif
const AVFilter *avfilter_get_by_name(const char *name);
int avfilter_init_str(AVFilterContext *ctx, const char *args);
int avfilter_init_dict(AVFilterContext *ctx, AVDictionary **options);
void avfilter_free(AVFilterContext *filter);
int avfilter_insert_filter(AVFilterLink *link, AVFilterContext *filt,
unsigned filt_srcpad_idx, unsigned filt_dstpad_idx);
const AVClass *avfilter_get_class(void);
typedef struct AVFilterGraphInternal AVFilterGraphInternal;
typedef int (avfilter_action_func)(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs);
typedef int (avfilter_execute_func)(AVFilterContext *ctx, avfilter_action_func *func,
void *arg, int *ret, int nb_jobs);
typedef struct AVFilterGraph {
const AVClass *av_class;
AVFilterContext **filters;
unsigned nb_filters;
char *scale_sws_opts; 
#if FF_API_LAVR_OPTS
attribute_deprecated char *resample_lavr_opts; 
#endif
int thread_type;
int nb_threads;
AVFilterGraphInternal *internal;
void *opaque;
avfilter_execute_func *execute;
char *aresample_swr_opts; 
AVFilterLink **sink_links;
int sink_links_count;
unsigned disable_auto_convert;
} AVFilterGraph;
AVFilterGraph *avfilter_graph_alloc(void);
AVFilterContext *avfilter_graph_alloc_filter(AVFilterGraph *graph,
const AVFilter *filter,
const char *name);
AVFilterContext *avfilter_graph_get_filter(AVFilterGraph *graph, const char *name);
int avfilter_graph_create_filter(AVFilterContext **filt_ctx, const AVFilter *filt,
const char *name, const char *args, void *opaque,
AVFilterGraph *graph_ctx);
void avfilter_graph_set_auto_convert(AVFilterGraph *graph, unsigned flags);
enum {
AVFILTER_AUTO_CONVERT_ALL = 0, 
AVFILTER_AUTO_CONVERT_NONE = -1, 
};
int avfilter_graph_config(AVFilterGraph *graphctx, void *log_ctx);
void avfilter_graph_free(AVFilterGraph **graph);
typedef struct AVFilterInOut {
char *name;
AVFilterContext *filter_ctx;
int pad_idx;
struct AVFilterInOut *next;
} AVFilterInOut;
AVFilterInOut *avfilter_inout_alloc(void);
void avfilter_inout_free(AVFilterInOut **inout);
int avfilter_graph_parse(AVFilterGraph *graph, const char *filters,
AVFilterInOut *inputs, AVFilterInOut *outputs,
void *log_ctx);
int avfilter_graph_parse_ptr(AVFilterGraph *graph, const char *filters,
AVFilterInOut **inputs, AVFilterInOut **outputs,
void *log_ctx);
int avfilter_graph_parse2(AVFilterGraph *graph, const char *filters,
AVFilterInOut **inputs,
AVFilterInOut **outputs);
int avfilter_graph_send_command(AVFilterGraph *graph, const char *target, const char *cmd, const char *arg, char *res, int res_len, int flags);
int avfilter_graph_queue_command(AVFilterGraph *graph, const char *target, const char *cmd, const char *arg, int flags, double ts);
char *avfilter_graph_dump(AVFilterGraph *graph, const char *options);
int avfilter_graph_request_oldest(AVFilterGraph *graph);
