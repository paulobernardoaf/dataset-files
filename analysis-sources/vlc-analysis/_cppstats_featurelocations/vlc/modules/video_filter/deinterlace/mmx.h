


























#include <stdint.h>

typedef union {
int64_t q; 
uint64_t uq; 
int32_t d[2]; 
uint32_t ud[2]; 
int16_t w[4]; 
uint16_t uw[4]; 
int8_t b[8]; 
uint8_t ub[8]; 
float s[2]; 
} mmx_t; 


#define mmx_i2r(op,imm,reg) __asm__ __volatile__ (#op " %0, %%" #reg : : "i" (imm) : #reg)





#define mmx_m2r(op,mem,reg) __asm__ __volatile__ (#op " %0, %%" #reg : : "m" (mem) : #reg)





#define mmx_r2m(op,reg,mem) __asm__ __volatile__ (#op " %%" #reg ", %0" : "=m" (mem) : : "memory")





#define mmx_r2r(op,regs,regd) __asm__ __volatile__ (#op " %%" #regs ", %%" #regd ::: #regd)



#define emms() __asm__ __volatile__ ("emms")

#define movd_m2r(var,reg) mmx_m2r (movd, var, reg)
#define movd_r2m(reg,var) mmx_r2m (movd, reg, var)
#define movd_r2r(regs,regd) mmx_r2r (movd, regs, regd)

#define movq_m2r(var,reg) mmx_m2r (movq, var, reg)
#define movq_r2m(reg,var) mmx_r2m (movq, reg, var)
#define movq_r2r(regs,regd) mmx_r2r (movq, regs, regd)

#define packssdw_m2r(var,reg) mmx_m2r (packssdw, var, reg)
#define packssdw_r2r(regs,regd) mmx_r2r (packssdw, regs, regd)
#define packsswb_m2r(var,reg) mmx_m2r (packsswb, var, reg)
#define packsswb_r2r(regs,regd) mmx_r2r (packsswb, regs, regd)

#define packuswb_m2r(var,reg) mmx_m2r (packuswb, var, reg)
#define packuswb_r2r(regs,regd) mmx_r2r (packuswb, regs, regd)

#define paddb_m2r(var,reg) mmx_m2r (paddb, var, reg)
#define paddb_r2r(regs,regd) mmx_r2r (paddb, regs, regd)
#define paddd_m2r(var,reg) mmx_m2r (paddd, var, reg)
#define paddd_r2r(regs,regd) mmx_r2r (paddd, regs, regd)
#define paddw_m2r(var,reg) mmx_m2r (paddw, var, reg)
#define paddw_r2r(regs,regd) mmx_r2r (paddw, regs, regd)

#define paddsb_m2r(var,reg) mmx_m2r (paddsb, var, reg)
#define paddsb_r2r(regs,regd) mmx_r2r (paddsb, regs, regd)
#define paddsw_m2r(var,reg) mmx_m2r (paddsw, var, reg)
#define paddsw_r2r(regs,regd) mmx_r2r (paddsw, regs, regd)

#define paddusb_m2r(var,reg) mmx_m2r (paddusb, var, reg)
#define paddusb_r2r(regs,regd) mmx_r2r (paddusb, regs, regd)
#define paddusw_m2r(var,reg) mmx_m2r (paddusw, var, reg)
#define paddusw_r2r(regs,regd) mmx_r2r (paddusw, regs, regd)

#define pand_m2r(var,reg) mmx_m2r (pand, var, reg)
#define pand_r2r(regs,regd) mmx_r2r (pand, regs, regd)

#define pandn_m2r(var,reg) mmx_m2r (pandn, var, reg)
#define pandn_r2r(regs,regd) mmx_r2r (pandn, regs, regd)

#define pcmpeqb_m2r(var,reg) mmx_m2r (pcmpeqb, var, reg)
#define pcmpeqb_r2r(regs,regd) mmx_r2r (pcmpeqb, regs, regd)
#define pcmpeqd_m2r(var,reg) mmx_m2r (pcmpeqd, var, reg)
#define pcmpeqd_r2r(regs,regd) mmx_r2r (pcmpeqd, regs, regd)
#define pcmpeqw_m2r(var,reg) mmx_m2r (pcmpeqw, var, reg)
#define pcmpeqw_r2r(regs,regd) mmx_r2r (pcmpeqw, regs, regd)

#define pcmpgtb_m2r(var,reg) mmx_m2r (pcmpgtb, var, reg)
#define pcmpgtb_r2r(regs,regd) mmx_r2r (pcmpgtb, regs, regd)
#define pcmpgtd_m2r(var,reg) mmx_m2r (pcmpgtd, var, reg)
#define pcmpgtd_r2r(regs,regd) mmx_r2r (pcmpgtd, regs, regd)
#define pcmpgtw_m2r(var,reg) mmx_m2r (pcmpgtw, var, reg)
#define pcmpgtw_r2r(regs,regd) mmx_r2r (pcmpgtw, regs, regd)

#define pmaddwd_m2r(var,reg) mmx_m2r (pmaddwd, var, reg)
#define pmaddwd_r2r(regs,regd) mmx_r2r (pmaddwd, regs, regd)

#define pmulhw_m2r(var,reg) mmx_m2r (pmulhw, var, reg)
#define pmulhw_r2r(regs,regd) mmx_r2r (pmulhw, regs, regd)

#define pmullw_m2r(var,reg) mmx_m2r (pmullw, var, reg)
#define pmullw_r2r(regs,regd) mmx_r2r (pmullw, regs, regd)

#define por_m2r(var,reg) mmx_m2r (por, var, reg)
#define por_r2r(regs,regd) mmx_r2r (por, regs, regd)

#define pslld_i2r(imm,reg) mmx_i2r (pslld, imm, reg)
#define pslld_m2r(var,reg) mmx_m2r (pslld, var, reg)
#define pslld_r2r(regs,regd) mmx_r2r (pslld, regs, regd)
#define psllq_i2r(imm,reg) mmx_i2r (psllq, imm, reg)
#define psllq_m2r(var,reg) mmx_m2r (psllq, var, reg)
#define psllq_r2r(regs,regd) mmx_r2r (psllq, regs, regd)
#define psllw_i2r(imm,reg) mmx_i2r (psllw, imm, reg)
#define psllw_m2r(var,reg) mmx_m2r (psllw, var, reg)
#define psllw_r2r(regs,regd) mmx_r2r (psllw, regs, regd)

#define psrad_i2r(imm,reg) mmx_i2r (psrad, imm, reg)
#define psrad_m2r(var,reg) mmx_m2r (psrad, var, reg)
#define psrad_r2r(regs,regd) mmx_r2r (psrad, regs, regd)
#define psraw_i2r(imm,reg) mmx_i2r (psraw, imm, reg)
#define psraw_m2r(var,reg) mmx_m2r (psraw, var, reg)
#define psraw_r2r(regs,regd) mmx_r2r (psraw, regs, regd)

#define psrld_i2r(imm,reg) mmx_i2r (psrld, imm, reg)
#define psrld_m2r(var,reg) mmx_m2r (psrld, var, reg)
#define psrld_r2r(regs,regd) mmx_r2r (psrld, regs, regd)
#define psrlq_i2r(imm,reg) mmx_i2r (psrlq, imm, reg)
#define psrlq_m2r(var,reg) mmx_m2r (psrlq, var, reg)
#define psrlq_r2r(regs,regd) mmx_r2r (psrlq, regs, regd)
#define psrlw_i2r(imm,reg) mmx_i2r (psrlw, imm, reg)
#define psrlw_m2r(var,reg) mmx_m2r (psrlw, var, reg)
#define psrlw_r2r(regs,regd) mmx_r2r (psrlw, regs, regd)

#define psubb_m2r(var,reg) mmx_m2r (psubb, var, reg)
#define psubb_r2r(regs,regd) mmx_r2r (psubb, regs, regd)
#define psubd_m2r(var,reg) mmx_m2r (psubd, var, reg)
#define psubd_r2r(regs,regd) mmx_r2r (psubd, regs, regd)
#define psubw_m2r(var,reg) mmx_m2r (psubw, var, reg)
#define psubw_r2r(regs,regd) mmx_r2r (psubw, regs, regd)

#define psubsb_m2r(var,reg) mmx_m2r (psubsb, var, reg)
#define psubsb_r2r(regs,regd) mmx_r2r (psubsb, regs, regd)
#define psubsw_m2r(var,reg) mmx_m2r (psubsw, var, reg)
#define psubsw_r2r(regs,regd) mmx_r2r (psubsw, regs, regd)

#define psubusb_m2r(var,reg) mmx_m2r (psubusb, var, reg)
#define psubusb_r2r(regs,regd) mmx_r2r (psubusb, regs, regd)
#define psubusw_m2r(var,reg) mmx_m2r (psubusw, var, reg)
#define psubusw_r2r(regs,regd) mmx_r2r (psubusw, regs, regd)

#define punpckhbw_m2r(var,reg) mmx_m2r (punpckhbw, var, reg)
#define punpckhbw_r2r(regs,regd) mmx_r2r (punpckhbw, regs, regd)
#define punpckhdq_m2r(var,reg) mmx_m2r (punpckhdq, var, reg)
#define punpckhdq_r2r(regs,regd) mmx_r2r (punpckhdq, regs, regd)
#define punpckhwd_m2r(var,reg) mmx_m2r (punpckhwd, var, reg)
#define punpckhwd_r2r(regs,regd) mmx_r2r (punpckhwd, regs, regd)

#define punpcklbw_m2r(var,reg) mmx_m2r (punpcklbw, var, reg)
#define punpcklbw_r2r(regs,regd) mmx_r2r (punpcklbw, regs, regd)
#define punpckldq_m2r(var,reg) mmx_m2r (punpckldq, var, reg)
#define punpckldq_r2r(regs,regd) mmx_r2r (punpckldq, regs, regd)
#define punpcklwd_m2r(var,reg) mmx_m2r (punpcklwd, var, reg)
#define punpcklwd_r2r(regs,regd) mmx_r2r (punpcklwd, regs, regd)

#define pxor_m2r(var,reg) mmx_m2r (pxor, var, reg)
#define pxor_r2r(regs,regd) mmx_r2r (pxor, regs, regd)




#define pavgusb_m2r(var,reg) mmx_m2r (pavgusb, var, reg)
#define pavgusb_r2r(regs,regd) mmx_r2r (pavgusb, regs, regd)





#define mmx_m2ri(op,mem,reg,imm) __asm__ __volatile__ (#op " %1, %0, %%" #reg : : "X" (mem), "X" (imm) : #reg)




#define mmx_r2ri(op,regs,regd,imm) __asm__ __volatile__ (#op " %0, %%" #regs ", %%" #regd : : "X" (imm) : #regd)





#define mmx_fetch(mem,hint) __asm__ __volatile__ ("prefetch" #hint " %0" : : "X" (mem))





#define maskmovq(regs,maskreg) mmx_r2ri (maskmovq, regs, maskreg)

#define movntq_r2m(mmreg,var) mmx_r2m (movntq, mmreg, var)

#define pavgb_m2r(var,reg) mmx_m2r (pavgb, var, reg)
#define pavgb_r2r(regs,regd) mmx_r2r (pavgb, regs, regd)
#define pavgw_m2r(var,reg) mmx_m2r (pavgw, var, reg)
#define pavgw_r2r(regs,regd) mmx_r2r (pavgw, regs, regd)

#define pextrw_r2r(mmreg,reg,imm) mmx_r2ri (pextrw, mmreg, reg, imm)

#define pinsrw_r2r(reg,mmreg,imm) mmx_r2ri (pinsrw, reg, mmreg, imm)

#define pmaxsw_m2r(var,reg) mmx_m2r (pmaxsw, var, reg)
#define pmaxsw_r2r(regs,regd) mmx_r2r (pmaxsw, regs, regd)

#define pmaxub_m2r(var,reg) mmx_m2r (pmaxub, var, reg)
#define pmaxub_r2r(regs,regd) mmx_r2r (pmaxub, regs, regd)

#define pminsw_m2r(var,reg) mmx_m2r (pminsw, var, reg)
#define pminsw_r2r(regs,regd) mmx_r2r (pminsw, regs, regd)

#define pminub_m2r(var,reg) mmx_m2r (pminub, var, reg)
#define pminub_r2r(regs,regd) mmx_r2r (pminub, regs, regd)

#define pmovmskb(mmreg,reg) __asm__ __volatile__ ("movmskps %" #mmreg ", %" #reg : : : #reg)


#define pmulhuw_m2r(var,reg) mmx_m2r (pmulhuw, var, reg)
#define pmulhuw_r2r(regs,regd) mmx_r2r (pmulhuw, regs, regd)

#define prefetcht0(mem) mmx_fetch (mem, t0)
#define prefetcht1(mem) mmx_fetch (mem, t1)
#define prefetcht2(mem) mmx_fetch (mem, t2)
#define prefetchnta(mem) mmx_fetch (mem, nta)

#define psadbw_m2r(var,reg) mmx_m2r (psadbw, var, reg)
#define psadbw_r2r(regs,regd) mmx_r2r (psadbw, regs, regd)

#define pshufw_m2r(var,reg,imm) mmx_m2ri(pshufw, var, reg, imm)
#define pshufw_r2r(regs,regd,imm) mmx_r2ri(pshufw, regs, regd, imm)

#define sfence() __asm__ __volatile__ ("sfence\n\t")
