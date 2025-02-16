





















#if !defined(AVCODEC_SCPR3_H)
#define AVCODEC_SCPR3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"

typedef struct PixelModel3 {
uint8_t type;
uint8_t length;
uint8_t maxpos;
uint8_t fshift;
uint16_t size;
uint32_t cntsum;
uint8_t symbols[256];
uint16_t freqs[256];
uint16_t freqs1[256];
uint16_t cnts[256];
uint8_t dectab[32];
} PixelModel3;

typedef struct FillModel3 {
uint32_t cntsum;
uint16_t freqs[2][5];
uint16_t cnts[5];
uint8_t dectab[32];
} FillModel3;

typedef struct OpModel3 {
uint32_t cntsum;
uint16_t freqs[2][6];
uint16_t cnts[6];
uint8_t dectab[32];
} OpModel3;

typedef struct RunModel3 {
uint32_t cntsum;
uint16_t freqs[2][256];
uint16_t cnts[256];
uint8_t dectab[32];
} RunModel3;

typedef struct SxyModel3 {
uint32_t cntsum;
uint16_t freqs[2][16];
uint16_t cnts[16];
uint8_t dectab[32];
} SxyModel3;

typedef struct MVModel3 {
uint32_t cntsum;
uint16_t freqs[2][512];
uint16_t cnts[512];
uint8_t dectab[32];
} MVModel3;

#endif 
