#include <stdint.h>
#define BLKSIZE 1024
typedef struct ADPCMChannelStatus {
int predictor;
int16_t step_index;
int step;
int prev_sample;
int sample1;
int sample2;
int coeff1;
int coeff2;
int idelta;
} ADPCMChannelStatus;
