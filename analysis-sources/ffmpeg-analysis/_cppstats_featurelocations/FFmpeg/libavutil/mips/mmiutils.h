






















#if !defined(AVUTIL_MIPS_MMIUTILS_H)
#define AVUTIL_MIPS_MMIUTILS_H

#include "config.h"
#include "libavutil/mips/asmdefs.h"

#if HAVE_LOONGSON2

#define DECLARE_VAR_LOW32 int32_t low32
#define RESTRICT_ASM_LOW32 [low32]"=&r"(low32),
#define DECLARE_VAR_ALL64 int64_t all64
#define RESTRICT_ASM_ALL64 [all64]"=&r"(all64),
#define DECLARE_VAR_ADDRT mips_reg addrt
#define RESTRICT_ASM_ADDRT [addrt]"=&r"(addrt),

#define MMI_LWX(reg, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" "lw "#reg", "#bias"(%[addrt]) \n\t"



#define MMI_SWX(reg, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" "sw "#reg", "#bias"(%[addrt]) \n\t"



#define MMI_LDX(reg, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" "ld "#reg", "#bias"(%[addrt]) \n\t"



#define MMI_SDX(reg, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" "sd "#reg", "#bias"(%[addrt]) \n\t"



#define MMI_LWC1(fp, addr, bias) "lwc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_ULWC1(fp, addr, bias) "ulw %[low32], "#bias"("#addr") \n\t" "mtc1 %[low32], "#fp" \n\t"



#define MMI_LWXC1(fp, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" MMI_LWC1(fp, %[addrt], bias)



#define MMI_SWC1(fp, addr, bias) "swc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_USWC1(fp, addr, bias) "mfc1 %[low32], "#fp" \n\t" "usw %[low32], "#bias"("#addr") \n\t"



#define MMI_SWXC1(fp, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" MMI_SWC1(fp, %[addrt], bias)



#define MMI_LDC1(fp, addr, bias) "ldc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_ULDC1(fp, addr, bias) "uld %[all64], "#bias"("#addr") \n\t" "dmtc1 %[all64], "#fp" \n\t"



#define MMI_LDXC1(fp, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" MMI_LDC1(fp, %[addrt], bias)



#define MMI_SDC1(fp, addr, bias) "sdc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_USDC1(fp, addr, bias) "dmfc1 %[all64], "#fp" \n\t" "usd %[all64], "#bias"("#addr") \n\t"



#define MMI_SDXC1(fp, addr, stride, bias) PTR_ADDU "%[addrt], "#addr", "#stride" \n\t" MMI_SDC1(fp, %[addrt], bias)



#define MMI_LQ(reg1, reg2, addr, bias) "ld "#reg1", "#bias"("#addr") \n\t" "ld "#reg2", 8+"#bias"("#addr") \n\t"



#define MMI_SQ(reg1, reg2, addr, bias) "sd "#reg1", "#bias"("#addr") \n\t" "sd "#reg2", 8+"#bias"("#addr") \n\t"



#define MMI_LQC1(fp1, fp2, addr, bias) "ldc1 "#fp1", "#bias"("#addr") \n\t" "ldc1 "#fp2", 8+"#bias"("#addr") \n\t"



#define MMI_SQC1(fp1, fp2, addr, bias) "sdc1 "#fp1", "#bias"("#addr") \n\t" "sdc1 "#fp2", 8+"#bias"("#addr") \n\t"



#elif HAVE_LOONGSON3 

#define DECLARE_VAR_ALL64
#define RESTRICT_ASM_ALL64
#define DECLARE_VAR_ADDRT
#define RESTRICT_ASM_ADDRT

#define MMI_LWX(reg, addr, stride, bias) "gslwx "#reg", "#bias"("#addr", "#stride") \n\t"


#define MMI_SWX(reg, addr, stride, bias) "gsswx "#reg", "#bias"("#addr", "#stride") \n\t"


#define MMI_LDX(reg, addr, stride, bias) "gsldx "#reg", "#bias"("#addr", "#stride") \n\t"


#define MMI_SDX(reg, addr, stride, bias) "gssdx "#reg", "#bias"("#addr", "#stride") \n\t"


#define MMI_LWC1(fp, addr, bias) "lwc1 "#fp", "#bias"("#addr") \n\t"


#if _MIPS_SIM == _ABIO32 

#define DECLARE_VAR_LOW32 int32_t low32
#define RESTRICT_ASM_LOW32 [low32]"=&r"(low32),

#define MMI_ULWC1(fp, addr, bias) "ulw %[low32], "#bias"("#addr") \n\t" "mtc1 %[low32], "#fp" \n\t"



#else 

#define DECLARE_VAR_LOW32
#define RESTRICT_ASM_LOW32

#define MMI_ULWC1(fp, addr, bias) "gslwlc1 "#fp", 3+"#bias"("#addr") \n\t" "gslwrc1 "#fp", "#bias"("#addr") \n\t"



#endif 

#define MMI_LWXC1(fp, addr, stride, bias) "gslwxc1 "#fp", "#bias"("#addr", "#stride") \n\t"


#define MMI_SWC1(fp, addr, bias) "swc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_USWC1(fp, addr, bias) "gsswlc1 "#fp", 3+"#bias"("#addr") \n\t" "gsswrc1 "#fp", "#bias"("#addr") \n\t"



#define MMI_SWXC1(fp, addr, stride, bias) "gsswxc1 "#fp", "#bias"("#addr", "#stride") \n\t"


#define MMI_LDC1(fp, addr, bias) "ldc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_ULDC1(fp, addr, bias) "gsldlc1 "#fp", 7+"#bias"("#addr") \n\t" "gsldrc1 "#fp", "#bias"("#addr") \n\t"



#define MMI_LDXC1(fp, addr, stride, bias) "gsldxc1 "#fp", "#bias"("#addr", "#stride") \n\t"


#define MMI_SDC1(fp, addr, bias) "sdc1 "#fp", "#bias"("#addr") \n\t"


#define MMI_USDC1(fp, addr, bias) "gssdlc1 "#fp", 7+"#bias"("#addr") \n\t" "gssdrc1 "#fp", "#bias"("#addr") \n\t"



#define MMI_SDXC1(fp, addr, stride, bias) "gssdxc1 "#fp", "#bias"("#addr", "#stride") \n\t"


#define MMI_LQ(reg1, reg2, addr, bias) "gslq "#reg1", "#reg2", "#bias"("#addr") \n\t"


#define MMI_SQ(reg1, reg2, addr, bias) "gssq "#reg1", "#reg2", "#bias"("#addr") \n\t"


#define MMI_LQC1(fp1, fp2, addr, bias) "gslqc1 "#fp1", "#fp2", "#bias"("#addr") \n\t"


#define MMI_SQC1(fp1, fp2, addr, bias) "gssqc1 "#fp1", "#fp2", "#bias"("#addr") \n\t"


#endif 




#define BACKUP_REG LOCAL_ALIGNED_16(double, temp_backup_reg, [8]); if (_MIPS_SIM == _ABI64) __asm__ volatile ( "gssqc1 $f25, $f24, 0x00(%[temp]) \n\t" "gssqc1 $f27, $f26, 0x10(%[temp]) \n\t" "gssqc1 $f29, $f28, 0x20(%[temp]) \n\t" "gssqc1 $f31, $f30, 0x30(%[temp]) \n\t" : : [temp]"r"(temp_backup_reg) : "memory" ); else __asm__ volatile ( "gssqc1 $f22, $f20, 0x00(%[temp]) \n\t" "gssqc1 $f26, $f24, 0x10(%[temp]) \n\t" "gssqc1 $f30, $f28, 0x20(%[temp]) \n\t" : : [temp]"r"(temp_backup_reg) : "memory" );
























#define RECOVER_REG if (_MIPS_SIM == _ABI64) __asm__ volatile ( "gslqc1 $f25, $f24, 0x00(%[temp]) \n\t" "gslqc1 $f27, $f26, 0x10(%[temp]) \n\t" "gslqc1 $f29, $f28, 0x20(%[temp]) \n\t" "gslqc1 $f31, $f30, 0x30(%[temp]) \n\t" : : [temp]"r"(temp_backup_reg) : "memory" ); else __asm__ volatile ( "gslqc1 $f22, $f20, 0x00(%[temp]) \n\t" "gslqc1 $f26, $f24, 0x10(%[temp]) \n\t" "gslqc1 $f30, $f28, 0x20(%[temp]) \n\t" : : [temp]"r"(temp_backup_reg) : "memory" );

























#define TRANSPOSE_2W(fr_i0, fr_i1, fr_o0, fr_o1) "punpcklwd "#fr_o0", "#fr_i0", "#fr_i1" \n\t" "punpckhwd "#fr_o1", "#fr_i0", "#fr_i1" \n\t"








#define TRANSPOSE_4H(fr_i0, fr_i1, fr_i2, fr_i3, fr_t0, fr_t1, fr_t2, fr_t3) "punpcklhw "#fr_t0", "#fr_i0", "#fr_i1" \n\t" "punpckhhw "#fr_t1", "#fr_i0", "#fr_i1" \n\t" "punpcklhw "#fr_t2", "#fr_i2", "#fr_i3" \n\t" "punpckhhw "#fr_t3", "#fr_i2", "#fr_i3" \n\t" "punpcklwd "#fr_i0", "#fr_t0", "#fr_t2" \n\t" "punpckhwd "#fr_i1", "#fr_t0", "#fr_t2" \n\t" "punpcklwd "#fr_i2", "#fr_t1", "#fr_t3" \n\t" "punpckhwd "#fr_i3", "#fr_t1", "#fr_t3" \n\t"















#define TRANSPOSE_8B(fr_i0, fr_i1, fr_i2, fr_i3, fr_i4, fr_i5, fr_i6, fr_i7, fr_t0, fr_t1, fr_t2, fr_t3) "punpcklbh "#fr_t0", "#fr_i0", "#fr_i1" \n\t" "punpckhbh "#fr_t1", "#fr_i0", "#fr_i1" \n\t" "punpcklbh "#fr_t2", "#fr_i2", "#fr_i3" \n\t" "punpckhbh "#fr_t3", "#fr_i2", "#fr_i3" \n\t" "punpcklbh "#fr_i0", "#fr_i4", "#fr_i5" \n\t" "punpckhbh "#fr_i1", "#fr_i4", "#fr_i5" \n\t" "punpcklbh "#fr_i2", "#fr_i6", "#fr_i7" \n\t" "punpckhbh "#fr_i3", "#fr_i6", "#fr_i7" \n\t" "punpcklhw "#fr_i4", "#fr_t0", "#fr_t2" \n\t" "punpckhhw "#fr_i5", "#fr_t0", "#fr_t2" \n\t" "punpcklhw "#fr_i6", "#fr_t1", "#fr_t3" \n\t" "punpckhhw "#fr_i7", "#fr_t1", "#fr_t3" \n\t" "punpcklhw "#fr_t0", "#fr_i0", "#fr_i2" \n\t" "punpckhhw "#fr_t1", "#fr_i0", "#fr_i2" \n\t" "punpcklhw "#fr_t2", "#fr_i1", "#fr_i3" \n\t" "punpckhhw "#fr_t3", "#fr_i1", "#fr_i3" \n\t" "punpcklwd "#fr_i0", "#fr_i4", "#fr_t0" \n\t" "punpckhwd "#fr_i1", "#fr_i4", "#fr_t0" \n\t" "punpcklwd "#fr_i2", "#fr_i5", "#fr_t1" \n\t" "punpckhwd "#fr_i3", "#fr_i5", "#fr_t1" \n\t" "punpcklwd "#fr_i4", "#fr_i6", "#fr_t2" \n\t" "punpckhwd "#fr_i5", "#fr_i6", "#fr_t2" \n\t" "punpcklwd "#fr_i6", "#fr_i7", "#fr_t3" \n\t" "punpckhwd "#fr_i7", "#fr_i7", "#fr_t3" \n\t"

































#define PSRAB_MMI(fr_i0, fr_i1, fr_t0, fr_t1, fr_d0) "punpcklbh "#fr_t0", "#fr_t0", "#fr_i0" \n\t" "punpckhbh "#fr_t1", "#fr_t1", "#fr_i0" \n\t" "psrah "#fr_t0", "#fr_t0", "#fr_i1" \n\t" "psrah "#fr_t1", "#fr_t1", "#fr_i1" \n\t" "packsshb "#fr_d0", "#fr_t0", "#fr_t1" \n\t"













#define PSRLB_MMI(fr_i0, fr_i1, fr_t0, fr_t1, fr_d0) "punpcklbh "#fr_t0", "#fr_t0", "#fr_i0" \n\t" "punpckhbh "#fr_t1", "#fr_t1", "#fr_i0" \n\t" "psrlh "#fr_t0", "#fr_t0", "#fr_i1" \n\t" "psrlh "#fr_t1", "#fr_t1", "#fr_i1" \n\t" "packsshb "#fr_d0", "#fr_t0", "#fr_t1" \n\t"






#define PSRAH_4_MMI(fp1, fp2, fp3, fp4, shift) "psrah "#fp1", "#fp1", "#shift" \n\t" "psrah "#fp2", "#fp2", "#shift" \n\t" "psrah "#fp3", "#fp3", "#shift" \n\t" "psrah "#fp4", "#fp4", "#shift" \n\t"





#define PSRAH_8_MMI(fp1, fp2, fp3, fp4, fp5, fp6, fp7, fp8, shift) PSRAH_4_MMI(fp1, fp2, fp3, fp4, shift) PSRAH_4_MMI(fp5, fp6, fp7, fp8, shift)










#define ROUND_POWER_OF_TWO_MMI(fr_i0, fr_i1, fr_t0, fr_t1, gr_t0) "li "#gr_t0", 0x01 \n\t" "dmtc1 "#gr_t0", "#fr_t0" \n\t" "punpcklwd "#fr_t0", "#fr_t0", "#fr_t0" \n\t" "psubw "#fr_t1", "#fr_i1", "#fr_t0" \n\t" "psllw "#fr_t1", "#fr_t0", "#fr_t1" \n\t" "paddw "#fr_i0", "#fr_i0", "#fr_t1" \n\t" "psraw "#fr_i0", "#fr_i0", "#fr_i1" \n\t"








#endif 
