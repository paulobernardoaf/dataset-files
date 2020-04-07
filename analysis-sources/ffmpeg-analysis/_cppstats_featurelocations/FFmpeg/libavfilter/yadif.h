

















#if !defined(AVFILTER_YADIF_H)
#define AVFILTER_YADIF_H

#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avfilter.h"

enum YADIFMode {
YADIF_MODE_SEND_FRAME = 0, 
YADIF_MODE_SEND_FIELD = 1, 
YADIF_MODE_SEND_FRAME_NOSPATIAL = 2, 
YADIF_MODE_SEND_FIELD_NOSPATIAL = 3, 
};

enum YADIFParity {
YADIF_PARITY_TFF = 0, 
YADIF_PARITY_BFF = 1, 
YADIF_PARITY_AUTO = -1, 
};

enum YADIFDeint {
YADIF_DEINT_ALL = 0, 
YADIF_DEINT_INTERLACED = 1, 
};

enum YADIFCurrentField {
YADIF_FIELD_BACK_END = -1, 
YADIF_FIELD_END = 0, 
YADIF_FIELD_NORMAL = 1, 
};

typedef struct YADIFContext {
const AVClass *class;

int mode; 
int parity; 
int deint; 

int frame_pending;

AVFrame *cur;
AVFrame *next;
AVFrame *prev;
AVFrame *out;

void (*filter)(AVFilterContext *ctx, AVFrame *dstpic, int parity, int tff);




void (*filter_line)(void *dst,
void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int parity, int mode);
void (*filter_edges)(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int parity, int mode);

const AVPixFmtDescriptor *csp;
int eof;
uint8_t *temp_line;
int temp_line_size;







int current_field; 
} YADIFContext;

void ff_yadif_init_x86(YADIFContext *yadif);

int ff_yadif_filter_frame(AVFilterLink *link, AVFrame *frame);

int ff_yadif_request_frame(AVFilterLink *link);

extern const AVOption ff_yadif_options[];

#endif 
