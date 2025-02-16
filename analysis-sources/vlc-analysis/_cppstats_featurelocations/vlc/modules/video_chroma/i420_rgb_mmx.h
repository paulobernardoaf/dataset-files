

























static const uint64_t mmx_80w = 0x0080008000800080ULL; 
static const uint64_t mmx_10w = 0x1010101010101010ULL; 
static const uint64_t mmx_00ffw = 0x00ff00ff00ff00ffULL; 
static const uint64_t mmx_Y_coeff = 0x253f253f253f253fULL; 

static const uint64_t mmx_U_green = 0xf37df37df37df37dULL; 
static const uint64_t mmx_U_blue = 0x4093409340934093ULL; 
static const uint64_t mmx_V_red = 0x3312331233123312ULL; 
static const uint64_t mmx_V_green = 0xe5fce5fce5fce5fcULL; 

static const uint64_t mmx_mask_f8 = 0xf8f8f8f8f8f8f8f8ULL; 
static const uint64_t mmx_mask_fc = 0xfcfcfcfcfcfcfcfcULL; 

#if defined(CAN_COMPILE_MMX)



#define MMX_CALL(MMX_INSTRUCTIONS) do { __asm__ __volatile__( ".p2align 3 \n\t" MMX_INSTRUCTIONS : : "r" (p_y), "r" (p_u), "r" (p_v), "r" (p_buffer), "m" (mmx_80w), "m" (mmx_10w), "m" (mmx_00ffw), "m" (mmx_Y_coeff), "m" (mmx_U_green), "m" (mmx_U_blue), "m" (mmx_V_red), "m" (mmx_V_green), "m" (mmx_mask_f8), "m" (mmx_mask_fc) : "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7" ); } while(0)















#define MMX_END __asm__ __volatile__ ( "emms" )

#define MMX_INIT_16 " \nmovd (%1), %%mm0 #Load 4 Cb 00 00 00 00 u3 u2 u1 u0 \nmovd (%2), %%mm1 #Load 4 Cr 00 00 00 00 v3 v2 v1 v0 \npxor %%mm4, %%mm4 #zero mm4 \nmovq (%0), %%mm6 #Load 8 Y Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 \n"






#define MMX_INIT_16_GRAY " \nmovq (%0), %%mm6 #Load 8 Y Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 \n#movl $0, (%3) #cache preload for image \n"




#define MMX_INIT_32 " \nmovd (%1), %%mm0 #Load 4 Cb 00 00 00 00 u3 u2 u1 u0 \nmovl $0, (%3) #cache preload for image \nmovd (%2), %%mm1 #Load 4 Cr 00 00 00 00 v3 v2 v1 v0 \npxor %%mm4, %%mm4 #zero mm4 \nmovq (%0), %%mm6 #Load 8 Y Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 \n"















#define MMX_YUV_MUL " \n#convert the chroma part \npunpcklbw %%mm4, %%mm0 #scatter 4 Cb 00 u3 00 u2 00 u1 00 u0 \npunpcklbw %%mm4, %%mm1 #scatter 4 Cr 00 v3 00 v2 00 v1 00 v0 \npsubsw %4, %%mm0 #Cb -= 128 \npsubsw %4, %%mm1 #Cr -= 128 \npsllw $3, %%mm0 #Promote precision \npsllw $3, %%mm1 #Promote precision \nmovq %%mm0, %%mm2 #Copy 4 Cb 00 u3 00 u2 00 u1 00 u0 \nmovq %%mm1, %%mm3 #Copy 4 Cr 00 v3 00 v2 00 v1 00 v0 \npmulhw %8, %%mm2 #Mul Cb with green coeff -> Cb green \npmulhw %11, %%mm3 #Mul Cr with green coeff -> Cr green \npmulhw %9, %%mm0 #Mul Cb -> Cblue 00 b3 00 b2 00 b1 00 b0 \npmulhw %10, %%mm1 #Mul Cr -> Cred 00 r3 00 r2 00 r1 00 r0 \npaddsw %%mm3, %%mm2 #Cb green + Cr green -> Cgreen \n\n#convert the luma part \npsubusb %5, %%mm6 #Y -= 16 \nmovq %%mm6, %%mm7 #Copy 8 Y Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 \npand %6, %%mm6 #get Y even 00 Y6 00 Y4 00 Y2 00 Y0 \npsrlw $8, %%mm7 #get Y odd 00 Y7 00 Y5 00 Y3 00 Y1 \npsllw $3, %%mm6 #Promote precision \npsllw $3, %%mm7 #Promote precision \npmulhw %7, %%mm6 #Mul 4 Y even 00 y6 00 y4 00 y2 00 y0 \npmulhw %7, %%mm7 #Mul 4 Y odd 00 y7 00 y5 00 y3 00 y1 \n"


































#define MMX_YUV_ADD " \n#Do horizontal and vertical scaling \nmovq %%mm0, %%mm3 #Copy Cblue \nmovq %%mm1, %%mm4 #Copy Cred \nmovq %%mm2, %%mm5 #Copy Cgreen \npaddsw %%mm6, %%mm0 #Y even + Cblue 00 B6 00 B4 00 B2 00 B0 \npaddsw %%mm7, %%mm3 #Y odd + Cblue 00 B7 00 B5 00 B3 00 B1 \npaddsw %%mm6, %%mm1 #Y even + Cred 00 R6 00 R4 00 R2 00 R0 \npaddsw %%mm7, %%mm4 #Y odd + Cred 00 R7 00 R5 00 R3 00 R1 \npaddsw %%mm6, %%mm2 #Y even + Cgreen 00 G6 00 G4 00 G2 00 G0 \npaddsw %%mm7, %%mm5 #Y odd + Cgreen 00 G7 00 G5 00 G3 00 G1 \n\n#Limit RGB even to 0..255 \npackuswb %%mm0, %%mm0 #B6 B4 B2 B0 B6 B4 B2 B0 \npackuswb %%mm1, %%mm1 #R6 R4 R2 R0 R6 R4 R2 R0 \npackuswb %%mm2, %%mm2 #G6 G4 G2 G0 G6 G4 G2 G0 \n\n#Limit RGB odd to 0..255 \npackuswb %%mm3, %%mm3 #B7 B5 B3 B1 B7 B5 B3 B1 \npackuswb %%mm4, %%mm4 #R7 R5 R3 R1 R7 R5 R3 R1 \npackuswb %%mm5, %%mm5 #G7 G5 G3 G1 G7 G5 G3 G1 \n\n#Interleave RGB even and odd \npunpcklbw %%mm3, %%mm0 #B7 B6 B5 B4 B3 B2 B1 B0 \npunpcklbw %%mm4, %%mm1 #R7 R6 R5 R4 R3 R2 R1 R0 \npunpcklbw %%mm5, %%mm2 #G7 G6 G5 G4 G3 G2 G1 G0 \n"































#define MMX_YUV_GRAY " \n#convert the luma part \npsubusb %5, %%mm6 \nmovq %%mm6, %%mm7 \npand %6, %%mm6 \npsrlw $8, %%mm7 \npsllw $3, %%mm6 \npsllw $3, %%mm7 \npmulhw %7, %%mm6 \npmulhw %7, %%mm7 \npackuswb %%mm6, %%mm6 \npackuswb %%mm7, %%mm7 \npunpcklbw %%mm7, %%mm6 \n"














#define MMX_UNPACK_16_GRAY " \nmovq %%mm6, %%mm5 \npand %12, %%mm6 \npand %13, %%mm5 \nmovq %%mm6, %%mm7 \npsrlw $3, %%mm7 \npxor %%mm3, %%mm3 \nmovq %%mm7, %%mm2 \nmovq %%mm5, %%mm0 \npunpcklbw %%mm3, %%mm5 \npunpcklbw %%mm6, %%mm7 \npsllw $3, %%mm5 \npor %%mm5, %%mm7 \nmovq %%mm7, (%3) \npunpckhbw %%mm3, %%mm0 \npunpckhbw %%mm6, %%mm2 \npsllw $3, %%mm0 \nmovq 8(%0), %%mm6 \npor %%mm0, %%mm2 \nmovq %%mm2, 8(%3) \n"





























#define MMX_UNPACK_15 " #Note, much of this shows bit patterns (of a pair of bytes) \n#mask unneeded bits off \npand %12, %%mm0 #b7b6b5b4 b3______ b7b6b5b4 b3______ \npsrlw $3,%%mm0 #______b7 b6b5b4b3 ______b7 b6b5b4b3 \npand %12, %%mm2 #g7g6g5g4 g3______ g7g6g5g4 g3______ \npand %12, %%mm1 #r7r6r5r4 r3______ r7r6r5r4 r3______ \npsrlw $1,%%mm1 #__r7r6r5 r4r3____ __r7r6r5 r4r3____ \npxor %%mm4, %%mm4 #zero mm4 \nmovq %%mm0, %%mm5 #Copy B7-B0 \nmovq %%mm2, %%mm7 #Copy G7-G0 \n\n#pack the 3 separate RGB bytes into 2 for pixels 0-3 \npunpcklbw %%mm4, %%mm2 #________ ________ g7g6g5g4 g3______ \npunpcklbw %%mm1, %%mm0 #__r7r6r5 r4r3____ ______b7 b6b5b4b3 \npsllw $2,%%mm2 #________ ____g7g6 g5g4g3__ ________ \npor %%mm2, %%mm0 #__r7r6r5 r4r3g7g6 g5g4g3b7 b6b5b4b3 \nmovq %%mm0, (%3) #store pixel 0-3 \n\n#pack the 3 separate RGB bytes into 2 for pixels 4-7 \npunpckhbw %%mm4, %%mm7 #________ ________ g7g6g5g4 g3______ \npunpckhbw %%mm1, %%mm5 #__r7r6r5 r4r3____ ______b7 b6b5b4b3 \npsllw $2,%%mm7 #________ ____g7g6 g5g4g3__ ________ \npor %%mm7, %%mm5 #__r7r6r5 r4r3g7g6 g5g4g3b7 b6b5b4b3 \nmovq %%mm5, 8(%3) #store pixel 4-7 \n"
































#define MMX_UNPACK_16 " #Note, much of this shows bit patterns (of a pair of bytes) \n#mask unneeded bits off \npand %12, %%mm0 #b7b6b5b4 b3______ b7b6b5b4 b3______ \npand %13, %%mm2 #g7g6g5g4 g3g2____ g7g6g5g4 g3g2____ \npand %12, %%mm1 #r7r6r5r4 r3______ r7r6r5r4 r3______ \npsrlw $3,%%mm0 #______b7 b6b5b4b3 ______b7 b6b5b4b3 \npxor %%mm4, %%mm4 #zero mm4 \nmovq %%mm0, %%mm5 #Copy B7-B0 \nmovq %%mm2, %%mm7 #Copy G7-G0 \n\n#pack the 3 separate RGB bytes into 2 for pixels 0-3 \npunpcklbw %%mm4, %%mm2 #________ ________ g7g6g5g4 g3g2____ \npunpcklbw %%mm1, %%mm0 #r7r6r5r4 r3______ ______b7 b6b5b4b3 \npsllw $3,%%mm2 #________ __g7g6g5 g4g3g2__ ________ \npor %%mm2, %%mm0 #r7r6r5r4 r3g7g6g5 g4g3g2b7 b6b5b4b3 \nmovq %%mm0, (%3) #store pixel 0-3 \n\n#pack the 3 separate RGB bytes into 2 for pixels 4-7 \npunpckhbw %%mm4, %%mm7 #________ ________ g7g6g5g4 g3g2____ \npunpckhbw %%mm1, %%mm5 #r7r6r5r4 r3______ ______b7 b6b5b4b3 \npsllw $3,%%mm7 #________ __g7g6g5 g4g3g2__ ________ \npor %%mm7, %%mm5 #r7r6r5r4 r3g7g6g5 g4g3g2b7 b6b5b4b3 \nmovq %%mm5, 8(%3) #store pixel 4-7 \n"





























#define MMX_UNPACK_32_ARGB " \npxor %%mm3, %%mm3 #zero mm3 \nmovq %%mm0, %%mm4 #B7 B6 B5 B4 B3 B2 B1 B0 \npunpcklbw %%mm2, %%mm4 #G3 B3 G2 B2 G1 B1 G0 B0 \nmovq %%mm1, %%mm5 #R7 R6 R5 R4 R3 R2 R1 R0 \npunpcklbw %%mm3, %%mm5 #00 R3 00 R2 00 R1 00 R0 \nmovq %%mm4, %%mm6 #G3 B3 G2 B2 G1 B1 G0 B0 \npunpcklwd %%mm5, %%mm4 #00 R1 B1 G1 00 R0 B0 G0 \nmovq %%mm4, (%3) #Store ARGB1 ARGB0 \npunpckhwd %%mm5, %%mm6 #00 R3 B3 G3 00 R2 B2 G2 \nmovq %%mm6, 8(%3) #Store ARGB3 ARGB2 \npunpckhbw %%mm2, %%mm0 #G7 B7 G6 B6 G5 B5 G4 B4 \npunpckhbw %%mm3, %%mm1 #00 R7 00 R6 00 R5 00 R4 \nmovq %%mm0, %%mm5 #G7 B7 G6 B6 G5 B5 G4 B4 \npunpcklwd %%mm1, %%mm5 #00 R5 B5 G5 00 R4 B4 G4 \nmovq %%mm5, 16(%3) #Store ARGB5 ARGB4 \npunpckhwd %%mm1, %%mm0 #00 R7 B7 G7 00 R6 B6 G6 \nmovq %%mm0, 24(%3) #Store ARGB7 ARGB6 \n"



















#define MMX_UNPACK_32_RGBA " \npxor %%mm3, %%mm3 #zero mm3 \nmovq %%mm2, %%mm4 #G7 G6 G5 G4 G3 G2 G1 G0 \npunpcklbw %%mm1, %%mm4 #R3 G3 R2 G2 R1 G1 R0 G0 \npunpcklbw %%mm0, %%mm3 #B3 00 B2 00 B1 00 B0 00 \nmovq %%mm3, %%mm5 #R3 00 R2 00 R1 00 R0 00 \npunpcklwd %%mm4, %%mm3 #R1 G1 B1 00 R0 G0 B0 00 \nmovq %%mm3, (%3) #Store RGBA1 RGBA0 \npunpckhwd %%mm4, %%mm5 #R3 G3 B3 00 R2 G2 B2 00 \nmovq %%mm5, 8(%3) #Store RGBA3 RGBA2 \npxor %%mm6, %%mm6 #zero mm6 \npunpckhbw %%mm1, %%mm2 #R7 G7 R6 G6 R5 G5 R4 G4 \npunpckhbw %%mm0, %%mm6 #B7 00 B6 00 B5 00 B4 00 \nmovq %%mm6, %%mm0 #B7 00 B6 00 B5 00 B4 00 \npunpcklwd %%mm2, %%mm6 #R5 G5 B5 00 R4 G4 B4 00 \nmovq %%mm6, 16(%3) #Store RGBA5 RGBA4 \npunpckhwd %%mm2, %%mm0 #R7 G7 B7 00 R6 G6 B6 00 \nmovq %%mm0, 24(%3) #Store RGBA7 RGBA6 \n"



















#define MMX_UNPACK_32_BGRA " \npxor %%mm3, %%mm3 #zero mm3 \nmovq %%mm2, %%mm4 #G7 G6 G5 G4 G3 G2 G1 G0 \npunpcklbw %%mm0, %%mm4 #B3 G3 B2 G2 B1 G1 B0 G0 \npunpcklbw %%mm1, %%mm3 #R3 00 R2 00 R1 00 R0 00 \nmovq %%mm3, %%mm5 #R3 00 R2 00 R1 00 R0 00 \npunpcklwd %%mm4, %%mm3 #B1 G1 R1 00 B0 G0 R0 00 \nmovq %%mm3, (%3) #Store BGRA1 BGRA0 \npunpckhwd %%mm4, %%mm5 #B3 G3 R3 00 B2 G2 R2 00 \nmovq %%mm5, 8(%3) #Store BGRA3 BGRA2 \npxor %%mm6, %%mm6 #zero mm6 \npunpckhbw %%mm0, %%mm2 #B7 G7 B6 G6 B5 G5 B4 G4 \npunpckhbw %%mm1, %%mm6 #R7 00 R6 00 R5 00 R4 00 \nmovq %%mm6, %%mm0 #R7 00 R6 00 R5 00 R4 00 \npunpcklwd %%mm2, %%mm6 #B5 G5 R5 00 B4 G4 R4 00 \nmovq %%mm6, 16(%3) #Store BGRA5 BGRA4 \npunpckhwd %%mm2, %%mm0 #B7 G7 R7 00 B6 G6 R6 00 \nmovq %%mm0, 24(%3) #Store BGRA7 BGRA6 \n"



















#define MMX_UNPACK_32_ABGR " \npxor %%mm3, %%mm3 #zero mm3 \nmovq %%mm1, %%mm4 #R7 R6 R5 R4 R3 R2 R1 R0 \npunpcklbw %%mm2, %%mm4 #G3 R3 G2 R2 G1 R1 G0 R0 \nmovq %%mm0, %%mm5 #B7 B6 B5 B4 B3 B2 B1 B0 \npunpcklbw %%mm3, %%mm5 #00 B3 00 B2 00 B1 00 B0 \nmovq %%mm4, %%mm6 #G3 R3 G2 R2 G1 R1 G0 R0 \npunpcklwd %%mm5, %%mm4 #00 B1 G1 R1 00 B0 G0 R0 \nmovq %%mm4, (%3) #Store ABGR1 ABGR0 \npunpckhwd %%mm5, %%mm6 #00 B3 G3 R3 00 B2 G2 R2 \nmovq %%mm6, 8(%3) #Store ABGR3 ABGR2 \npunpckhbw %%mm2, %%mm1 #G7 R7 G6 R6 G5 R5 G4 R4 \npunpckhbw %%mm3, %%mm0 #00 B7 00 B6 00 B5 00 B4 \nmovq %%mm1, %%mm2 #G7 R7 G6 R6 G5 R5 G4 R4 \npunpcklwd %%mm0, %%mm1 #00 B5 G5 R5 00 B4 G4 R4 \nmovq %%mm1, 16(%3) #Store ABGR5 ABGR4 \npunpckhwd %%mm0, %%mm2 #B7 G7 R7 00 B6 G6 R6 00 \nmovq %%mm2, 24(%3) #Store ABGR7 ABGR6 \n"



















#elif defined(HAVE_MMX_INTRINSICS)



#include <mmintrin.h>

#define MMX_CALL(MMX_INSTRUCTIONS) do { __m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7; MMX_INSTRUCTIONS } while(0)






#define MMX_END _mm_empty()

#define MMX_INIT_16 mm0 = _mm_cvtsi32_si64(*(int*)p_u); mm1 = _mm_cvtsi32_si64(*(int*)p_v); mm4 = _mm_setzero_si64(); mm6 = (__m64)*(uint64_t *)p_y;





#define MMX_INIT_32 mm0 = _mm_cvtsi32_si64(*(int*)p_u); *(uint16_t *)p_buffer = 0; mm1 = _mm_cvtsi32_si64(*(int*)p_v); mm4 = _mm_setzero_si64(); mm6 = (__m64)*(uint64_t *)p_y;






#define MMX_YUV_MUL mm0 = _mm_unpacklo_pi8(mm0, mm4); mm1 = _mm_unpacklo_pi8(mm1, mm4); mm0 = _mm_subs_pi16(mm0, (__m64)mmx_80w); mm1 = _mm_subs_pi16(mm1, (__m64)mmx_80w); mm0 = _mm_slli_pi16(mm0, 3); mm1 = _mm_slli_pi16(mm1, 3); mm2 = mm0; mm3 = mm1; mm2 = _mm_mulhi_pi16(mm2, (__m64)mmx_U_green); mm3 = _mm_mulhi_pi16(mm3, (__m64)mmx_V_green); mm0 = _mm_mulhi_pi16(mm0, (__m64)mmx_U_blue); mm1 = _mm_mulhi_pi16(mm1, (__m64)mmx_V_red); mm2 = _mm_adds_pi16(mm2, mm3); mm6 = _mm_subs_pu8(mm6, (__m64)mmx_10w); mm7 = mm6; mm6 = _mm_and_si64(mm6, (__m64)mmx_00ffw); mm7 = _mm_srli_pi16(mm7, 8); mm6 = _mm_slli_pi16(mm6, 3); mm7 = _mm_slli_pi16(mm7, 3); mm6 = _mm_mulhi_pi16(mm6, (__m64)mmx_Y_coeff); mm7 = _mm_mulhi_pi16(mm7, (__m64)mmx_Y_coeff);























#define MMX_YUV_ADD mm3 = mm0; mm4 = mm1; mm5 = mm2; mm0 = _mm_adds_pi16(mm0, mm6); mm3 = _mm_adds_pi16(mm3, mm7); mm1 = _mm_adds_pi16(mm1, mm6); mm4 = _mm_adds_pi16(mm4, mm7); mm2 = _mm_adds_pi16(mm2, mm6); mm5 = _mm_adds_pi16(mm5, mm7); mm0 = _mm_packs_pu16(mm0, mm0); mm1 = _mm_packs_pu16(mm1, mm1); mm2 = _mm_packs_pu16(mm2, mm2); mm3 = _mm_packs_pu16(mm3, mm3); mm4 = _mm_packs_pu16(mm4, mm4); mm5 = _mm_packs_pu16(mm5, mm5); mm0 = _mm_unpacklo_pi8(mm0, mm3); mm1 = _mm_unpacklo_pi8(mm1, mm4); mm2 = _mm_unpacklo_pi8(mm2, mm5);






















#define MMX_UNPACK_15 mm0 = _mm_and_si64(mm0, (__m64)mmx_mask_f8); mm0 = _mm_srli_pi16(mm0, 3); mm2 = _mm_and_si64(mm2, (__m64)mmx_mask_f8); mm1 = _mm_and_si64(mm1, (__m64)mmx_mask_f8); mm1 = _mm_srli_pi16(mm1, 1); mm4 = _mm_setzero_si64(); mm5 = mm0; mm7 = mm2; mm2 = _mm_unpacklo_pi8(mm2, mm4); mm0 = _mm_unpacklo_pi8(mm0, mm1); mm2 = _mm_slli_pi16(mm2, 2); mm0 = _mm_or_si64(mm0, mm2); *(uint64_t *)p_buffer = (uint64_t)mm0; mm7 = _mm_unpackhi_pi8(mm7, mm4); mm5 = _mm_unpackhi_pi8(mm5, mm1); mm7 = _mm_slli_pi16(mm7, 2); mm5 = _mm_or_si64(mm5, mm7); *(uint64_t *)(p_buffer + 4) = (uint64_t)mm5;





















#define MMX_UNPACK_16 mm0 = _mm_and_si64(mm0, (__m64)mmx_mask_f8); mm2 = _mm_and_si64(mm2, (__m64)mmx_mask_fc); mm1 = _mm_and_si64(mm1, (__m64)mmx_mask_f8); mm0 = _mm_srli_pi16(mm0, 3); mm4 = _mm_setzero_si64(); mm5 = mm0; mm7 = mm2; mm2 = _mm_unpacklo_pi8(mm2, mm4); mm0 = _mm_unpacklo_pi8(mm0, mm1); mm2 = _mm_slli_pi16(mm2, 3); mm0 = _mm_or_si64(mm0, mm2); *(uint64_t *)p_buffer = (uint64_t)mm0; mm7 = _mm_unpackhi_pi8(mm7, mm4); mm5 = _mm_unpackhi_pi8(mm5, mm1); mm7 = _mm_slli_pi16(mm7, 3); mm5 = _mm_or_si64(mm5, mm7); *(uint64_t *)(p_buffer + 4) = (uint64_t)mm5;




















#define MMX_UNPACK_32_ARGB mm3 = _mm_setzero_si64(); mm4 = mm0; mm4 = _mm_unpacklo_pi8(mm4, mm2); mm5 = mm1; mm5 = _mm_unpacklo_pi8(mm5, mm3); mm6 = mm4; mm4 = _mm_unpacklo_pi16(mm4, mm5); *(uint64_t *)p_buffer = (uint64_t)mm4; mm6 = _mm_unpackhi_pi16(mm6, mm5); *(uint64_t *)(p_buffer + 2) = (uint64_t)mm6;mm0 = _mm_unpackhi_pi8(mm0, mm2); mm1 = _mm_unpackhi_pi8(mm1, mm3); mm5 = mm0; mm5 = _mm_unpacklo_pi16(mm5, mm1); *(uint64_t *)(p_buffer + 4) = (uint64_t)mm5;mm0 = _mm_unpackhi_pi16(mm0, mm1); *(uint64_t *)(p_buffer + 6) = (uint64_t)mm0;


















#define MMX_UNPACK_32_RGBA mm3 = _mm_setzero_si64(); mm4 = mm2; mm4 = _mm_unpacklo_pi8(mm4, mm1); mm3 = _mm_unpacklo_pi8(mm3, mm0); mm5 = mm3; mm3 = _mm_unpacklo_pi16(mm3, mm4); *(uint64_t *)p_buffer = (uint64_t)mm3; mm5 = _mm_unpackhi_pi16(mm5, mm4); *(uint64_t *)(p_buffer + 2) = (uint64_t)mm5;mm6 = _mm_setzero_si64(); mm2 = _mm_unpackhi_pi8(mm2, mm1); mm6 = _mm_unpackhi_pi8(mm6, mm0); mm0 = mm6; mm6 = _mm_unpacklo_pi16(mm6, mm2); *(uint64_t *)(p_buffer + 4) = (uint64_t)mm6;mm0 = _mm_unpackhi_pi16(mm0, mm2); *(uint64_t *)(p_buffer + 6) = (uint64_t)mm0;


















#define MMX_UNPACK_32_BGRA mm3 = _mm_setzero_si64(); mm4 = mm2; mm4 = _mm_unpacklo_pi8(mm4, mm0); mm3 = _mm_unpacklo_pi8(mm3, mm1); mm5 = mm3; mm3 = _mm_unpacklo_pi16(mm3, mm4); *(uint64_t *)p_buffer = (uint64_t)mm3; mm5 = _mm_unpackhi_pi16(mm5, mm4); *(uint64_t *)(p_buffer + 2) = (uint64_t)mm5;mm6 = _mm_setzero_si64(); mm2 = _mm_unpackhi_pi8(mm2, mm0); mm6 = _mm_unpackhi_pi8(mm6, mm1); mm0 = mm6; mm6 = _mm_unpacklo_pi16(mm6, mm2); *(uint64_t *)(p_buffer + 4) = (uint64_t)mm6;mm0 = _mm_unpackhi_pi16(mm0, mm2); *(uint64_t *)(p_buffer + 6) = (uint64_t)mm0;


















#define MMX_UNPACK_32_ABGR mm3 = _mm_setzero_si64(); mm4 = mm1; mm4 = _mm_unpacklo_pi8(mm4, mm2); mm5 = mm0; mm5 = _mm_unpacklo_pi8(mm5, mm3); mm6 = mm4; mm4 = _mm_unpacklo_pi16(mm4, mm5); *(uint64_t *)p_buffer = (uint64_t)mm4; mm6 = _mm_unpackhi_pi16(mm6, mm5); *(uint64_t *)(p_buffer + 2) = (uint64_t)mm6;mm1 = _mm_unpackhi_pi8(mm1, mm2); mm0 = _mm_unpackhi_pi8(mm0, mm3); mm2 = mm1; mm1 = _mm_unpacklo_pi16(mm1, mm0); *(uint64_t *)(p_buffer + 4) = (uint64_t)mm1;mm2 = _mm_unpackhi_pi16(mm2, mm0); *(uint64_t *)(p_buffer + 6) = (uint64_t)mm2;


















#endif

