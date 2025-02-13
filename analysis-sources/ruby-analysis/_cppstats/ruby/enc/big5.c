#include "regenc.h"
static const int EncLen_BIG5[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};
static const int EncLen_BIG5_HKSCS[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
};
static const int EncLen_BIG5_UAO[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
};
typedef enum { FAILURE = -2, ACCEPT = -1, S0 = 0, S1 } state_t;
#define A ACCEPT
#define F FAILURE
static const signed char trans[][0x100] = {
{ 
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, F
},
{ 
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, F
},
{ 
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
F, F, F, F, F, F, F, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, F
}
};
#undef A
#undef F
static int
big5_mbc_enc_len0(const UChar* p, const UChar* e, int tridx, const int tbl[])
{
int firstbyte = *p++;
state_t s = trans[tridx][firstbyte];
#define RETURN(n) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(n) : ONIGENC_CONSTRUCT_MBCLEN_INVALID()
if (s < 0) RETURN(1);
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(tbl[firstbyte]-1);
s = trans[s][*p++];
RETURN(2);
#undef RETURN
}
static int
big5_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
return big5_mbc_enc_len0(p, e, 0, EncLen_BIG5);
}
static int
big5_hkscs_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
return big5_mbc_enc_len0(p, e, 2, EncLen_BIG5_HKSCS);
}
static int
big5_uao_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
return big5_mbc_enc_len0(p, e, 2, EncLen_BIG5_UAO);
}
static OnigCodePoint
big5_mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_mbc_to_code(enc, p, end);
}
static int
big5_code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc)
{
return onigenc_mb2_code_to_mbc(enc, code, buf);
}
static int
big5_mbc_case_fold(OnigCaseFoldType flag, const UChar** pp, const UChar* end,
UChar* lower, OnigEncoding enc)
{
return onigenc_mbn_mbc_case_fold(enc, flag,
pp, end, lower);
}
#if 0
static int
big5_is_mbc_ambiguous(OnigCaseFoldType flag,
const UChar** pp, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_is_mbc_ambiguous(enc, flag, pp, end);
}
#endif
static int
big5_is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc)
{
return onigenc_mb2_is_code_ctype(enc, code, ctype);
}
static const char BIG5_CAN_BE_TRAIL_TABLE[256] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
};
#define BIG5_HKSCS_P(enc) ((enc)->precise_mbc_enc_len == big5_hkscs_mbc_enc_len)
#define BIG5_UAO_P(enc) ((enc)->precise_mbc_enc_len == big5_uao_mbc_enc_len)
#define BIG5_ISMB_FIRST(byte) ( BIG5_HKSCS_P(enc) ? EncLen_BIG5_HKSCS[byte] > 1 : EncLen_BIG5[byte] > 1 )
#define BIG5_ISMB_TRAIL(byte) BIG5_CAN_BE_TRAIL_TABLE[(byte)]
static UChar*
big5_left_adjust_char_head(const UChar* start, const UChar* s, const UChar* end, OnigEncoding enc)
{
const UChar *p;
int len;
if (s <= start) return (UChar* )s;
p = s;
if (BIG5_ISMB_TRAIL(*p)) {
while (p > start) {
if (! BIG5_ISMB_FIRST(*--p)) {
p++;
break;
}
}
}
len = enclen(enc, p, end);
if (p + len > s) return (UChar* )p;
p += len;
return (UChar* )(p + ((s - p) & ~1));
}
static int
big5_is_allowed_reverse_match(const UChar* s, const UChar* end ARG_UNUSED, OnigEncoding enc ARG_UNUSED)
{
const UChar c = *s;
return (BIG5_ISMB_TRAIL(c) ? FALSE : TRUE);
}
OnigEncodingDefine(big5, BIG5) = {
big5_mbc_enc_len,
"Big5", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
big5_mbc_to_code,
onigenc_mb2_code_to_mbclen,
big5_code_to_mbc,
big5_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
big5_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
big5_left_adjust_char_head,
big5_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
ENC_REPLICATE("CP950", "Big5")
OnigEncodingDefine(big5_hkscs, BIG5_HKSCS) = {
big5_hkscs_mbc_enc_len,
"Big5-HKSCS", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
big5_mbc_to_code,
onigenc_mb2_code_to_mbclen,
big5_code_to_mbc,
big5_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
big5_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
big5_left_adjust_char_head,
big5_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
ENC_ALIAS("Big5-HKSCS:2008", "Big5-HKSCS")
ENC_REPLICATE("CP951", "Big5-HKSCS")
OnigEncodingDefine(big5_uao, BIG5_UAO) = {
big5_uao_mbc_enc_len,
"Big5-UAO", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
big5_mbc_to_code,
onigenc_mb2_code_to_mbclen,
big5_code_to_mbc,
big5_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
big5_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
big5_left_adjust_char_head,
big5_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
