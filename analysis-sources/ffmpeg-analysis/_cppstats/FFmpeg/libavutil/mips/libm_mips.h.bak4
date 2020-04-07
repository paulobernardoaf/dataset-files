



















































#if !defined(AVUTIL_MIPS_LIBM_MIPS_H)
#define AVUTIL_MIPS_LIBM_MIPS_H

static av_always_inline av_const long int lrintf_mips(float x)
{
register int ret_int;

__asm__ volatile (
"cvt.w.s %[x], %[x] \n\t"
"mfc1 %[ret_int], %[x] \n\t"

:[x]"+f"(x), [ret_int]"=r"(ret_int)
);
return ret_int;
}

#undef lrintf
#define lrintf(x) lrintf_mips(x)

#define HAVE_LRINTF 1
#endif 
