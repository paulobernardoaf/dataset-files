#include "regenc.h"
static const int EncLen_EUCKR[] = {
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
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, F
}
};
#undef A
#undef F
static int
euckr_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
int firstbyte = *p++;
state_t s = trans[0][firstbyte];
#define RETURN(n) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(n) : ONIGENC_CONSTRUCT_MBCLEN_INVALID()
if (s < 0) RETURN(1);
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_EUCKR[firstbyte]-1);
s = trans[s][*p++];
RETURN(2);
#undef RETURN
}
static OnigCodePoint
euckr_mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_mbc_to_code(enc, p, end);
}
static int
euckr_code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc)
{
return onigenc_mb2_code_to_mbc(enc, code, buf);
}
static int
euckr_mbc_case_fold(OnigCaseFoldType flag, const UChar** pp, const UChar* end,
UChar* lower, OnigEncoding enc)
{
return onigenc_mbn_mbc_case_fold(enc, flag,
pp, end, lower);
}
#if 0
static int
euckr_is_mbc_ambiguous(OnigCaseFoldType flag,
const UChar** pp, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_is_mbc_ambiguous(enc, flag, pp, end);
}
#endif
static int
euckr_is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc)
{
return onigenc_mb2_is_code_ctype(enc, code, ctype);
}
#define euckr_islead(c) ((c) < 0xa1 || (c) == 0xff)
static UChar*
euckr_left_adjust_char_head(const UChar* start, const UChar* s, const UChar* end, OnigEncoding enc)
{
const UChar *p;
int len;
if (s <= start) return (UChar* )s;
p = s;
while (!euckr_islead(*p) && p > start) p--;
len = enclen(enc, p, end);
if (p + len > s) return (UChar* )p;
p += len;
return (UChar* )(p + ((s - p) & ~1));
}
static int
euckr_is_allowed_reverse_match(const UChar* s, const UChar* end ARG_UNUSED, OnigEncoding enc ARG_UNUSED)
{
const UChar c = *s;
if (c <= 0x7e) return TRUE;
else return FALSE;
}
OnigEncodingDefine(euc_kr, EUC_KR) = {
euckr_mbc_enc_len,
"EUC-KR", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
euckr_mbc_to_code,
onigenc_mb2_code_to_mbclen,
euckr_code_to_mbc,
euckr_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
euckr_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
euckr_left_adjust_char_head,
euckr_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
ENC_ALIAS("eucKR", "EUC-KR")
#if !defined(RUBY)
OnigEncodingDefine(euc_cn, EUC_CN) = {
euckr_mbc_enc_len,
"EUC-CN", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
euckr_mbc_to_code,
onigenc_mb2_code_to_mbclen,
euckr_code_to_mbc,
euckr_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
euckr_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
euckr_left_adjust_char_head,
euckr_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
#endif 
