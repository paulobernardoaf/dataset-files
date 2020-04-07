static void FUNC(ff_hevc_idct_4x4, BIT_DEPTH)(int16_t *coeffs, int col_limit)
{
const int shift = 7;
const int shift2 = 20 - BIT_DEPTH;
vec_s16 src_01, src_23;
vec_s32 res[4];
vec_s16 res_packed[2];
src_01 = vec_ld(0, coeffs);
src_23 = vec_ld(16, coeffs);
transform4x4(src_01, src_23, res, shift, coeffs);
src_01 = vec_packs(res[0], res[1]);
src_23 = vec_packs(res[2], res[3]);
scale(res, res_packed, shift);
src_01 = vec_perm(res_packed[0], res_packed[1], mask[0]);
src_23 = vec_perm(res_packed[0], res_packed[1], mask[1]);
transform4x4(src_01, src_23, res, shift2, coeffs);
scale(res, res_packed, shift2);
src_01 = vec_perm(res_packed[0], res_packed[1], mask[0]);
src_23 = vec_perm(res_packed[0], res_packed[1], mask[1]);
vec_st(src_01, 0, coeffs);
vec_st(src_23, 16, coeffs);
}
