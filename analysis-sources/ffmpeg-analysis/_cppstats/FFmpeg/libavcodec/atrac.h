typedef struct AtracGainInfo {
int num_points; 
int lev_code[7]; 
int loc_code[7]; 
} AtracGainInfo;
typedef struct AtracGCContext {
float gain_tab1[16]; 
float gain_tab2[31]; 
int id2exp_offset; 
int loc_scale; 
int loc_size; 
} AtracGCContext;
extern float ff_atrac_sf_table[64];
void ff_atrac_generate_tables(void);
void ff_atrac_init_gain_compensation(AtracGCContext *gctx, int id2exp_offset,
int loc_scale);
void ff_atrac_gain_compensation(AtracGCContext *gctx, float *in, float *prev,
AtracGainInfo *gc_now, AtracGainInfo *gc_next,
int num_samples, float *out);
void ff_atrac_iqmf(float *inlo, float *inhi, unsigned int nIn, float *pOut,
float *delayBuf, float *temp);
