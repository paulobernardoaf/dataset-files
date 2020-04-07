#include "libavutil/lfg.h"
#include "avfilter.h"
#define MAX_NOISE 5120
#define MAX_SHIFT 1024
#define MAX_RES (MAX_NOISE-MAX_SHIFT)
#define NOISE_UNIFORM 1
#define NOISE_TEMPORAL 2
#define NOISE_AVERAGED 8
#define NOISE_PATTERN 16
typedef struct FilterParams {
int strength;
unsigned flags;
AVLFG lfg;
int seed;
int8_t *noise;
int8_t *prev_shift[MAX_RES][3];
int rand_shift[MAX_RES];
int rand_shift_init;
} FilterParams;
typedef struct NoiseContext {
const AVClass *class;
int nb_planes;
int bytewidth[4];
int height[4];
FilterParams all;
FilterParams param[4];
void (*line_noise)(uint8_t *dst, const uint8_t *src, const int8_t *noise, int len, int shift);
void (*line_noise_avg)(uint8_t *dst, const uint8_t *src, int len, const int8_t * const *shift);
} NoiseContext;
void ff_line_noise_c(uint8_t *dst, const uint8_t *src, const int8_t *noise, int len, int shift);
void ff_line_noise_avg_c(uint8_t *dst, const uint8_t *src, int len, const int8_t * const *shift);
void ff_noise_init_x86(NoiseContext *n);
