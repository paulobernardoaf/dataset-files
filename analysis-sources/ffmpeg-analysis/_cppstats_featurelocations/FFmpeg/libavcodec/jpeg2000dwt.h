




















#if !defined(AVCODEC_JPEG2000DWT_H)
#define AVCODEC_JPEG2000DWT_H






#include <stdint.h>

#define FF_DWT_MAX_DECLVLS 32 
#define F_LFTG_K 1.230174104914001f
#define F_LFTG_X 0.812893066115961f

enum DWTType {
FF_DWT97,
FF_DWT53,
FF_DWT97_INT,
FF_DWT_NB
};

typedef struct DWTContext {

int linelen[FF_DWT_MAX_DECLVLS][2];
uint8_t mod[FF_DWT_MAX_DECLVLS][2]; 
uint8_t ndeclevels; 
uint8_t type; 
int32_t *i_linebuf; 
float *f_linebuf; 
} DWTContext;








int ff_jpeg2000_dwt_init(DWTContext *s, int border[2][2],
int decomp_levels, int type);

int ff_dwt_encode(DWTContext *s, void *t);
int ff_dwt_decode(DWTContext *s, void *t);

void ff_dwt_destroy(DWTContext *s);

#endif 
