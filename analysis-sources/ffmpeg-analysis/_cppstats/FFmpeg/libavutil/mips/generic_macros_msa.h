#include <stdint.h>
#include <msa.h>
#include <config.h>
#if HAVE_MSA2
#include <msa2.h>
#endif
#define ALIGNMENT 16
#define ALLOC_ALIGNED(align) __attribute__ ((aligned((align) << 1)))
#define LD_V(RTYPE, psrc) *((RTYPE *)(psrc))
#define LD_UB(...) LD_V(v16u8, __VA_ARGS__)
#define LD_SB(...) LD_V(v16i8, __VA_ARGS__)
#define LD_UH(...) LD_V(v8u16, __VA_ARGS__)
#define LD_SH(...) LD_V(v8i16, __VA_ARGS__)
#define LD_UW(...) LD_V(v4u32, __VA_ARGS__)
#define LD_SW(...) LD_V(v4i32, __VA_ARGS__)
#define ST_V(RTYPE, in, pdst) *((RTYPE *)(pdst)) = (in)
#define ST_UB(...) ST_V(v16u8, __VA_ARGS__)
#define ST_SB(...) ST_V(v16i8, __VA_ARGS__)
#define ST_UH(...) ST_V(v8u16, __VA_ARGS__)
#define ST_SH(...) ST_V(v8i16, __VA_ARGS__)
#define ST_UW(...) ST_V(v4u32, __VA_ARGS__)
#define ST_SW(...) ST_V(v4i32, __VA_ARGS__)
#if (__mips_isa_rev >= 6)
#define LH(psrc) ( { uint16_t val_lh_m = *(uint16_t *)(psrc); val_lh_m; } )
#define LW(psrc) ( { uint32_t val_lw_m = *(uint32_t *)(psrc); val_lw_m; } )
#if (__mips == 64)
#define LD(psrc) ( { uint64_t val_ld_m = *(uint64_t *)(psrc); val_ld_m; } )
#else 
#define LD(psrc) ( { uint8_t *psrc_ld_m = (uint8_t *) (psrc); uint32_t val0_ld_m, val1_ld_m; uint64_t val_ld_m = 0; val0_ld_m = LW(psrc_ld_m); val1_ld_m = LW(psrc_ld_m + 4); val_ld_m = (uint64_t) (val1_ld_m); val_ld_m = (uint64_t) ((val_ld_m << 32) & 0xFFFFFFFF00000000); val_ld_m = (uint64_t) (val_ld_m | (uint64_t) val0_ld_m); val_ld_m; } )
#endif 
#define SH(val, pdst) *(uint16_t *)(pdst) = (val);
#define SW(val, pdst) *(uint32_t *)(pdst) = (val);
#define SD(val, pdst) *(uint64_t *)(pdst) = (val);
#else 
#define LH(psrc) ( { uint8_t *psrc_lh_m = (uint8_t *) (psrc); uint16_t val_lh_m; __asm__ volatile ( "ulh %[val_lh_m], %[psrc_lh_m] \n\t" : [val_lh_m] "=r" (val_lh_m) : [psrc_lh_m] "m" (*psrc_lh_m) ); val_lh_m; } )
#define LW(psrc) ( { uint8_t *psrc_lw_m = (uint8_t *) (psrc); uint32_t val_lw_m; __asm__ volatile ( "ulw %[val_lw_m], %[psrc_lw_m] \n\t" : [val_lw_m] "=r" (val_lw_m) : [psrc_lw_m] "m" (*psrc_lw_m) ); val_lw_m; } )
#if (__mips == 64)
#define LD(psrc) ( { uint8_t *psrc_ld_m = (uint8_t *) (psrc); uint64_t val_ld_m = 0; __asm__ volatile ( "uld %[val_ld_m], %[psrc_ld_m] \n\t" : [val_ld_m] "=r" (val_ld_m) : [psrc_ld_m] "m" (*psrc_ld_m) ); val_ld_m; } )
#else 
#define LD(psrc) ( { uint8_t *psrc_ld_m = (uint8_t *) (psrc); uint32_t val0_ld_m, val1_ld_m; uint64_t val_ld_m = 0; val0_ld_m = LW(psrc_ld_m); val1_ld_m = LW(psrc_ld_m + 4); val_ld_m = (uint64_t) (val1_ld_m); val_ld_m = (uint64_t) ((val_ld_m << 32) & 0xFFFFFFFF00000000); val_ld_m = (uint64_t) (val_ld_m | (uint64_t) val0_ld_m); val_ld_m; } )
#endif 
#define SH(val, pdst) { uint8_t *pdst_sh_m = (uint8_t *) (pdst); uint16_t val_sh_m = (val); __asm__ volatile ( "ush %[val_sh_m], %[pdst_sh_m] \n\t" : [pdst_sh_m] "=m" (*pdst_sh_m) : [val_sh_m] "r" (val_sh_m) ); }
#define SW(val, pdst) { uint8_t *pdst_sw_m = (uint8_t *) (pdst); uint32_t val_sw_m = (val); __asm__ volatile ( "usw %[val_sw_m], %[pdst_sw_m] \n\t" : [pdst_sw_m] "=m" (*pdst_sw_m) : [val_sw_m] "r" (val_sw_m) ); }
#define SD(val, pdst) { uint8_t *pdst_sd_m = (uint8_t *) (pdst); uint32_t val0_sd_m, val1_sd_m; val0_sd_m = (uint32_t) ((val) & 0x00000000FFFFFFFF); val1_sd_m = (uint32_t) (((val) >> 32) & 0x00000000FFFFFFFF); SW(val0_sd_m, pdst_sd_m); SW(val1_sd_m, pdst_sd_m + 4); }
#endif 
#define LW4(psrc, stride, out0, out1, out2, out3) { out0 = LW((psrc)); out1 = LW((psrc) + stride); out2 = LW((psrc) + 2 * stride); out3 = LW((psrc) + 3 * stride); }
#define LW2(psrc, stride, out0, out1) { out0 = LW((psrc)); out1 = LW((psrc) + stride); }
#define LD2(psrc, stride, out0, out1) { out0 = LD((psrc)); out1 = LD((psrc) + stride); }
#define LD4(psrc, stride, out0, out1, out2, out3) { LD2((psrc), stride, out0, out1); LD2((psrc) + 2 * stride, stride, out2, out3); }
#define SW4(in0, in1, in2, in3, pdst, stride) { SW(in0, (pdst)) SW(in1, (pdst) + stride); SW(in2, (pdst) + 2 * stride); SW(in3, (pdst) + 3 * stride); }
#define SD4(in0, in1, in2, in3, pdst, stride) { SD(in0, (pdst)) SD(in1, (pdst) + stride); SD(in2, (pdst) + 2 * stride); SD(in3, (pdst) + 3 * stride); }
#define LD_V2(RTYPE, psrc, stride, out0, out1) { out0 = LD_V(RTYPE, (psrc)); out1 = LD_V(RTYPE, (psrc) + stride); }
#define LD_UB2(...) LD_V2(v16u8, __VA_ARGS__)
#define LD_SB2(...) LD_V2(v16i8, __VA_ARGS__)
#define LD_UH2(...) LD_V2(v8u16, __VA_ARGS__)
#define LD_SH2(...) LD_V2(v8i16, __VA_ARGS__)
#define LD_SW2(...) LD_V2(v4i32, __VA_ARGS__)
#define LD_V3(RTYPE, psrc, stride, out0, out1, out2) { LD_V2(RTYPE, (psrc), stride, out0, out1); out2 = LD_V(RTYPE, (psrc) + 2 * stride); }
#define LD_UB3(...) LD_V3(v16u8, __VA_ARGS__)
#define LD_SB3(...) LD_V3(v16i8, __VA_ARGS__)
#define LD_V4(RTYPE, psrc, stride, out0, out1, out2, out3) { LD_V2(RTYPE, (psrc), stride, out0, out1); LD_V2(RTYPE, (psrc) + 2 * stride , stride, out2, out3); }
#define LD_UB4(...) LD_V4(v16u8, __VA_ARGS__)
#define LD_SB4(...) LD_V4(v16i8, __VA_ARGS__)
#define LD_UH4(...) LD_V4(v8u16, __VA_ARGS__)
#define LD_SH4(...) LD_V4(v8i16, __VA_ARGS__)
#define LD_SW4(...) LD_V4(v4i32, __VA_ARGS__)
#define LD_V5(RTYPE, psrc, stride, out0, out1, out2, out3, out4) { LD_V4(RTYPE, (psrc), stride, out0, out1, out2, out3); out4 = LD_V(RTYPE, (psrc) + 4 * stride); }
#define LD_UB5(...) LD_V5(v16u8, __VA_ARGS__)
#define LD_SB5(...) LD_V5(v16i8, __VA_ARGS__)
#define LD_V6(RTYPE, psrc, stride, out0, out1, out2, out3, out4, out5) { LD_V4(RTYPE, (psrc), stride, out0, out1, out2, out3); LD_V2(RTYPE, (psrc) + 4 * stride, stride, out4, out5); }
#define LD_UB6(...) LD_V6(v16u8, __VA_ARGS__)
#define LD_SB6(...) LD_V6(v16i8, __VA_ARGS__)
#define LD_UH6(...) LD_V6(v8u16, __VA_ARGS__)
#define LD_SH6(...) LD_V6(v8i16, __VA_ARGS__)
#define LD_V7(RTYPE, psrc, stride, out0, out1, out2, out3, out4, out5, out6) { LD_V5(RTYPE, (psrc), stride, out0, out1, out2, out3, out4); LD_V2(RTYPE, (psrc) + 5 * stride, stride, out5, out6); }
#define LD_UB7(...) LD_V7(v16u8, __VA_ARGS__)
#define LD_SB7(...) LD_V7(v16i8, __VA_ARGS__)
#define LD_V8(RTYPE, psrc, stride, out0, out1, out2, out3, out4, out5, out6, out7) { LD_V4(RTYPE, (psrc), stride, out0, out1, out2, out3); LD_V4(RTYPE, (psrc) + 4 * stride, stride, out4, out5, out6, out7); }
#define LD_UB8(...) LD_V8(v16u8, __VA_ARGS__)
#define LD_SB8(...) LD_V8(v16i8, __VA_ARGS__)
#define LD_UH8(...) LD_V8(v8u16, __VA_ARGS__)
#define LD_SH8(...) LD_V8(v8i16, __VA_ARGS__)
#define LD_SW8(...) LD_V8(v4i32, __VA_ARGS__)
#define LD_V16(RTYPE, psrc, stride, out0, out1, out2, out3, out4, out5, out6, out7, out8, out9, out10, out11, out12, out13, out14, out15) { LD_V8(RTYPE, (psrc), stride, out0, out1, out2, out3, out4, out5, out6, out7); LD_V8(RTYPE, (psrc) + 8 * stride, stride, out8, out9, out10, out11, out12, out13, out14, out15); }
#define LD_SH16(...) LD_V16(v8i16, __VA_ARGS__)
#define ST_V2(RTYPE, in0, in1, pdst, stride) { ST_V(RTYPE, in0, (pdst)); ST_V(RTYPE, in1, (pdst) + stride); }
#define ST_UB2(...) ST_V2(v16u8, __VA_ARGS__)
#define ST_SB2(...) ST_V2(v16i8, __VA_ARGS__)
#define ST_UH2(...) ST_V2(v8u16, __VA_ARGS__)
#define ST_SH2(...) ST_V2(v8i16, __VA_ARGS__)
#define ST_SW2(...) ST_V2(v4i32, __VA_ARGS__)
#define ST_V4(RTYPE, in0, in1, in2, in3, pdst, stride) { ST_V2(RTYPE, in0, in1, (pdst), stride); ST_V2(RTYPE, in2, in3, (pdst) + 2 * stride, stride); }
#define ST_UB4(...) ST_V4(v16u8, __VA_ARGS__)
#define ST_SB4(...) ST_V4(v16i8, __VA_ARGS__)
#define ST_SH4(...) ST_V4(v8i16, __VA_ARGS__)
#define ST_SW4(...) ST_V4(v4i32, __VA_ARGS__)
#define ST_V6(RTYPE, in0, in1, in2, in3, in4, in5, pdst, stride) { ST_V4(RTYPE, in0, in1, in2, in3, (pdst), stride); ST_V2(RTYPE, in4, in5, (pdst) + 4 * stride, stride); }
#define ST_SH6(...) ST_V6(v8i16, __VA_ARGS__)
#define ST_V8(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { ST_V4(RTYPE, in0, in1, in2, in3, (pdst), stride); ST_V4(RTYPE, in4, in5, in6, in7, (pdst) + 4 * stride, stride); }
#define ST_UB8(...) ST_V8(v16u8, __VA_ARGS__)
#define ST_SH8(...) ST_V8(v8i16, __VA_ARGS__)
#define ST_SW8(...) ST_V8(v4i32, __VA_ARGS__)
#define ST_H1(in, idx, pdst) { uint16_t out0_m; out0_m = __msa_copy_u_h((v8i16) in, idx); SH(out0_m, (pdst)); }
#define ST_H2(in, idx0, idx1, pdst, stride) { uint16_t out0_m, out1_m; out0_m = __msa_copy_u_h((v8i16) in, idx0); out1_m = __msa_copy_u_h((v8i16) in, idx1); SH(out0_m, (pdst)); SH(out1_m, (pdst) + stride); }
#define ST_H4(in, idx0, idx1, idx2, idx3, pdst, stride) { uint16_t out0_m, out1_m, out2_m, out3_m; out0_m = __msa_copy_u_h((v8i16) in, idx0); out1_m = __msa_copy_u_h((v8i16) in, idx1); out2_m = __msa_copy_u_h((v8i16) in, idx2); out3_m = __msa_copy_u_h((v8i16) in, idx3); SH(out0_m, (pdst)); SH(out1_m, (pdst) + stride); SH(out2_m, (pdst) + 2 * stride); SH(out3_m, (pdst) + 3 * stride); }
#define ST_H8(in, idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7, pdst, stride) { ST_H4(in, idx0, idx1, idx2, idx3, pdst, stride) ST_H4(in, idx4, idx5, idx6, idx7, (pdst) + 4*stride, stride) }
#define ST_W1(in, idx, pdst) { uint32_t out0_m; out0_m = __msa_copy_u_w((v4i32) in, idx); SW(out0_m, (pdst)); }
#define ST_W2(in, idx0, idx1, pdst, stride) { uint32_t out0_m, out1_m; out0_m = __msa_copy_u_w((v4i32) in, idx0); out1_m = __msa_copy_u_w((v4i32) in, idx1); SW(out0_m, (pdst)); SW(out1_m, (pdst) + stride); }
#define ST_W4(in, idx0, idx1, idx2, idx3, pdst, stride) { uint32_t out0_m, out1_m, out2_m, out3_m; out0_m = __msa_copy_u_w((v4i32) in, idx0); out1_m = __msa_copy_u_w((v4i32) in, idx1); out2_m = __msa_copy_u_w((v4i32) in, idx2); out3_m = __msa_copy_u_w((v4i32) in, idx3); SW(out0_m, (pdst)); SW(out1_m, (pdst) + stride); SW(out2_m, (pdst) + 2*stride); SW(out3_m, (pdst) + 3*stride); }
#define ST_W8(in0, in1, idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7, pdst, stride) { ST_W4(in0, idx0, idx1, idx2, idx3, pdst, stride) ST_W4(in1, idx4, idx5, idx6, idx7, pdst + 4*stride, stride) }
#define ST_D1(in, idx, pdst) { uint64_t out0_m; out0_m = __msa_copy_u_d((v2i64) in, idx); SD(out0_m, (pdst)); }
#define ST_D2(in, idx0, idx1, pdst, stride) { uint64_t out0_m, out1_m; out0_m = __msa_copy_u_d((v2i64) in, idx0); out1_m = __msa_copy_u_d((v2i64) in, idx1); SD(out0_m, (pdst)); SD(out1_m, (pdst) + stride); }
#define ST_D4(in0, in1, idx0, idx1, idx2, idx3, pdst, stride) { uint64_t out0_m, out1_m, out2_m, out3_m; out0_m = __msa_copy_u_d((v2i64) in0, idx0); out1_m = __msa_copy_u_d((v2i64) in0, idx1); out2_m = __msa_copy_u_d((v2i64) in1, idx2); out3_m = __msa_copy_u_d((v2i64) in1, idx3); SD(out0_m, (pdst)); SD(out1_m, (pdst) + stride); SD(out2_m, (pdst) + 2 * stride); SD(out3_m, (pdst) + 3 * stride); }
#define ST_D8(in0, in1, in2, in3, idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7, pdst, stride) { ST_D4(in0, in1, idx0, idx1, idx2, idx3, pdst, stride) ST_D4(in2, in3, idx4, idx5, idx6, idx7, pdst + 4 * stride, stride) }
#define ST12x8_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { uint64_t out0_m, out1_m, out2_m, out3_m; uint64_t out4_m, out5_m, out6_m, out7_m; uint32_t out8_m, out9_m, out10_m, out11_m; uint32_t out12_m, out13_m, out14_m, out15_m; uint8_t *pblk_12x8_m = (uint8_t *) (pdst); out0_m = __msa_copy_u_d((v2i64) in0, 0); out1_m = __msa_copy_u_d((v2i64) in1, 0); out2_m = __msa_copy_u_d((v2i64) in2, 0); out3_m = __msa_copy_u_d((v2i64) in3, 0); out4_m = __msa_copy_u_d((v2i64) in4, 0); out5_m = __msa_copy_u_d((v2i64) in5, 0); out6_m = __msa_copy_u_d((v2i64) in6, 0); out7_m = __msa_copy_u_d((v2i64) in7, 0); out8_m = __msa_copy_u_w((v4i32) in0, 2); out9_m = __msa_copy_u_w((v4i32) in1, 2); out10_m = __msa_copy_u_w((v4i32) in2, 2); out11_m = __msa_copy_u_w((v4i32) in3, 2); out12_m = __msa_copy_u_w((v4i32) in4, 2); out13_m = __msa_copy_u_w((v4i32) in5, 2); out14_m = __msa_copy_u_w((v4i32) in6, 2); out15_m = __msa_copy_u_w((v4i32) in7, 2); SD(out0_m, pblk_12x8_m); SW(out8_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out1_m, pblk_12x8_m); SW(out9_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out2_m, pblk_12x8_m); SW(out10_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out3_m, pblk_12x8_m); SW(out11_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out4_m, pblk_12x8_m); SW(out12_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out5_m, pblk_12x8_m); SW(out13_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out6_m, pblk_12x8_m); SW(out14_m, pblk_12x8_m + 8); pblk_12x8_m += stride; SD(out7_m, pblk_12x8_m); SW(out15_m, pblk_12x8_m + 8); }
#define AVER_UB2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_aver_u_b((v16u8) in0, (v16u8) in1); out1 = (RTYPE) __msa_aver_u_b((v16u8) in2, (v16u8) in3); }
#define AVER_UB2_UB(...) AVER_UB2(v16u8, __VA_ARGS__)
#define AVER_UB4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { AVER_UB2(RTYPE, in0, in1, in2, in3, out0, out1) AVER_UB2(RTYPE, in4, in5, in6, in7, out2, out3) }
#define AVER_UB4_UB(...) AVER_UB4(v16u8, __VA_ARGS__)
#define SLDI_B(RTYPE, d, s, slide_val, out) { out = (RTYPE) __msa_sldi_b((v16i8) d, (v16i8) s, slide_val); }
#define SLDI_B2(RTYPE, d0, s0, d1, s1, slide_val, out0, out1) { SLDI_B(RTYPE, d0, s0, slide_val, out0) SLDI_B(RTYPE, d1, s1, slide_val, out1) }
#define SLDI_B2_UB(...) SLDI_B2(v16u8, __VA_ARGS__)
#define SLDI_B2_SB(...) SLDI_B2(v16i8, __VA_ARGS__)
#define SLDI_B2_SH(...) SLDI_B2(v8i16, __VA_ARGS__)
#define SLDI_B2_SW(...) SLDI_B2(v4i32, __VA_ARGS__)
#define SLDI_B3(RTYPE, d0, s0, d1, s1, d2, s2, slide_val, out0, out1, out2) { SLDI_B2(RTYPE, d0, s0, d1, s1, slide_val, out0, out1) SLDI_B(RTYPE, d2, s2, slide_val, out2) }
#define SLDI_B3_UB(...) SLDI_B3(v16u8, __VA_ARGS__)
#define SLDI_B3_SB(...) SLDI_B3(v16i8, __VA_ARGS__)
#define SLDI_B3_UH(...) SLDI_B3(v8u16, __VA_ARGS__)
#define SLDI_B4(RTYPE, d0, s0, d1, s1, d2, s2, d3, s3, slide_val, out0, out1, out2, out3) { SLDI_B2(RTYPE, d0, s0, d1, s1, slide_val, out0, out1) SLDI_B2(RTYPE, d2, s2, d3, s3, slide_val, out2, out3) }
#define SLDI_B4_UB(...) SLDI_B4(v16u8, __VA_ARGS__)
#define SLDI_B4_SB(...) SLDI_B4(v16i8, __VA_ARGS__)
#define SLDI_B4_SH(...) SLDI_B4(v8i16, __VA_ARGS__)
#define VSHF_B2(RTYPE, in0, in1, in2, in3, mask0, mask1, out0, out1) { out0 = (RTYPE) __msa_vshf_b((v16i8) mask0, (v16i8) in1, (v16i8) in0); out1 = (RTYPE) __msa_vshf_b((v16i8) mask1, (v16i8) in3, (v16i8) in2); }
#define VSHF_B2_UB(...) VSHF_B2(v16u8, __VA_ARGS__)
#define VSHF_B2_SB(...) VSHF_B2(v16i8, __VA_ARGS__)
#define VSHF_B2_UH(...) VSHF_B2(v8u16, __VA_ARGS__)
#define VSHF_B2_SH(...) VSHF_B2(v8i16, __VA_ARGS__)
#define VSHF_B3(RTYPE, in0, in1, in2, in3, in4, in5, mask0, mask1, mask2, out0, out1, out2) { VSHF_B2(RTYPE, in0, in1, in2, in3, mask0, mask1, out0, out1); out2 = (RTYPE) __msa_vshf_b((v16i8) mask2, (v16i8) in5, (v16i8) in4); }
#define VSHF_B3_SB(...) VSHF_B3(v16i8, __VA_ARGS__)
#define VSHF_B4(RTYPE, in0, in1, mask0, mask1, mask2, mask3, out0, out1, out2, out3) { VSHF_B2(RTYPE, in0, in1, in0, in1, mask0, mask1, out0, out1); VSHF_B2(RTYPE, in0, in1, in0, in1, mask2, mask3, out2, out3); }
#define VSHF_B4_SB(...) VSHF_B4(v16i8, __VA_ARGS__)
#define VSHF_B4_SH(...) VSHF_B4(v8i16, __VA_ARGS__)
#define VSHF_H2(RTYPE, in0, in1, in2, in3, mask0, mask1, out0, out1) { out0 = (RTYPE) __msa_vshf_h((v8i16) mask0, (v8i16) in1, (v8i16) in0); out1 = (RTYPE) __msa_vshf_h((v8i16) mask1, (v8i16) in3, (v8i16) in2); }
#define VSHF_H2_SH(...) VSHF_H2(v8i16, __VA_ARGS__)
#define VSHF_H3(RTYPE, in0, in1, in2, in3, in4, in5, mask0, mask1, mask2, out0, out1, out2) { VSHF_H2(RTYPE, in0, in1, in2, in3, mask0, mask1, out0, out1); out2 = (RTYPE) __msa_vshf_h((v8i16) mask2, (v8i16) in5, (v8i16) in4); }
#define VSHF_H3_SH(...) VSHF_H3(v8i16, __VA_ARGS__)
#define VSHF_W2(RTYPE, in0, in1, in2, in3, mask0, mask1, out0, out1) { out0 = (RTYPE) __msa_vshf_w((v4i32) mask0, (v4i32) in1, (v4i32) in0); out1 = (RTYPE) __msa_vshf_w((v4i32) mask1, (v4i32) in3, (v4i32) in2); }
#define VSHF_W2_SB(...) VSHF_W2(v16i8, __VA_ARGS__)
#define DOTP_UB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1) { out0 = (RTYPE) __msa_dotp_u_h((v16u8) mult0, (v16u8) cnst0); out1 = (RTYPE) __msa_dotp_u_h((v16u8) mult1, (v16u8) cnst1); }
#define DOTP_UB2_UH(...) DOTP_UB2(v8u16, __VA_ARGS__)
#define DOTP_UB4(RTYPE, mult0, mult1, mult2, mult3, cnst0, cnst1, cnst2, cnst3, out0, out1, out2, out3) { DOTP_UB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1); DOTP_UB2(RTYPE, mult2, mult3, cnst2, cnst3, out2, out3); }
#define DOTP_UB4_UH(...) DOTP_UB4(v8u16, __VA_ARGS__)
#define DOTP_SB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1) { out0 = (RTYPE) __msa_dotp_s_h((v16i8) mult0, (v16i8) cnst0); out1 = (RTYPE) __msa_dotp_s_h((v16i8) mult1, (v16i8) cnst1); }
#define DOTP_SB2_SH(...) DOTP_SB2(v8i16, __VA_ARGS__)
#define DOTP_SB3(RTYPE, mult0, mult1, mult2, cnst0, cnst1, cnst2, out0, out1, out2) { DOTP_SB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1); out2 = (RTYPE) __msa_dotp_s_h((v16i8) mult2, (v16i8) cnst2); }
#define DOTP_SB3_SH(...) DOTP_SB3(v8i16, __VA_ARGS__)
#define DOTP_SB4(RTYPE, mult0, mult1, mult2, mult3, cnst0, cnst1, cnst2, cnst3, out0, out1, out2, out3) { DOTP_SB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1); DOTP_SB2(RTYPE, mult2, mult3, cnst2, cnst3, out2, out3); }
#define DOTP_SB4_SH(...) DOTP_SB4(v8i16, __VA_ARGS__)
#define DOTP_SH2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1) { out0 = (RTYPE) __msa_dotp_s_w((v8i16) mult0, (v8i16) cnst0); out1 = (RTYPE) __msa_dotp_s_w((v8i16) mult1, (v8i16) cnst1); }
#define DOTP_SH2_SW(...) DOTP_SH2(v4i32, __VA_ARGS__)
#define DOTP_SH4(RTYPE, mult0, mult1, mult2, mult3, cnst0, cnst1, cnst2, cnst3, out0, out1, out2, out3) { DOTP_SH2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1); DOTP_SH2(RTYPE, mult2, mult3, cnst2, cnst3, out2, out3); }
#define DOTP_SH4_SW(...) DOTP_SH4(v4i32, __VA_ARGS__)
#define DPADD_SB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1) { out0 = (RTYPE) __msa_dpadd_s_h((v8i16) out0, (v16i8) mult0, (v16i8) cnst0); out1 = (RTYPE) __msa_dpadd_s_h((v8i16) out1, (v16i8) mult1, (v16i8) cnst1); }
#define DPADD_SB2_SH(...) DPADD_SB2(v8i16, __VA_ARGS__)
#define DPADD_SB4(RTYPE, mult0, mult1, mult2, mult3, cnst0, cnst1, cnst2, cnst3, out0, out1, out2, out3) { DPADD_SB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1); DPADD_SB2(RTYPE, mult2, mult3, cnst2, cnst3, out2, out3); }
#define DPADD_SB4_SH(...) DPADD_SB4(v8i16, __VA_ARGS__)
#define DPADD_UB2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1) { out0 = (RTYPE) __msa_dpadd_u_h((v8u16) out0, (v16u8) mult0, (v16u8) cnst0); out1 = (RTYPE) __msa_dpadd_u_h((v8u16) out1, (v16u8) mult1, (v16u8) cnst1); }
#define DPADD_UB2_UH(...) DPADD_UB2(v8u16, __VA_ARGS__)
#define DPADD_SH2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1) { out0 = (RTYPE) __msa_dpadd_s_w((v4i32) out0, (v8i16) mult0, (v8i16) cnst0); out1 = (RTYPE) __msa_dpadd_s_w((v4i32) out1, (v8i16) mult1, (v8i16) cnst1); }
#define DPADD_SH2_SW(...) DPADD_SH2(v4i32, __VA_ARGS__)
#define DPADD_SH4(RTYPE, mult0, mult1, mult2, mult3, cnst0, cnst1, cnst2, cnst3, out0, out1, out2, out3) { DPADD_SH2(RTYPE, mult0, mult1, cnst0, cnst1, out0, out1); DPADD_SH2(RTYPE, mult2, mult3, cnst2, cnst3, out2, out3); }
#define DPADD_SH4_SW(...) DPADD_SH4(v4i32, __VA_ARGS__)
#define MIN_UH2(RTYPE, in0, in1, min_vec) { in0 = (RTYPE) __msa_min_u_h((v8u16) in0, min_vec); in1 = (RTYPE) __msa_min_u_h((v8u16) in1, min_vec); }
#define MIN_UH2_UH(...) MIN_UH2(v8u16, __VA_ARGS__)
#define MIN_UH4(RTYPE, in0, in1, in2, in3, min_vec) { MIN_UH2(RTYPE, in0, in1, min_vec); MIN_UH2(RTYPE, in2, in3, min_vec); }
#define MIN_UH4_UH(...) MIN_UH4(v8u16, __VA_ARGS__)
#define CLIP_SH(in, min, max) { in = __msa_max_s_h((v8i16) min, (v8i16) in); in = __msa_min_s_h((v8i16) max, (v8i16) in); }
#define CLIP_SH_0_255(in) { in = __msa_maxi_s_h((v8i16) in, 0); in = (v8i16) __msa_sat_u_h((v8u16) in, 7); }
#define CLIP_SH2_0_255(in0, in1) { CLIP_SH_0_255(in0); CLIP_SH_0_255(in1); }
#define CLIP_SH4_0_255(in0, in1, in2, in3) { CLIP_SH2_0_255(in0, in1); CLIP_SH2_0_255(in2, in3); }
#define CLIP_SH8_0_255(in0, in1, in2, in3, in4, in5, in6, in7) { CLIP_SH4_0_255(in0, in1, in2, in3); CLIP_SH4_0_255(in4, in5, in6, in7); }
#define CLIP_SW_0_255(in) { in = __msa_maxi_s_w((v4i32) in, 0); in = (v4i32) __msa_sat_u_w((v4u32) in, 7); }
#define CLIP_SW2_0_255(in0, in1) { CLIP_SW_0_255(in0); CLIP_SW_0_255(in1); }
#define CLIP_SW4_0_255(in0, in1, in2, in3) { CLIP_SW2_0_255(in0, in1); CLIP_SW2_0_255(in2, in3); }
#define CLIP_SW8_0_255(in0, in1, in2, in3, in4, in5, in6, in7) { CLIP_SW4_0_255(in0, in1, in2, in3); CLIP_SW4_0_255(in4, in5, in6, in7); }
#define HADD_SW_S32(in) ( { v2i64 res0_m, res1_m; int32_t sum_m; res0_m = __msa_hadd_s_d((v4i32) in, (v4i32) in); res1_m = __msa_splati_d(res0_m, 1); res0_m += res1_m; sum_m = __msa_copy_s_w((v4i32) res0_m, 0); sum_m; } )
#define HADD_UH_U32(in) ( { v4u32 res_m; v2u64 res0_m, res1_m; uint32_t sum_m; res_m = __msa_hadd_u_w((v8u16) in, (v8u16) in); res0_m = __msa_hadd_u_d(res_m, res_m); res1_m = (v2u64) __msa_splati_d((v2i64) res0_m, 1); res0_m += res1_m; sum_m = __msa_copy_u_w((v4i32) res0_m, 0); sum_m; } )
#define HADD_SB2(RTYPE, in0, in1, out0, out1) { out0 = (RTYPE) __msa_hadd_s_h((v16i8) in0, (v16i8) in0); out1 = (RTYPE) __msa_hadd_s_h((v16i8) in1, (v16i8) in1); }
#define HADD_SB2_SH(...) HADD_SB2(v8i16, __VA_ARGS__)
#define HADD_SB4(RTYPE, in0, in1, in2, in3, out0, out1, out2, out3) { HADD_SB2(RTYPE, in0, in1, out0, out1); HADD_SB2(RTYPE, in2, in3, out2, out3); }
#define HADD_SB4_UH(...) HADD_SB4(v8u16, __VA_ARGS__)
#define HADD_SB4_SH(...) HADD_SB4(v8i16, __VA_ARGS__)
#define HADD_UB2(RTYPE, in0, in1, out0, out1) { out0 = (RTYPE) __msa_hadd_u_h((v16u8) in0, (v16u8) in0); out1 = (RTYPE) __msa_hadd_u_h((v16u8) in1, (v16u8) in1); }
#define HADD_UB2_UH(...) HADD_UB2(v8u16, __VA_ARGS__)
#define HADD_UB3(RTYPE, in0, in1, in2, out0, out1, out2) { HADD_UB2(RTYPE, in0, in1, out0, out1); out2 = (RTYPE) __msa_hadd_u_h((v16u8) in2, (v16u8) in2); }
#define HADD_UB3_UH(...) HADD_UB3(v8u16, __VA_ARGS__)
#define HADD_UB4(RTYPE, in0, in1, in2, in3, out0, out1, out2, out3) { HADD_UB2(RTYPE, in0, in1, out0, out1); HADD_UB2(RTYPE, in2, in3, out2, out3); }
#define HADD_UB4_UB(...) HADD_UB4(v16u8, __VA_ARGS__)
#define HADD_UB4_UH(...) HADD_UB4(v8u16, __VA_ARGS__)
#define HADD_UB4_SH(...) HADD_UB4(v8i16, __VA_ARGS__)
#define HSUB_UB2(RTYPE, in0, in1, out0, out1) { out0 = (RTYPE) __msa_hsub_u_h((v16u8) in0, (v16u8) in0); out1 = (RTYPE) __msa_hsub_u_h((v16u8) in1, (v16u8) in1); }
#define HSUB_UB2_UH(...) HSUB_UB2(v8u16, __VA_ARGS__)
#define HSUB_UB2_SH(...) HSUB_UB2(v8i16, __VA_ARGS__)
#define HSUB_UB4(RTYPE, in0, in1, in2, in3, out0, out1, out2, out3) { HSUB_UB2(RTYPE, in0, in1, out0, out1); HSUB_UB2(RTYPE, in2, in3, out2, out3); }
#define HSUB_UB4_UH(...) HSUB_UB4(v8u16, __VA_ARGS__)
#define HSUB_UB4_SH(...) HSUB_UB4(v8i16, __VA_ARGS__)
#if HAVE_MSA2
#define SAD_UB2_UH(in0, in1, ref0, ref1) ( { v8u16 sad_m = { 0 }; sad_m += __builtin_msa2_sad_adj2_u_w2x_b((v16u8) in0, (v16u8) ref0); sad_m += __builtin_msa2_sad_adj2_u_w2x_b((v16u8) in1, (v16u8) ref1); sad_m; } )
#else
#define SAD_UB2_UH(in0, in1, ref0, ref1) ( { v16u8 diff0_m, diff1_m; v8u16 sad_m = { 0 }; diff0_m = __msa_asub_u_b((v16u8) in0, (v16u8) ref0); diff1_m = __msa_asub_u_b((v16u8) in1, (v16u8) ref1); sad_m += __msa_hadd_u_h((v16u8) diff0_m, (v16u8) diff0_m); sad_m += __msa_hadd_u_h((v16u8) diff1_m, (v16u8) diff1_m); sad_m; } )
#endif 
#define INSERT_W2(RTYPE, in0, in1, out) { out = (RTYPE) __msa_insert_w((v4i32) out, 0, in0); out = (RTYPE) __msa_insert_w((v4i32) out, 1, in1); }
#define INSERT_W2_UB(...) INSERT_W2(v16u8, __VA_ARGS__)
#define INSERT_W2_SB(...) INSERT_W2(v16i8, __VA_ARGS__)
#define INSERT_W4(RTYPE, in0, in1, in2, in3, out) { out = (RTYPE) __msa_insert_w((v4i32) out, 0, in0); out = (RTYPE) __msa_insert_w((v4i32) out, 1, in1); out = (RTYPE) __msa_insert_w((v4i32) out, 2, in2); out = (RTYPE) __msa_insert_w((v4i32) out, 3, in3); }
#define INSERT_W4_UB(...) INSERT_W4(v16u8, __VA_ARGS__)
#define INSERT_W4_SB(...) INSERT_W4(v16i8, __VA_ARGS__)
#define INSERT_W4_SH(...) INSERT_W4(v8i16, __VA_ARGS__)
#define INSERT_W4_SW(...) INSERT_W4(v4i32, __VA_ARGS__)
#define INSERT_D2(RTYPE, in0, in1, out) { out = (RTYPE) __msa_insert_d((v2i64) out, 0, in0); out = (RTYPE) __msa_insert_d((v2i64) out, 1, in1); }
#define INSERT_D2_UB(...) INSERT_D2(v16u8, __VA_ARGS__)
#define INSERT_D2_SB(...) INSERT_D2(v16i8, __VA_ARGS__)
#define INSERT_D2_SH(...) INSERT_D2(v8i16, __VA_ARGS__)
#define INSERT_D2_SD(...) INSERT_D2(v2i64, __VA_ARGS__)
#define ILVEV_B2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvev_b((v16i8) in1, (v16i8) in0); out1 = (RTYPE) __msa_ilvev_b((v16i8) in3, (v16i8) in2); }
#define ILVEV_B2_UB(...) ILVEV_B2(v16u8, __VA_ARGS__)
#define ILVEV_B2_SB(...) ILVEV_B2(v16i8, __VA_ARGS__)
#define ILVEV_B2_SH(...) ILVEV_B2(v8i16, __VA_ARGS__)
#define ILVEV_B2_SD(...) ILVEV_B2(v2i64, __VA_ARGS__)
#define ILVEV_H2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvev_h((v8i16) in1, (v8i16) in0); out1 = (RTYPE) __msa_ilvev_h((v8i16) in3, (v8i16) in2); }
#define ILVEV_H2_UB(...) ILVEV_H2(v16u8, __VA_ARGS__)
#define ILVEV_H2_SH(...) ILVEV_H2(v8i16, __VA_ARGS__)
#define ILVEV_H2_SW(...) ILVEV_H2(v4i32, __VA_ARGS__)
#define ILVEV_W2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvev_w((v4i32) in1, (v4i32) in0); out1 = (RTYPE) __msa_ilvev_w((v4i32) in3, (v4i32) in2); }
#define ILVEV_W2_UB(...) ILVEV_W2(v16u8, __VA_ARGS__)
#define ILVEV_W2_SB(...) ILVEV_W2(v16i8, __VA_ARGS__)
#define ILVEV_W2_UH(...) ILVEV_W2(v8u16, __VA_ARGS__)
#define ILVEV_W2_SD(...) ILVEV_W2(v2i64, __VA_ARGS__)
#define ILVEV_D2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvev_d((v2i64) in1, (v2i64) in0); out1 = (RTYPE) __msa_ilvev_d((v2i64) in3, (v2i64) in2); }
#define ILVEV_D2_UB(...) ILVEV_D2(v16u8, __VA_ARGS__)
#define ILVEV_D2_SB(...) ILVEV_D2(v16i8, __VA_ARGS__)
#define ILVEV_D2_SW(...) ILVEV_D2(v4i32, __VA_ARGS__)
#define ILVL_B2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvl_b((v16i8) in0, (v16i8) in1); out1 = (RTYPE) __msa_ilvl_b((v16i8) in2, (v16i8) in3); }
#define ILVL_B2_UB(...) ILVL_B2(v16u8, __VA_ARGS__)
#define ILVL_B2_SB(...) ILVL_B2(v16i8, __VA_ARGS__)
#define ILVL_B2_UH(...) ILVL_B2(v8u16, __VA_ARGS__)
#define ILVL_B2_SH(...) ILVL_B2(v8i16, __VA_ARGS__)
#define ILVL_B4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ILVL_B2(RTYPE, in0, in1, in2, in3, out0, out1); ILVL_B2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ILVL_B4_UB(...) ILVL_B4(v16u8, __VA_ARGS__)
#define ILVL_B4_SB(...) ILVL_B4(v16i8, __VA_ARGS__)
#define ILVL_B4_UH(...) ILVL_B4(v8u16, __VA_ARGS__)
#define ILVL_B4_SH(...) ILVL_B4(v8i16, __VA_ARGS__)
#define ILVL_H2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvl_h((v8i16) in0, (v8i16) in1); out1 = (RTYPE) __msa_ilvl_h((v8i16) in2, (v8i16) in3); }
#define ILVL_H2_SH(...) ILVL_H2(v8i16, __VA_ARGS__)
#define ILVL_H2_SW(...) ILVL_H2(v4i32, __VA_ARGS__)
#define ILVL_H4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ILVL_H2(RTYPE, in0, in1, in2, in3, out0, out1); ILVL_H2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ILVL_H4_SH(...) ILVL_H4(v8i16, __VA_ARGS__)
#define ILVL_H4_SW(...) ILVL_H4(v4i32, __VA_ARGS__)
#define ILVL_W2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvl_w((v4i32) in0, (v4i32) in1); out1 = (RTYPE) __msa_ilvl_w((v4i32) in2, (v4i32) in3); }
#define ILVL_W2_UB(...) ILVL_W2(v16u8, __VA_ARGS__)
#define ILVL_W2_SB(...) ILVL_W2(v16i8, __VA_ARGS__)
#define ILVL_W2_SH(...) ILVL_W2(v8i16, __VA_ARGS__)
#define ILVR_B2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvr_b((v16i8) in0, (v16i8) in1); out1 = (RTYPE) __msa_ilvr_b((v16i8) in2, (v16i8) in3); }
#define ILVR_B2_UB(...) ILVR_B2(v16u8, __VA_ARGS__)
#define ILVR_B2_SB(...) ILVR_B2(v16i8, __VA_ARGS__)
#define ILVR_B2_UH(...) ILVR_B2(v8u16, __VA_ARGS__)
#define ILVR_B2_SH(...) ILVR_B2(v8i16, __VA_ARGS__)
#define ILVR_B2_SW(...) ILVR_B2(v4i32, __VA_ARGS__)
#define ILVR_B3(RTYPE, in0, in1, in2, in3, in4, in5, out0, out1, out2) { ILVR_B2(RTYPE, in0, in1, in2, in3, out0, out1); out2 = (RTYPE) __msa_ilvr_b((v16i8) in4, (v16i8) in5); }
#define ILVR_B3_UB(...) ILVR_B3(v16u8, __VA_ARGS__)
#define ILVR_B3_SB(...) ILVR_B3(v16i8, __VA_ARGS__)
#define ILVR_B3_UH(...) ILVR_B3(v8u16, __VA_ARGS__)
#define ILVR_B3_SH(...) ILVR_B3(v8i16, __VA_ARGS__)
#define ILVR_B4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ILVR_B2(RTYPE, in0, in1, in2, in3, out0, out1); ILVR_B2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ILVR_B4_UB(...) ILVR_B4(v16u8, __VA_ARGS__)
#define ILVR_B4_SB(...) ILVR_B4(v16i8, __VA_ARGS__)
#define ILVR_B4_UH(...) ILVR_B4(v8u16, __VA_ARGS__)
#define ILVR_B4_SH(...) ILVR_B4(v8i16, __VA_ARGS__)
#define ILVR_B4_SW(...) ILVR_B4(v4i32, __VA_ARGS__)
#define ILVR_B8(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, out0, out1, out2, out3, out4, out5, out6, out7) { ILVR_B4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3); ILVR_B4(RTYPE, in8, in9, in10, in11, in12, in13, in14, in15, out4, out5, out6, out7); }
#define ILVR_B8_UH(...) ILVR_B8(v8u16, __VA_ARGS__)
#define ILVR_B8_SW(...) ILVR_B8(v4i32, __VA_ARGS__)
#define ILVR_H2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvr_h((v8i16) in0, (v8i16) in1); out1 = (RTYPE) __msa_ilvr_h((v8i16) in2, (v8i16) in3); }
#define ILVR_H2_SH(...) ILVR_H2(v8i16, __VA_ARGS__)
#define ILVR_H2_SW(...) ILVR_H2(v4i32, __VA_ARGS__)
#define ILVR_H3(RTYPE, in0, in1, in2, in3, in4, in5, out0, out1, out2) { ILVR_H2(RTYPE, in0, in1, in2, in3, out0, out1); out2 = (RTYPE) __msa_ilvr_h((v8i16) in4, (v8i16) in5); }
#define ILVR_H3_SH(...) ILVR_H3(v8i16, __VA_ARGS__)
#define ILVR_H4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ILVR_H2(RTYPE, in0, in1, in2, in3, out0, out1); ILVR_H2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ILVR_H4_SH(...) ILVR_H4(v8i16, __VA_ARGS__)
#define ILVR_H4_SW(...) ILVR_H4(v4i32, __VA_ARGS__)
#define ILVR_W2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvr_w((v4i32) in0, (v4i32) in1); out1 = (RTYPE) __msa_ilvr_w((v4i32) in2, (v4i32) in3); }
#define ILVR_W2_UB(...) ILVR_W2(v16u8, __VA_ARGS__)
#define ILVR_W2_SB(...) ILVR_W2(v16i8, __VA_ARGS__)
#define ILVR_W2_SH(...) ILVR_W2(v8i16, __VA_ARGS__)
#define ILVR_W4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ILVR_W2(RTYPE, in0, in1, in2, in3, out0, out1); ILVR_W2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ILVR_W4_SB(...) ILVR_W4(v16i8, __VA_ARGS__)
#define ILVR_W4_UB(...) ILVR_W4(v16u8, __VA_ARGS__)
#define ILVR_D2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvr_d((v2i64) in0, (v2i64) in1); out1 = (RTYPE) __msa_ilvr_d((v2i64) in2, (v2i64) in3); }
#define ILVR_D2_UB(...) ILVR_D2(v16u8, __VA_ARGS__)
#define ILVR_D2_SB(...) ILVR_D2(v16i8, __VA_ARGS__)
#define ILVR_D2_SH(...) ILVR_D2(v8i16, __VA_ARGS__)
#define ILVR_D3(RTYPE, in0, in1, in2, in3, in4, in5, out0, out1, out2) { ILVR_D2(RTYPE, in0, in1, in2, in3, out0, out1); out2 = (RTYPE) __msa_ilvr_d((v2i64) in4, (v2i64) in5); }
#define ILVR_D3_SB(...) ILVR_D3(v16i8, __VA_ARGS__)
#define ILVR_D4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ILVR_D2(RTYPE, in0, in1, in2, in3, out0, out1); ILVR_D2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ILVR_D4_SB(...) ILVR_D4(v16i8, __VA_ARGS__)
#define ILVR_D4_UB(...) ILVR_D4(v16u8, __VA_ARGS__)
#define ILVL_D2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_ilvl_d((v2i64) in0, (v2i64) in1); out1 = (RTYPE) __msa_ilvl_d((v2i64) in2, (v2i64) in3); }
#define ILVL_D2_UB(...) ILVL_D2(v16u8, __VA_ARGS__)
#define ILVL_D2_SB(...) ILVL_D2(v16i8, __VA_ARGS__)
#define ILVL_D2_SH(...) ILVL_D2(v8i16, __VA_ARGS__)
#define ILVRL_B2(RTYPE, in0, in1, out0, out1) { out0 = (RTYPE) __msa_ilvr_b((v16i8) in0, (v16i8) in1); out1 = (RTYPE) __msa_ilvl_b((v16i8) in0, (v16i8) in1); }
#define ILVRL_B2_UB(...) ILVRL_B2(v16u8, __VA_ARGS__)
#define ILVRL_B2_SB(...) ILVRL_B2(v16i8, __VA_ARGS__)
#define ILVRL_B2_UH(...) ILVRL_B2(v8u16, __VA_ARGS__)
#define ILVRL_B2_SH(...) ILVRL_B2(v8i16, __VA_ARGS__)
#define ILVRL_B2_SW(...) ILVRL_B2(v4i32, __VA_ARGS__)
#define ILVRL_H2(RTYPE, in0, in1, out0, out1) { out0 = (RTYPE) __msa_ilvr_h((v8i16) in0, (v8i16) in1); out1 = (RTYPE) __msa_ilvl_h((v8i16) in0, (v8i16) in1); }
#define ILVRL_H2_UB(...) ILVRL_H2(v16u8, __VA_ARGS__)
#define ILVRL_H2_SB(...) ILVRL_H2(v16i8, __VA_ARGS__)
#define ILVRL_H2_SH(...) ILVRL_H2(v8i16, __VA_ARGS__)
#define ILVRL_H2_SW(...) ILVRL_H2(v4i32, __VA_ARGS__)
#define ILVRL_W2(RTYPE, in0, in1, out0, out1) { out0 = (RTYPE) __msa_ilvr_w((v4i32) in0, (v4i32) in1); out1 = (RTYPE) __msa_ilvl_w((v4i32) in0, (v4i32) in1); }
#define ILVRL_W2_UB(...) ILVRL_W2(v16u8, __VA_ARGS__)
#define ILVRL_W2_SH(...) ILVRL_W2(v8i16, __VA_ARGS__)
#define ILVRL_W2_SW(...) ILVRL_W2(v4i32, __VA_ARGS__)
#define MAXI_SH2(RTYPE, in0, in1, max_val) { in0 = (RTYPE) __msa_maxi_s_h((v8i16) in0, max_val); in1 = (RTYPE) __msa_maxi_s_h((v8i16) in1, max_val); }
#define MAXI_SH2_UH(...) MAXI_SH2(v8u16, __VA_ARGS__)
#define MAXI_SH2_SH(...) MAXI_SH2(v8i16, __VA_ARGS__)
#define MAXI_SH4(RTYPE, in0, in1, in2, in3, max_val) { MAXI_SH2(RTYPE, in0, in1, max_val); MAXI_SH2(RTYPE, in2, in3, max_val); }
#define MAXI_SH4_UH(...) MAXI_SH4(v8u16, __VA_ARGS__)
#define MAXI_SH4_SH(...) MAXI_SH4(v8i16, __VA_ARGS__)
#define MAXI_SH8(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, max_val) { MAXI_SH4(RTYPE, in0, in1, in2, in3, max_val); MAXI_SH4(RTYPE, in4, in5, in6, in7, max_val); }
#define MAXI_SH8_UH(...) MAXI_SH8(v8u16, __VA_ARGS__)
#define MAXI_SH8_SH(...) MAXI_SH8(v8i16, __VA_ARGS__)
#define SAT_UH2(RTYPE, in0, in1, sat_val) { in0 = (RTYPE) __msa_sat_u_h((v8u16) in0, sat_val); in1 = (RTYPE) __msa_sat_u_h((v8u16) in1, sat_val); }
#define SAT_UH2_UH(...) SAT_UH2(v8u16, __VA_ARGS__)
#define SAT_UH2_SH(...) SAT_UH2(v8i16, __VA_ARGS__)
#define SAT_UH4(RTYPE, in0, in1, in2, in3, sat_val) { SAT_UH2(RTYPE, in0, in1, sat_val); SAT_UH2(RTYPE, in2, in3, sat_val); }
#define SAT_UH4_UH(...) SAT_UH4(v8u16, __VA_ARGS__)
#define SAT_UH4_SH(...) SAT_UH4(v8i16, __VA_ARGS__)
#define SAT_UH8(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, sat_val) { SAT_UH4(RTYPE, in0, in1, in2, in3, sat_val); SAT_UH4(RTYPE, in4, in5, in6, in7, sat_val); }
#define SAT_UH8_UH(...) SAT_UH8(v8u16, __VA_ARGS__)
#define SAT_UH8_SH(...) SAT_UH8(v8i16, __VA_ARGS__)
#define SAT_SH2(RTYPE, in0, in1, sat_val) { in0 = (RTYPE) __msa_sat_s_h((v8i16) in0, sat_val); in1 = (RTYPE) __msa_sat_s_h((v8i16) in1, sat_val); }
#define SAT_SH2_SH(...) SAT_SH2(v8i16, __VA_ARGS__)
#define SAT_SH3(RTYPE, in0, in1, in2, sat_val) { SAT_SH2(RTYPE, in0, in1, sat_val); in2 = (RTYPE) __msa_sat_s_h((v8i16) in2, sat_val); }
#define SAT_SH3_SH(...) SAT_SH3(v8i16, __VA_ARGS__)
#define SAT_SH4(RTYPE, in0, in1, in2, in3, sat_val) { SAT_SH2(RTYPE, in0, in1, sat_val); SAT_SH2(RTYPE, in2, in3, sat_val); }
#define SAT_SH4_SH(...) SAT_SH4(v8i16, __VA_ARGS__)
#define SAT_SW2(RTYPE, in0, in1, sat_val) { in0 = (RTYPE) __msa_sat_s_w((v4i32) in0, sat_val); in1 = (RTYPE) __msa_sat_s_w((v4i32) in1, sat_val); }
#define SAT_SW2_SW(...) SAT_SW2(v4i32, __VA_ARGS__)
#define SAT_SW4(RTYPE, in0, in1, in2, in3, sat_val) { SAT_SW2(RTYPE, in0, in1, sat_val); SAT_SW2(RTYPE, in2, in3, sat_val); }
#define SAT_SW4_SW(...) SAT_SW4(v4i32, __VA_ARGS__)
#define SPLATI_H2(RTYPE, in, idx0, idx1, out0, out1) { out0 = (RTYPE) __msa_splati_h((v8i16) in, idx0); out1 = (RTYPE) __msa_splati_h((v8i16) in, idx1); }
#define SPLATI_H2_SB(...) SPLATI_H2(v16i8, __VA_ARGS__)
#define SPLATI_H2_SH(...) SPLATI_H2(v8i16, __VA_ARGS__)
#define SPLATI_H3(RTYPE, in, idx0, idx1, idx2, out0, out1, out2) { SPLATI_H2(RTYPE, in, idx0, idx1, out0, out1); out2 = (RTYPE) __msa_splati_h((v8i16) in, idx2); }
#define SPLATI_H3_SB(...) SPLATI_H3(v16i8, __VA_ARGS__)
#define SPLATI_H3_SH(...) SPLATI_H3(v8i16, __VA_ARGS__)
#define SPLATI_H4(RTYPE, in, idx0, idx1, idx2, idx3, out0, out1, out2, out3) { SPLATI_H2(RTYPE, in, idx0, idx1, out0, out1); SPLATI_H2(RTYPE, in, idx2, idx3, out2, out3); }
#define SPLATI_H4_SB(...) SPLATI_H4(v16i8, __VA_ARGS__)
#define SPLATI_H4_SH(...) SPLATI_H4(v8i16, __VA_ARGS__)
#define SPLATI_W2(RTYPE, in, stidx, out0, out1) { out0 = (RTYPE) __msa_splati_w((v4i32) in, stidx); out1 = (RTYPE) __msa_splati_w((v4i32) in, (stidx+1)); }
#define SPLATI_W2_SH(...) SPLATI_W2(v8i16, __VA_ARGS__)
#define SPLATI_W2_SW(...) SPLATI_W2(v4i32, __VA_ARGS__)
#define SPLATI_W4(RTYPE, in, out0, out1, out2, out3) { SPLATI_W2(RTYPE, in, 0, out0, out1); SPLATI_W2(RTYPE, in, 2, out2, out3); }
#define SPLATI_W4_SH(...) SPLATI_W4(v8i16, __VA_ARGS__)
#define SPLATI_W4_SW(...) SPLATI_W4(v4i32, __VA_ARGS__)
#define PCKEV_B2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_pckev_b((v16i8) in0, (v16i8) in1); out1 = (RTYPE) __msa_pckev_b((v16i8) in2, (v16i8) in3); }
#define PCKEV_B2_SB(...) PCKEV_B2(v16i8, __VA_ARGS__)
#define PCKEV_B2_UB(...) PCKEV_B2(v16u8, __VA_ARGS__)
#define PCKEV_B2_SH(...) PCKEV_B2(v8i16, __VA_ARGS__)
#define PCKEV_B2_SW(...) PCKEV_B2(v4i32, __VA_ARGS__)
#define PCKEV_B3(RTYPE, in0, in1, in2, in3, in4, in5, out0, out1, out2) { PCKEV_B2(RTYPE, in0, in1, in2, in3, out0, out1); out2 = (RTYPE) __msa_pckev_b((v16i8) in4, (v16i8) in5); }
#define PCKEV_B3_UB(...) PCKEV_B3(v16u8, __VA_ARGS__)
#define PCKEV_B3_SB(...) PCKEV_B3(v16i8, __VA_ARGS__)
#define PCKEV_B4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { PCKEV_B2(RTYPE, in0, in1, in2, in3, out0, out1); PCKEV_B2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define PCKEV_B4_SB(...) PCKEV_B4(v16i8, __VA_ARGS__)
#define PCKEV_B4_UB(...) PCKEV_B4(v16u8, __VA_ARGS__)
#define PCKEV_B4_SH(...) PCKEV_B4(v8i16, __VA_ARGS__)
#define PCKEV_B4_SW(...) PCKEV_B4(v4i32, __VA_ARGS__)
#define PCKEV_H2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_pckev_h((v8i16) in0, (v8i16) in1); out1 = (RTYPE) __msa_pckev_h((v8i16) in2, (v8i16) in3); }
#define PCKEV_H2_SH(...) PCKEV_H2(v8i16, __VA_ARGS__)
#define PCKEV_H2_SW(...) PCKEV_H2(v4i32, __VA_ARGS__)
#define PCKEV_H4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { PCKEV_H2(RTYPE, in0, in1, in2, in3, out0, out1); PCKEV_H2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define PCKEV_H4_SH(...) PCKEV_H4(v8i16, __VA_ARGS__)
#define PCKEV_H4_SW(...) PCKEV_H4(v4i32, __VA_ARGS__)
#define PCKEV_D2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_pckev_d((v2i64) in0, (v2i64) in1); out1 = (RTYPE) __msa_pckev_d((v2i64) in2, (v2i64) in3); }
#define PCKEV_D2_UB(...) PCKEV_D2(v16u8, __VA_ARGS__)
#define PCKEV_D2_SB(...) PCKEV_D2(v16i8, __VA_ARGS__)
#define PCKEV_D2_SH(...) PCKEV_D2(v8i16, __VA_ARGS__)
#define PCKEV_D4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { PCKEV_D2(RTYPE, in0, in1, in2, in3, out0, out1); PCKEV_D2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define PCKEV_D4_UB(...) PCKEV_D4(v16u8, __VA_ARGS__)
#define PCKOD_D2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_pckod_d((v2i64) in0, (v2i64) in1); out1 = (RTYPE) __msa_pckod_d((v2i64) in2, (v2i64) in3); }
#define PCKOD_D2_UB(...) PCKOD_D2(v16u8, __VA_ARGS__)
#define PCKOD_D2_SH(...) PCKOD_D2(v8i16, __VA_ARGS__)
#define PCKOD_D2_SD(...) PCKOD_D2(v2i64, __VA_ARGS__)
#define XORI_B2_128(RTYPE, in0, in1) { in0 = (RTYPE) __msa_xori_b((v16u8) in0, 128); in1 = (RTYPE) __msa_xori_b((v16u8) in1, 128); }
#define XORI_B2_128_UB(...) XORI_B2_128(v16u8, __VA_ARGS__)
#define XORI_B2_128_SB(...) XORI_B2_128(v16i8, __VA_ARGS__)
#define XORI_B2_128_SH(...) XORI_B2_128(v8i16, __VA_ARGS__)
#define XORI_B3_128(RTYPE, in0, in1, in2) { XORI_B2_128(RTYPE, in0, in1); in2 = (RTYPE) __msa_xori_b((v16u8) in2, 128); }
#define XORI_B3_128_SB(...) XORI_B3_128(v16i8, __VA_ARGS__)
#define XORI_B4_128(RTYPE, in0, in1, in2, in3) { XORI_B2_128(RTYPE, in0, in1); XORI_B2_128(RTYPE, in2, in3); }
#define XORI_B4_128_UB(...) XORI_B4_128(v16u8, __VA_ARGS__)
#define XORI_B4_128_SB(...) XORI_B4_128(v16i8, __VA_ARGS__)
#define XORI_B4_128_SH(...) XORI_B4_128(v8i16, __VA_ARGS__)
#define XORI_B5_128(RTYPE, in0, in1, in2, in3, in4) { XORI_B3_128(RTYPE, in0, in1, in2); XORI_B2_128(RTYPE, in3, in4); }
#define XORI_B5_128_SB(...) XORI_B5_128(v16i8, __VA_ARGS__)
#define XORI_B6_128(RTYPE, in0, in1, in2, in3, in4, in5) { XORI_B4_128(RTYPE, in0, in1, in2, in3); XORI_B2_128(RTYPE, in4, in5); }
#define XORI_B6_128_SB(...) XORI_B6_128(v16i8, __VA_ARGS__)
#define XORI_B7_128(RTYPE, in0, in1, in2, in3, in4, in5, in6) { XORI_B4_128(RTYPE, in0, in1, in2, in3); XORI_B3_128(RTYPE, in4, in5, in6); }
#define XORI_B7_128_SB(...) XORI_B7_128(v16i8, __VA_ARGS__)
#define XORI_B8_128(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7) { XORI_B4_128(RTYPE, in0, in1, in2, in3); XORI_B4_128(RTYPE, in4, in5, in6, in7); }
#define XORI_B8_128_SB(...) XORI_B8_128(v16i8, __VA_ARGS__)
#define XORI_B8_128_UB(...) XORI_B8_128(v16u8, __VA_ARGS__)
#define ADDS_SH2(RTYPE, in0, in1, in2, in3, out0, out1) { out0 = (RTYPE) __msa_adds_s_h((v8i16) in0, (v8i16) in1); out1 = (RTYPE) __msa_adds_s_h((v8i16) in2, (v8i16) in3); }
#define ADDS_SH2_SH(...) ADDS_SH2(v8i16, __VA_ARGS__)
#define ADDS_SH4(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ADDS_SH2(RTYPE, in0, in1, in2, in3, out0, out1); ADDS_SH2(RTYPE, in4, in5, in6, in7, out2, out3); }
#define ADDS_SH4_UH(...) ADDS_SH4(v8u16, __VA_ARGS__)
#define ADDS_SH4_SH(...) ADDS_SH4(v8i16, __VA_ARGS__)
#define SLLI_2V(in0, in1, shift) { in0 = in0 << shift; in1 = in1 << shift; }
#define SLLI_4V(in0, in1, in2, in3, shift) { in0 = in0 << shift; in1 = in1 << shift; in2 = in2 << shift; in3 = in3 << shift; }
#define SRA_4V(in0, in1, in2, in3, shift) { in0 = in0 >> shift; in1 = in1 >> shift; in2 = in2 >> shift; in3 = in3 >> shift; }
#define SRL_H4(RTYPE, in0, in1, in2, in3, shift) { in0 = (RTYPE) __msa_srl_h((v8i16) in0, (v8i16) shift); in1 = (RTYPE) __msa_srl_h((v8i16) in1, (v8i16) shift); in2 = (RTYPE) __msa_srl_h((v8i16) in2, (v8i16) shift); in3 = (RTYPE) __msa_srl_h((v8i16) in3, (v8i16) shift); }
#define SRL_H4_UH(...) SRL_H4(v8u16, __VA_ARGS__)
#define SRLR_H4(RTYPE, in0, in1, in2, in3, shift) { in0 = (RTYPE) __msa_srlr_h((v8i16) in0, (v8i16) shift); in1 = (RTYPE) __msa_srlr_h((v8i16) in1, (v8i16) shift); in2 = (RTYPE) __msa_srlr_h((v8i16) in2, (v8i16) shift); in3 = (RTYPE) __msa_srlr_h((v8i16) in3, (v8i16) shift); }
#define SRLR_H4_UH(...) SRLR_H4(v8u16, __VA_ARGS__)
#define SRLR_H4_SH(...) SRLR_H4(v8i16, __VA_ARGS__)
#define SRLR_H8(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, shift) { SRLR_H4(RTYPE, in0, in1, in2, in3, shift); SRLR_H4(RTYPE, in4, in5, in6, in7, shift); }
#define SRLR_H8_UH(...) SRLR_H8(v8u16, __VA_ARGS__)
#define SRLR_H8_SH(...) SRLR_H8(v8i16, __VA_ARGS__)
#define SRAR_H2(RTYPE, in0, in1, shift) { in0 = (RTYPE) __msa_srar_h((v8i16) in0, (v8i16) shift); in1 = (RTYPE) __msa_srar_h((v8i16) in1, (v8i16) shift); }
#define SRAR_H2_UH(...) SRAR_H2(v8u16, __VA_ARGS__)
#define SRAR_H2_SH(...) SRAR_H2(v8i16, __VA_ARGS__)
#define SRAR_H3(RTYPE, in0, in1, in2, shift) { SRAR_H2(RTYPE, in0, in1, shift) in2 = (RTYPE) __msa_srar_h((v8i16) in2, (v8i16) shift); }
#define SRAR_H3_SH(...) SRAR_H3(v8i16, __VA_ARGS__)
#define SRAR_H4(RTYPE, in0, in1, in2, in3, shift) { SRAR_H2(RTYPE, in0, in1, shift) SRAR_H2(RTYPE, in2, in3, shift) }
#define SRAR_H4_UH(...) SRAR_H4(v8u16, __VA_ARGS__)
#define SRAR_H4_SH(...) SRAR_H4(v8i16, __VA_ARGS__)
#define SRAR_W2(RTYPE, in0, in1, shift) { in0 = (RTYPE) __msa_srar_w((v4i32) in0, (v4i32) shift); in1 = (RTYPE) __msa_srar_w((v4i32) in1, (v4i32) shift); }
#define SRAR_W2_SW(...) SRAR_W2(v4i32, __VA_ARGS__)
#define SRAR_W4(RTYPE, in0, in1, in2, in3, shift) { SRAR_W2(RTYPE, in0, in1, shift) SRAR_W2(RTYPE, in2, in3, shift) }
#define SRAR_W4_SW(...) SRAR_W4(v4i32, __VA_ARGS__)
#define SRARI_H2(RTYPE, in0, in1, shift) { in0 = (RTYPE) __msa_srari_h((v8i16) in0, shift); in1 = (RTYPE) __msa_srari_h((v8i16) in1, shift); }
#define SRARI_H2_UH(...) SRARI_H2(v8u16, __VA_ARGS__)
#define SRARI_H2_SH(...) SRARI_H2(v8i16, __VA_ARGS__)
#define SRARI_H4(RTYPE, in0, in1, in2, in3, shift) { SRARI_H2(RTYPE, in0, in1, shift); SRARI_H2(RTYPE, in2, in3, shift); }
#define SRARI_H4_UH(...) SRARI_H4(v8u16, __VA_ARGS__)
#define SRARI_H4_SH(...) SRARI_H4(v8i16, __VA_ARGS__)
#define SRARI_W2(RTYPE, in0, in1, shift) { in0 = (RTYPE) __msa_srari_w((v4i32) in0, shift); in1 = (RTYPE) __msa_srari_w((v4i32) in1, shift); }
#define SRARI_W2_SW(...) SRARI_W2(v4i32, __VA_ARGS__)
#define SRARI_W4(RTYPE, in0, in1, in2, in3, shift) { SRARI_W2(RTYPE, in0, in1, shift); SRARI_W2(RTYPE, in2, in3, shift); }
#define SRARI_W4_SH(...) SRARI_W4(v8i16, __VA_ARGS__)
#define SRARI_W4_SW(...) SRARI_W4(v4i32, __VA_ARGS__)
#define MUL2(in0, in1, in2, in3, out0, out1) { out0 = in0 * in1; out1 = in2 * in3; }
#define MUL4(in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { MUL2(in0, in1, in2, in3, out0, out1); MUL2(in4, in5, in6, in7, out2, out3); }
#define ADD2(in0, in1, in2, in3, out0, out1) { out0 = in0 + in1; out1 = in2 + in3; }
#define ADD4(in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { ADD2(in0, in1, in2, in3, out0, out1); ADD2(in4, in5, in6, in7, out2, out3); }
#define SUB2(in0, in1, in2, in3, out0, out1) { out0 = in0 - in1; out1 = in2 - in3; }
#define SUB4(in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { out0 = in0 - in1; out1 = in2 - in3; out2 = in4 - in5; out3 = in6 - in7; }
#define UNPCK_R_SB_SH(in, out) { v16i8 sign_m; sign_m = __msa_clti_s_b((v16i8) in, 0); out = (v8i16) __msa_ilvr_b(sign_m, (v16i8) in); }
#if HAVE_MSA2
#define UNPCK_R_SH_SW(in, out) { out = (v4i32) __builtin_msa2_w2x_lo_s_h((v8i16) in); }
#else
#define UNPCK_R_SH_SW(in, out) { v8i16 sign_m; sign_m = __msa_clti_s_h((v8i16) in, 0); out = (v4i32) __msa_ilvr_h(sign_m, (v8i16) in); }
#endif 
#if HAVE_MSA2
#define UNPCK_SB_SH(in, out0, out1) { out0 = (v4i32) __builtin_msa2_w2x_lo_s_b((v16i8) in); out1 = (v4i32) __builtin_msa2_w2x_hi_s_b((v16i8) in); }
#else
#define UNPCK_SB_SH(in, out0, out1) { v16i8 tmp_m; tmp_m = __msa_clti_s_b((v16i8) in, 0); ILVRL_B2_SH(tmp_m, in, out0, out1); }
#endif 
#define UNPCK_UB_SH(in, out0, out1) { v16i8 zero_m = { 0 }; ILVRL_B2_SH(zero_m, in, out0, out1); }
#if HAVE_MSA2
#define UNPCK_SH_SW(in, out0, out1) { out0 = (v4i32) __builtin_msa2_w2x_lo_s_h((v8i16) in); out1 = (v4i32) __builtin_msa2_w2x_hi_s_h((v8i16) in); }
#else
#define UNPCK_SH_SW(in, out0, out1) { v8i16 tmp_m; tmp_m = __msa_clti_s_h((v8i16) in, 0); ILVRL_H2_SW(tmp_m, in, out0, out1); }
#endif 
#define SWAP(in0, in1) { in0 = in0 ^ in1; in1 = in0 ^ in1; in0 = in0 ^ in1; }
#define BUTTERFLY_4(in0, in1, in2, in3, out0, out1, out2, out3) { out0 = in0 + in3; out1 = in1 + in2; out2 = in1 - in2; out3 = in0 - in3; }
#define BUTTERFLY_8(in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3, out4, out5, out6, out7) { out0 = in0 + in7; out1 = in1 + in6; out2 = in2 + in5; out3 = in3 + in4; out4 = in3 - in4; out5 = in2 - in5; out6 = in1 - in6; out7 = in0 - in7; }
#define BUTTERFLY_16(in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, out0, out1, out2, out3, out4, out5, out6, out7, out8, out9, out10, out11, out12, out13, out14, out15) { out0 = in0 + in15; out1 = in1 + in14; out2 = in2 + in13; out3 = in3 + in12; out4 = in4 + in11; out5 = in5 + in10; out6 = in6 + in9; out7 = in7 + in8; out8 = in7 - in8; out9 = in6 - in9; out10 = in5 - in10; out11 = in4 - in11; out12 = in3 - in12; out13 = in2 - in13; out14 = in1 - in14; out15 = in0 - in15; }
#define TRANSPOSE4x4_UB_UB(in0, in1, in2, in3, out0, out1, out2, out3) { v16i8 zero_m = { 0 }; v16i8 s0_m, s1_m, s2_m, s3_m; ILVR_D2_SB(in1, in0, in3, in2, s0_m, s1_m); ILVRL_B2_SB(s1_m, s0_m, s2_m, s3_m); out0 = (v16u8) __msa_ilvr_b(s3_m, s2_m); out1 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out0, 4); out2 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out1, 4); out3 = (v16u8) __msa_sldi_b(zero_m, (v16i8) out2, 4); }
#define TRANSPOSE8x4_UB(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3) { v16i8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; ILVEV_W2_SB(in0, in4, in1, in5, tmp0_m, tmp1_m); tmp2_m = __msa_ilvr_b(tmp1_m, tmp0_m); ILVEV_W2_SB(in2, in6, in3, in7, tmp0_m, tmp1_m); tmp3_m = __msa_ilvr_b(tmp1_m, tmp0_m); ILVRL_H2_SB(tmp3_m, tmp2_m, tmp0_m, tmp1_m); ILVRL_W2(RTYPE, tmp1_m, tmp0_m, out0, out2); out1 = (RTYPE) __msa_ilvl_d((v2i64) out2, (v2i64) out0); out3 = (RTYPE) __msa_ilvl_d((v2i64) out0, (v2i64) out2); }
#define TRANSPOSE8x4_UB_UB(...) TRANSPOSE8x4_UB(v16u8, __VA_ARGS__)
#define TRANSPOSE8x4_UB_UH(...) TRANSPOSE8x4_UB(v8u16, __VA_ARGS__)
#define TRANSPOSE8x8_UB(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3, out4, out5, out6, out7) { v16i8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; v16i8 tmp4_m, tmp5_m, tmp6_m, tmp7_m; v16i8 zeros = { 0 }; ILVR_B4_SB(in2, in0, in3, in1, in6, in4, in7, in5, tmp0_m, tmp1_m, tmp2_m, tmp3_m); ILVRL_B2_SB(tmp1_m, tmp0_m, tmp4_m, tmp5_m); ILVRL_B2_SB(tmp3_m, tmp2_m, tmp6_m, tmp7_m); ILVRL_W2(RTYPE, tmp6_m, tmp4_m, out0, out2); ILVRL_W2(RTYPE, tmp7_m, tmp5_m, out4, out6); SLDI_B4(RTYPE, zeros, out0, zeros, out2, zeros, out4, zeros, out6, 8, out1, out3, out5, out7); }
#define TRANSPOSE8x8_UB_UB(...) TRANSPOSE8x8_UB(v16u8, __VA_ARGS__)
#define TRANSPOSE8x8_UB_UH(...) TRANSPOSE8x8_UB(v8u16, __VA_ARGS__)
#define TRANSPOSE16x4_UB_UB(in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, out0, out1, out2, out3) { v2i64 tmp0_m, tmp1_m, tmp2_m, tmp3_m; ILVEV_W2_SD(in0, in4, in8, in12, tmp0_m, tmp1_m); out1 = (v16u8) __msa_ilvev_d(tmp1_m, tmp0_m); ILVEV_W2_SD(in1, in5, in9, in13, tmp0_m, tmp1_m); out3 = (v16u8) __msa_ilvev_d(tmp1_m, tmp0_m); ILVEV_W2_SD(in2, in6, in10, in14, tmp0_m, tmp1_m); tmp2_m = __msa_ilvev_d(tmp1_m, tmp0_m); ILVEV_W2_SD(in3, in7, in11, in15, tmp0_m, tmp1_m); tmp3_m = __msa_ilvev_d(tmp1_m, tmp0_m); ILVEV_B2_SD(out1, out3, tmp2_m, tmp3_m, tmp0_m, tmp1_m); out0 = (v16u8) __msa_ilvev_h((v8i16) tmp1_m, (v8i16) tmp0_m); out2 = (v16u8) __msa_ilvod_h((v8i16) tmp1_m, (v8i16) tmp0_m); tmp0_m = (v2i64) __msa_ilvod_b((v16i8) out3, (v16i8) out1); tmp1_m = (v2i64) __msa_ilvod_b((v16i8) tmp3_m, (v16i8) tmp2_m); out1 = (v16u8) __msa_ilvev_h((v8i16) tmp1_m, (v8i16) tmp0_m); out3 = (v16u8) __msa_ilvod_h((v8i16) tmp1_m, (v8i16) tmp0_m); }
#define TRANSPOSE16x8_UB_UB(in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, out0, out1, out2, out3, out4, out5, out6, out7) { v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; v16u8 tmp4_m, tmp5_m, tmp6_m, tmp7_m; ILVEV_D2_UB(in0, in8, in1, in9, out7, out6); ILVEV_D2_UB(in2, in10, in3, in11, out5, out4); ILVEV_D2_UB(in4, in12, in5, in13, out3, out2); ILVEV_D2_UB(in6, in14, in7, in15, out1, out0); tmp0_m = (v16u8) __msa_ilvev_b((v16i8) out6, (v16i8) out7); tmp4_m = (v16u8) __msa_ilvod_b((v16i8) out6, (v16i8) out7); tmp1_m = (v16u8) __msa_ilvev_b((v16i8) out4, (v16i8) out5); tmp5_m = (v16u8) __msa_ilvod_b((v16i8) out4, (v16i8) out5); out5 = (v16u8) __msa_ilvev_b((v16i8) out2, (v16i8) out3); tmp6_m = (v16u8) __msa_ilvod_b((v16i8) out2, (v16i8) out3); out7 = (v16u8) __msa_ilvev_b((v16i8) out0, (v16i8) out1); tmp7_m = (v16u8) __msa_ilvod_b((v16i8) out0, (v16i8) out1); ILVEV_H2_UB(tmp0_m, tmp1_m, out5, out7, tmp2_m, tmp3_m); out0 = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m); out4 = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m); tmp2_m = (v16u8) __msa_ilvod_h((v8i16) tmp1_m, (v8i16) tmp0_m); tmp3_m = (v16u8) __msa_ilvod_h((v8i16) out7, (v8i16) out5); out2 = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m); out6 = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m); ILVEV_H2_UB(tmp4_m, tmp5_m, tmp6_m, tmp7_m, tmp2_m, tmp3_m); out1 = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m); out5 = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m); tmp2_m = (v16u8) __msa_ilvod_h((v8i16) tmp5_m, (v8i16) tmp4_m); tmp3_m = (v16u8) __msa_ilvod_h((v8i16) tmp7_m, (v8i16) tmp6_m); out3 = (v16u8) __msa_ilvev_w((v4i32) tmp3_m, (v4i32) tmp2_m); out7 = (v16u8) __msa_ilvod_w((v4i32) tmp3_m, (v4i32) tmp2_m); }
#define TRANSPOSE4x4_SH_SH(in0, in1, in2, in3, out0, out1, out2, out3) { v8i16 s0_m, s1_m; ILVR_H2_SH(in1, in0, in3, in2, s0_m, s1_m); ILVRL_W2_SH(s1_m, s0_m, out0, out2); out1 = (v8i16) __msa_ilvl_d((v2i64) out0, (v2i64) out0); out3 = (v8i16) __msa_ilvl_d((v2i64) out0, (v2i64) out2); }
#define TRANSPOSE8x8_H(RTYPE, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3, out4, out5, out6, out7) { v8i16 s0_m, s1_m; v8i16 tmp0_m, tmp1_m, tmp2_m, tmp3_m; v8i16 tmp4_m, tmp5_m, tmp6_m, tmp7_m; ILVR_H2_SH(in6, in4, in7, in5, s0_m, s1_m); ILVRL_H2_SH(s1_m, s0_m, tmp0_m, tmp1_m); ILVL_H2_SH(in6, in4, in7, in5, s0_m, s1_m); ILVRL_H2_SH(s1_m, s0_m, tmp2_m, tmp3_m); ILVR_H2_SH(in2, in0, in3, in1, s0_m, s1_m); ILVRL_H2_SH(s1_m, s0_m, tmp4_m, tmp5_m); ILVL_H2_SH(in2, in0, in3, in1, s0_m, s1_m); ILVRL_H2_SH(s1_m, s0_m, tmp6_m, tmp7_m); PCKEV_D4(RTYPE, tmp0_m, tmp4_m, tmp1_m, tmp5_m, tmp2_m, tmp6_m, tmp3_m, tmp7_m, out0, out2, out4, out6); out1 = (RTYPE) __msa_pckod_d((v2i64) tmp0_m, (v2i64) tmp4_m); out3 = (RTYPE) __msa_pckod_d((v2i64) tmp1_m, (v2i64) tmp5_m); out5 = (RTYPE) __msa_pckod_d((v2i64) tmp2_m, (v2i64) tmp6_m); out7 = (RTYPE) __msa_pckod_d((v2i64) tmp3_m, (v2i64) tmp7_m); }
#define TRANSPOSE8x8_UH_UH(...) TRANSPOSE8x8_H(v8u16, __VA_ARGS__)
#define TRANSPOSE8x8_SH_SH(...) TRANSPOSE8x8_H(v8i16, __VA_ARGS__)
#define TRANSPOSE4x4_SW_SW(in0, in1, in2, in3, out0, out1, out2, out3) { v4i32 s0_m, s1_m, s2_m, s3_m; ILVRL_W2_SW(in1, in0, s0_m, s1_m); ILVRL_W2_SW(in3, in2, s2_m, s3_m); out0 = (v4i32) __msa_ilvr_d((v2i64) s2_m, (v2i64) s0_m); out1 = (v4i32) __msa_ilvl_d((v2i64) s2_m, (v2i64) s0_m); out2 = (v4i32) __msa_ilvr_d((v2i64) s3_m, (v2i64) s1_m); out3 = (v4i32) __msa_ilvl_d((v2i64) s3_m, (v2i64) s1_m); }
#define AVE_ST8x4_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { uint64_t out0_m, out1_m, out2_m, out3_m; v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; tmp0_m = __msa_ave_u_b((v16u8) in0, (v16u8) in1); tmp1_m = __msa_ave_u_b((v16u8) in2, (v16u8) in3); tmp2_m = __msa_ave_u_b((v16u8) in4, (v16u8) in5); tmp3_m = __msa_ave_u_b((v16u8) in6, (v16u8) in7); out0_m = __msa_copy_u_d((v2i64) tmp0_m, 0); out1_m = __msa_copy_u_d((v2i64) tmp1_m, 0); out2_m = __msa_copy_u_d((v2i64) tmp2_m, 0); out3_m = __msa_copy_u_d((v2i64) tmp3_m, 0); SD4(out0_m, out1_m, out2_m, out3_m, pdst, stride); }
#define AVE_ST16x4_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; tmp0_m = __msa_ave_u_b((v16u8) in0, (v16u8) in1); tmp1_m = __msa_ave_u_b((v16u8) in2, (v16u8) in3); tmp2_m = __msa_ave_u_b((v16u8) in4, (v16u8) in5); tmp3_m = __msa_ave_u_b((v16u8) in6, (v16u8) in7); ST_UB4(tmp0_m, tmp1_m, tmp2_m, tmp3_m, pdst, stride); }
#define AVER_ST8x4_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { uint64_t out0_m, out1_m, out2_m, out3_m; v16u8 tp0_m, tp1_m, tp2_m, tp3_m; AVER_UB4_UB(in0, in1, in2, in3, in4, in5, in6, in7, tp0_m, tp1_m, tp2_m, tp3_m); out0_m = __msa_copy_u_d((v2i64) tp0_m, 0); out1_m = __msa_copy_u_d((v2i64) tp1_m, 0); out2_m = __msa_copy_u_d((v2i64) tp2_m, 0); out3_m = __msa_copy_u_d((v2i64) tp3_m, 0); SD4(out0_m, out1_m, out2_m, out3_m, pdst, stride); }
#define AVER_ST16x4_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { v16u8 t0_m, t1_m, t2_m, t3_m; AVER_UB4_UB(in0, in1, in2, in3, in4, in5, in6, in7, t0_m, t1_m, t2_m, t3_m); ST_UB4(t0_m, t1_m, t2_m, t3_m, pdst, stride); }
#define AVER_DST_ST8x4_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; v16u8 dst0_m, dst1_m, dst2_m, dst3_m; LD_UB4(pdst, stride, dst0_m, dst1_m, dst2_m, dst3_m); AVER_UB4_UB(in0, in1, in2, in3, in4, in5, in6, in7, tmp0_m, tmp1_m, tmp2_m, tmp3_m); AVER_ST8x4_UB(dst0_m, tmp0_m, dst1_m, tmp1_m, dst2_m, tmp2_m, dst3_m, tmp3_m, pdst, stride); }
#define AVER_DST_ST16x4_UB(in0, in1, in2, in3, in4, in5, in6, in7, pdst, stride) { v16u8 tmp0_m, tmp1_m, tmp2_m, tmp3_m; v16u8 dst0_m, dst1_m, dst2_m, dst3_m; LD_UB4(pdst, stride, dst0_m, dst1_m, dst2_m, dst3_m); AVER_UB4_UB(in0, in1, in2, in3, in4, in5, in6, in7, tmp0_m, tmp1_m, tmp2_m, tmp3_m); AVER_ST16x4_UB(dst0_m, tmp0_m, dst1_m, tmp1_m, dst2_m, tmp2_m, dst3_m, tmp3_m, pdst, stride); }
#define ADDBLK_ST4x4_UB(in0, in1, in2, in3, pdst, stride) { uint32_t src0_m, src1_m, src2_m, src3_m; uint32_t out0_m, out1_m, out2_m, out3_m; v8i16 inp0_m, inp1_m, res0_m, res1_m; v16i8 dst0_m = { 0 }; v16i8 dst1_m = { 0 }; v16i8 zero_m = { 0 }; ILVR_D2_SH(in1, in0, in3, in2, inp0_m, inp1_m) LW4(pdst, stride, src0_m, src1_m, src2_m, src3_m); INSERT_W2_SB(src0_m, src1_m, dst0_m); INSERT_W2_SB(src2_m, src3_m, dst1_m); ILVR_B2_SH(zero_m, dst0_m, zero_m, dst1_m, res0_m, res1_m); ADD2(res0_m, inp0_m, res1_m, inp1_m, res0_m, res1_m); CLIP_SH2_0_255(res0_m, res1_m); PCKEV_B2_SB(res0_m, res0_m, res1_m, res1_m, dst0_m, dst1_m); out0_m = __msa_copy_u_w((v4i32) dst0_m, 0); out1_m = __msa_copy_u_w((v4i32) dst0_m, 1); out2_m = __msa_copy_u_w((v4i32) dst1_m, 0); out3_m = __msa_copy_u_w((v4i32) dst1_m, 1); SW4(out0_m, out1_m, out2_m, out3_m, pdst, stride); }
#define DPADD_SH3_SH(in0, in1, in2, coeff0, coeff1, coeff2) ( { v8i16 out0_m; out0_m = __msa_dotp_s_h((v16i8) in0, (v16i8) coeff0); out0_m = __msa_dpadd_s_h(out0_m, (v16i8) in1, (v16i8) coeff1); out0_m = __msa_dpadd_s_h(out0_m, (v16i8) in2, (v16i8) coeff2); out0_m; } )
#define PCKEV_XORI128_UB(in0, in1) ( { v16u8 out_m; out_m = (v16u8) __msa_pckev_b((v16i8) in1, (v16i8) in0); out_m = (v16u8) __msa_xori_b((v16u8) out_m, 128); out_m; } )
#define CONVERT_UB_AVG_ST8x4_UB(in0, in1, in2, in3, dst0, dst1, pdst, stride) { v16u8 tmp0_m, tmp1_m; uint8_t *pdst_m = (uint8_t *) (pdst); tmp0_m = PCKEV_XORI128_UB(in0, in1); tmp1_m = PCKEV_XORI128_UB(in2, in3); AVER_UB2_UB(tmp0_m, dst0, tmp1_m, dst1, tmp0_m, tmp1_m); ST_D4(tmp0_m, tmp1_m, 0, 1, 0, 1, pdst_m, stride); }
#define PCKEV_ST4x4_UB(in0, in1, in2, in3, pdst, stride) { uint32_t out0_m, out1_m, out2_m, out3_m; v16i8 tmp0_m, tmp1_m; PCKEV_B2_SB(in1, in0, in3, in2, tmp0_m, tmp1_m); out0_m = __msa_copy_u_w((v4i32) tmp0_m, 0); out1_m = __msa_copy_u_w((v4i32) tmp0_m, 2); out2_m = __msa_copy_u_w((v4i32) tmp1_m, 0); out3_m = __msa_copy_u_w((v4i32) tmp1_m, 2); SW4(out0_m, out1_m, out2_m, out3_m, pdst, stride); }
#define PCKEV_ST_SB(in0, in1, pdst) { v16i8 tmp_m; tmp_m = __msa_pckev_b((v16i8) in1, (v16i8) in0); ST_SB(tmp_m, (pdst)); }
#define HORIZ_2TAP_FILT_UH(in0, in1, mask, coeff, shift) ( { v16i8 tmp0_m; v8u16 tmp1_m; tmp0_m = __msa_vshf_b((v16i8) mask, (v16i8) in1, (v16i8) in0); tmp1_m = __msa_dotp_u_h((v16u8) tmp0_m, (v16u8) coeff); tmp1_m = (v8u16) __msa_srari_h((v8i16) tmp1_m, shift); tmp1_m = __msa_sat_u_h(tmp1_m, shift); tmp1_m; } )
