#include "libavutil/pixdesc.h"
#include "avfilter.h"
#define HIST_SIZE 4
typedef int (*ff_idet_filter_func)(const uint8_t *a, const uint8_t *b, const uint8_t *c, int w);
typedef enum {
TFF,
BFF,
PROGRESSIVE,
UNDETERMINED,
} Type;
typedef enum {
REPEAT_NONE,
REPEAT_TOP,
REPEAT_BOTTOM,
} RepeatedField;
typedef struct IDETContext {
const AVClass *class;
float interlace_threshold;
float progressive_threshold;
float repeat_threshold;
float half_life;
uint64_t decay_coefficient;
Type last_type;
uint64_t repeats[3];
uint64_t prestat[4];
uint64_t poststat[4];
uint64_t total_repeats[3];
uint64_t total_prestat[4];
uint64_t total_poststat[4];
uint8_t history[HIST_SIZE];
AVFrame *cur;
AVFrame *next;
AVFrame *prev;
ff_idet_filter_func filter_line;
int interlaced_flag_accuracy;
int analyze_interlaced_flag;
int analyze_interlaced_flag_done;
const AVPixFmtDescriptor *csp;
int eof;
} IDETContext;
void ff_idet_init_x86(IDETContext *idet, int for_16b);
int ff_idet_filter_line_c(const uint8_t *a, const uint8_t *b, const uint8_t *c, int w);
int ff_idet_filter_line_c_16bit(const uint16_t *a, const uint16_t *b, const uint16_t *c, int w);
