#include "bufferqueue.h"
enum EOFAction {
EOF_ACTION_REPEAT,
EOF_ACTION_ENDALL,
EOF_ACTION_PASS
};
enum FFFrameSyncExtMode {
EXT_STOP,
EXT_NULL,
EXT_INFINITY,
};
typedef struct FFFrameSyncIn {
enum FFFrameSyncExtMode before;
enum FFFrameSyncExtMode after;
AVRational time_base;
AVFrame *frame;
AVFrame *frame_next;
int64_t pts;
int64_t pts_next;
uint8_t have_next;
uint8_t state;
unsigned sync;
} FFFrameSyncIn;
typedef struct FFFrameSync {
const AVClass *class;
AVFilterContext *parent;
unsigned nb_in;
AVRational time_base;
int64_t pts;
int (*on_event)(struct FFFrameSync *fs);
void *opaque;
unsigned in_request;
unsigned sync_level;
uint8_t frame_ready;
uint8_t eof;
FFFrameSyncIn *in;
int opt_repeatlast;
int opt_shortest;
int opt_eof_action;
} FFFrameSync;
const AVClass *ff_framesync_get_class(void);
void ff_framesync_preinit(FFFrameSync *fs);
int ff_framesync_init(FFFrameSync *fs, AVFilterContext *parent, unsigned nb_in);
int ff_framesync_configure(FFFrameSync *fs);
void ff_framesync_uninit(FFFrameSync *fs);
int ff_framesync_get_frame(FFFrameSync *fs, unsigned in, AVFrame **rframe,
unsigned get);
int ff_framesync_activate(FFFrameSync *fs);
int ff_framesync_init_dualinput(FFFrameSync *fs, AVFilterContext *parent);
int ff_framesync_dualinput_get(FFFrameSync *fs, AVFrame **f0, AVFrame **f1);
int ff_framesync_dualinput_get_writable(FFFrameSync *fs, AVFrame **f0, AVFrame **f1);
#define FRAMESYNC_DEFINE_CLASS(name, context, field) static int name##_framesync_preinit(AVFilterContext *ctx) { context *s = ctx->priv; ff_framesync_preinit(&s->field); return 0; } static const AVClass *name##_child_class_next(const AVClass *prev) { return prev ? NULL : ff_framesync_get_class(); } static void *name##_child_next(void *obj, void *prev) { context *s = obj; s->fs.class = ff_framesync_get_class(); return prev ? NULL : &s->field; } static const AVClass name##_class = { .class_name = #name, .item_name = av_default_item_name, .option = name##_options, .version = LIBAVUTIL_VERSION_INT, .category = AV_CLASS_CATEGORY_FILTER, .child_class_next = name##_child_class_next, .child_next = name##_child_next, }
