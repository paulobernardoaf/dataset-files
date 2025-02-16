

















#if !defined(AVFILTER_OVERLAY_H)
#define AVFILTER_OVERLAY_H

#include "libavutil/eval.h"
#include "libavutil/pixdesc.h"
#include "framesync.h"
#include "avfilter.h"

enum var_name {
VAR_MAIN_W, VAR_MW,
VAR_MAIN_H, VAR_MH,
VAR_OVERLAY_W, VAR_OW,
VAR_OVERLAY_H, VAR_OH,
VAR_HSUB,
VAR_VSUB,
VAR_X,
VAR_Y,
VAR_N,
VAR_POS,
VAR_T,
VAR_VARS_NB
};

enum OverlayFormat {
OVERLAY_FORMAT_YUV420,
OVERLAY_FORMAT_YUV422,
OVERLAY_FORMAT_YUV444,
OVERLAY_FORMAT_RGB,
OVERLAY_FORMAT_GBRP,
OVERLAY_FORMAT_AUTO,
OVERLAY_FORMAT_NB
};

typedef struct OverlayContext {
const AVClass *class;
int x, y; 

uint8_t main_is_packed_rgb;
uint8_t main_rgba_map[4];
uint8_t main_has_alpha;
uint8_t overlay_is_packed_rgb;
uint8_t overlay_rgba_map[4];
uint8_t overlay_has_alpha;
int format; 
int alpha_format;
int eval_mode; 

FFFrameSync fs;

int main_pix_step[4]; 
int overlay_pix_step[4]; 
int hsub, vsub; 
const AVPixFmtDescriptor *main_desc; 

double var_values[VAR_VARS_NB];
char *x_expr, *y_expr;

AVExpr *x_pexpr, *y_pexpr;

int (*blend_row[4])(uint8_t *d, uint8_t *da, uint8_t *s, uint8_t *a, int w,
ptrdiff_t alinesize);
int (*blend_slice)(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs);
} OverlayContext;

void ff_overlay_init_x86(OverlayContext *s, int format, int pix_format,
int alpha_format, int main_has_alpha);

#endif 
