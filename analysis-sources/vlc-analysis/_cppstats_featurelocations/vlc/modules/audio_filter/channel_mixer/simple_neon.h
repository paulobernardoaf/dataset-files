
























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_cpu.h>





#define NEON_WRAPPER(in, out) void convert_##in##_to_##out##_neon_asm(float *dst, const float *src, int num, bool lfeChannel); static inline void DoWork_##in##_to_##out##_neon( filter_t *p_filter, block_t *p_in_buf, block_t *p_out_buf ) { const float *p_src = (const float *)p_in_buf->p_buffer; float *p_dest = (float *)p_out_buf->p_buffer; convert_##in##_to_##out##_neon_asm( p_dest, p_src, p_in_buf->i_nb_samples, p_filter->fmt_in.audio.i_physical_channels & AOUT_CHAN_LFE ); } static inline void (*GET_WORK_##in##_to_##out##_neon())(filter_t*, block_t*, block_t*) { return vlc_CPU_ARM_NEON() ? DoWork_##in##_to_##out##_neon : DoWork_##in##_to_##out; }













NEON_WRAPPER(7_x,2_0)
NEON_WRAPPER(5_x,2_0)
NEON_WRAPPER(4_0,2_0)
NEON_WRAPPER(3_x,2_0)
NEON_WRAPPER(7_x,1_0)
NEON_WRAPPER(5_x,1_0)
NEON_WRAPPER(7_x,4_0)
NEON_WRAPPER(5_x,4_0)



#define C_WRAPPER(in, out) static inline void (*GET_WORK_##in##_to_##out##_neon())(filter_t*, block_t*, block_t*) { return DoWork_##in##_to_##out; }





C_WRAPPER(4_0,1_0)
C_WRAPPER(3_x,1_0)
C_WRAPPER(2_x,1_0)
C_WRAPPER(6_1,2_0)
C_WRAPPER(7_x,5_x)
C_WRAPPER(6_1,5_x)
