#include "aacenc.h"
#define INT_STEREO_LOW_LIMIT 6100
struct AACISError {
int pass; 
int phase; 
float error; 
float dist1; 
float dist2; 
float ener01;
};
struct AACISError ff_aac_is_encoding_err(AACEncContext *s, ChannelElement *cpe,
int start, int w, int g, float ener0,
float ener1, float ener01,
int use_pcoeffs, int phase);
void ff_aac_search_for_is(AACEncContext *s, AVCodecContext *avctx, ChannelElement *cpe);
