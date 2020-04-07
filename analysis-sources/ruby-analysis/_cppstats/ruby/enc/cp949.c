#include "regenc.h"
static const int EncLen_CP949[] = {
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
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};
static const char CP949_CAN_BE_TRAIL_TABLE[256] = {
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
#define CP949_ISMB_FIRST(byte) (EncLen_CP949[byte] > 1)
#define CP949_ISMB_TRAIL(byte) CP949_CAN_BE_TRAIL_TABLE[(byte)]
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
A, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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
F, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, F, F, F, F, F,
F, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, F, F, F, F, F,
F, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
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
cp949_mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
int firstbyte = *p++;
state_t s = trans[0][firstbyte];
#define RETURN(n) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(n) : ONIGENC_CONSTRUCT_MBCLEN_INVALID()
if (s < 0) RETURN(1);
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_CP949[firstbyte]-1);
s = trans[s][*p++];
RETURN(2);
#undef RETURN
}
static OnigCodePoint
cp949_mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_mbc_to_code(enc, p, end);
}
static int
cp949_code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc)
{
return onigenc_mb2_code_to_mbc(enc, code, buf);
}
static int
cp949_mbc_case_fold(OnigCaseFoldType flag, const UChar** pp, const UChar* end,
UChar* lower, OnigEncoding enc)
{
return onigenc_mbn_mbc_case_fold(enc, flag,
pp, end, lower);
}
#if 0
static int
cp949_is_mbc_ambiguous(OnigCaseFoldType flag,
const UChar** pp, const UChar* end, OnigEncoding enc)
{
return onigenc_mbn_is_mbc_ambiguous(enc, flag, pp, end);
}
#endif
static int
cp949_is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc)
{
return onigenc_mb2_is_code_ctype(enc, code, ctype);
}
static UChar*
cp949_left_adjust_char_head(const UChar* start, const UChar* s, const UChar* end, OnigEncoding enc)
{
const UChar *p;
int len;
if (s <= start) return (UChar* )s;
p = s;
if (CP949_ISMB_TRAIL(*p)) {
while (p > start) {
if (! CP949_ISMB_FIRST(*--p)) {
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
cp949_is_allowed_reverse_match(const UChar* s, const UChar* end ARG_UNUSED, OnigEncoding enc ARG_UNUSED)
{
const UChar c = *s;
return (CP949_ISMB_TRAIL(c) ? FALSE : TRUE);
}
OnigEncodingDefine(cp949, CP949) = {
cp949_mbc_enc_len,
"CP949", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
cp949_mbc_to_code,
onigenc_mb2_code_to_mbclen,
cp949_code_to_mbc,
cp949_mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
cp949_is_code_ctype,
onigenc_not_support_get_ctype_code_range,
cp949_left_adjust_char_head,
cp949_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
