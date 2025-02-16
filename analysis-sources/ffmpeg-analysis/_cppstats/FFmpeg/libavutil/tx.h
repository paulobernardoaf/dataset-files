#include <stdint.h>
#include <stddef.h>
typedef struct AVTXContext AVTXContext;
typedef struct AVComplexFloat {
float re, im;
} AVComplexFloat;
typedef struct AVComplexDouble {
double re, im;
} AVComplexDouble;
typedef struct AVComplexInt32 {
int32_t re, im;
} AVComplexInt32;
enum AVTXType {
AV_TX_FLOAT_FFT = 0,
AV_TX_FLOAT_MDCT = 1,
AV_TX_DOUBLE_FFT = 2,
AV_TX_DOUBLE_MDCT = 3,
AV_TX_INT32_FFT = 4,
AV_TX_INT32_MDCT = 5,
};
typedef void (*av_tx_fn)(AVTXContext *s, void *out, void *in, ptrdiff_t stride);
int av_tx_init(AVTXContext **ctx, av_tx_fn *tx, enum AVTXType type,
int inv, int len, const void *scale, uint64_t flags);
void av_tx_uninit(AVTXContext **ctx);
