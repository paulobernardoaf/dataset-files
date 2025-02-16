#include <stdint.h>
typedef int DWTELEM;
typedef short IDWTELEM;
#define MAX_DWT_SUPPORT 8
#define MAX_DECOMPOSITIONS 8
typedef struct DWTCompose {
uint8_t *b[MAX_DWT_SUPPORT];
int y;
} DWTCompose;
typedef struct DWTPlane {
int width;
int height;
int stride;
uint8_t *buf;
uint8_t *buf_base;
uint8_t *tmp;
} DWTPlane;
struct DWTContext;
typedef void (*vertical_compose_2tap)(uint8_t *b0, uint8_t *b1, int width);
typedef void (*vertical_compose_3tap)(uint8_t *b0, uint8_t *b1, uint8_t *b2, int width);
typedef void (*vertical_compose_5tap)(uint8_t *b0, uint8_t *b1, uint8_t *b2, uint8_t *b3, uint8_t *b4, int width);
typedef void (*vertical_compose_9tap)(uint8_t *dst, uint8_t *b[8], int width);
typedef struct DWTContext {
uint8_t *buffer;
uint8_t *temp;
int width;
int height;
int stride;
int decomposition_count;
int support;
void (*spatial_compose)(struct DWTContext *cs, int level, int width, int height, int stride);
void (*vertical_compose_l0)(void);
void (*vertical_compose_h0)(void);
void (*vertical_compose_l1)(void);
void (*vertical_compose_h1)(void);
void (*vertical_compose)(void); 
void (*horizontal_compose)(uint8_t *b, uint8_t *tmp, int width);
DWTCompose cs[MAX_DECOMPOSITIONS];
} DWTContext;
enum dwt_type {
DWT_SNOW_DAUB9_7,
DWT_SNOW_LEGALL5_3,
DWT_DIRAC_DD9_7,
DWT_DIRAC_LEGALL5_3,
DWT_DIRAC_DD13_7,
DWT_DIRAC_HAAR0,
DWT_DIRAC_HAAR1,
DWT_DIRAC_FIDELITY,
DWT_DIRAC_DAUB9_7,
DWT_NUM_TYPES
};
int ff_spatial_idwt_init(DWTContext *d, DWTPlane *p, enum dwt_type type,
int decomposition_count, int bit_depth);
void ff_spatial_idwt_init_x86(DWTContext *d, enum dwt_type type);
void ff_spatial_idwt_slice2(DWTContext *d, int y);
#define COMPOSE_53iL0(b0, b1, b2)(b1 - (unsigned)((int)(b0 + (unsigned)(b2) + 2) >> 2))
#define COMPOSE_DIRAC53iH0(b0, b1, b2)(b1 + (unsigned)((int)(b0 + (unsigned)(b2) + 1) >> 1))
#define COMPOSE_DD97iH0(b0, b1, b2, b3, b4)(int)(((unsigned)(b2) + ((int)(9U*b1 + 9U*b3 - b4 - b0 + 8) >> 4)))
#define COMPOSE_DD137iL0(b0, b1, b2, b3, b4)(int)(((unsigned)(b2) - ((int)(9U*b1 + 9U*b3 - b4 - b0 + 16) >> 5)))
#define COMPOSE_HAARiL0(b0, b1)((int)(b0 - (unsigned)((int)(b1 + 1U) >> 1)))
#define COMPOSE_HAARiH0(b0, b1)((int)(b0 + (unsigned)(b1)))
#define COMPOSE_FIDELITYiL0(b0, b1, b2, b3, b4, b5, b6, b7, b8)((unsigned)b4 - ((int)(-8*(b0+(unsigned)b8) + 21*(b1+(unsigned)b7) - 46*(b2+(unsigned)b6) + 161*(b3+(unsigned)b5) + 128) >> 8))
#define COMPOSE_FIDELITYiH0(b0, b1, b2, b3, b4, b5, b6, b7, b8)((unsigned)b4 + ((int)(-2*(b0+(unsigned)b8) + 10*(b1+(unsigned)b7) - 25*(b2+(unsigned)b6) + 81*(b3+(unsigned)b5) + 128) >> 8))
#define COMPOSE_DAUB97iL1(b0, b1, b2)((unsigned)(b1) - ((int)(1817*(b0 + (unsigned)b2) + 2048) >> 12))
#define COMPOSE_DAUB97iH1(b0, b1, b2)((unsigned)(b1) - ((int)( 113*(b0 + (unsigned)b2) + 64) >> 7))
#define COMPOSE_DAUB97iL0(b0, b1, b2)((unsigned)(b1) + ((int)( 217*(b0 + (unsigned)b2) + 2048) >> 12))
#define COMPOSE_DAUB97iH0(b0, b1, b2)((unsigned)(b1) + ((int)(6497*(b0 + (unsigned)b2) + 2048) >> 12))
