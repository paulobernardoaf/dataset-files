#include "regenc.h"
#if defined(RUBY)
#include "encindex.h"
#endif
#if !defined(ENCINDEX_UTF_8)
#define ENCINDEX_UTF_8 0
#endif
#define USE_INVALID_CODE_SCHEME
#if defined(USE_INVALID_CODE_SCHEME)
#define INVALID_CODE_FE 0xfffffffe
#define INVALID_CODE_FF 0xffffffff
#endif
#define VALID_CODE_LIMIT 0x0010ffff
#define utf8_islead(c) ((UChar )((c) & 0xc0) != 0x80)
static const int EncLen_UTF8[] = {
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
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
4, 4, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
typedef enum {
FAILURE = -2,
ACCEPT,
S0, S1, S2, S3,
S4, S5, S6, S7
} state_t;
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
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3,
5, 6, 6, 6, 7, F, F, F, F, F, F, F, F, F, F, F
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
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
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
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
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
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
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
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
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
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
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
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
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
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F
},
};
#undef A
#undef F
static int
mbc_enc_len(const UChar* p, const UChar* e, OnigEncoding enc ARG_UNUSED)
{
int firstbyte = *p++;
state_t s;
s = trans[0][firstbyte];
if (s < 0) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(1) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_UTF8[firstbyte]-1);
s = trans[s][*p++];
if (s < 0) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(2) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_UTF8[firstbyte]-2);
s = trans[s][*p++];
if (s < 0) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(3) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_UTF8[firstbyte]-3);
s = trans[s][*p++];
return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(4) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
}
static int
is_mbc_newline(const UChar* p, const UChar* end, OnigEncoding enc)
{
if (p < end) {
if (*p == 0x0a) return 1;
#if defined(USE_UNICODE_ALL_LINE_TERMINATORS)
if (*p == 0x0b || *p == 0x0c || *p == 0x0d) return 1;
if (p + 1 < end) {
if (*(p+1) == 0x85 && *p == 0xc2) 
return 1;
if (p + 2 < end) {
if ((*(p+2) == 0xa8 || *(p+2) == 0xa9)
&& *(p+1) == 0x80 && *p == 0xe2) 
return 1;
}
}
#endif
}
return 0;
}
static OnigCodePoint
mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc)
{
int c, len;
OnigCodePoint n;
len = mbc_enc_len(p, end, enc);
c = *p++;
if (len > 1) {
len--;
n = c & ((1 << (6 - len)) - 1);
while (len--) {
c = *p++;
n = (n << 6) | (c & ((1 << 6) - 1));
}
return n;
}
else {
#if defined(USE_INVALID_CODE_SCHEME)
if (c > 0xfd) {
return ((c == 0xfe) ? INVALID_CODE_FE : INVALID_CODE_FF);
}
#endif
return (OnigCodePoint )c;
}
}
static int
code_to_mbclen(OnigCodePoint code, OnigEncoding enc ARG_UNUSED)
{
if ((code & 0xffffff80) == 0) return 1;
else if ((code & 0xfffff800) == 0) return 2;
else if ((code & 0xffff0000) == 0) return 3;
else if (code <= VALID_CODE_LIMIT) return 4;
#if defined(USE_INVALID_CODE_SCHEME)
else if (code == INVALID_CODE_FE) return 1;
else if (code == INVALID_CODE_FF) return 1;
#endif
else
return ONIGERR_TOO_BIG_WIDE_CHAR_VALUE;
}
static int
code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc ARG_UNUSED)
{
#define UTF8_TRAILS(code, shift) (UChar )((((code) >> (shift)) & 0x3f) | 0x80)
#define UTF8_TRAIL0(code) (UChar )(((code) & 0x3f) | 0x80)
if ((code & 0xffffff80) == 0) {
*buf = (UChar )code;
return 1;
}
else {
UChar *p = buf;
if ((code & 0xfffff800) == 0) {
*p++ = (UChar )(((code>>6)& 0x1f) | 0xc0);
}
else if ((code & 0xffff0000) == 0) {
*p++ = (UChar )(((code>>12) & 0x0f) | 0xe0);
*p++ = UTF8_TRAILS(code, 6);
}
else if (code <= VALID_CODE_LIMIT) {
*p++ = (UChar )(((code>>18) & 0x07) | 0xf0);
*p++ = UTF8_TRAILS(code, 12);
*p++ = UTF8_TRAILS(code, 6);
}
#if defined(USE_INVALID_CODE_SCHEME)
else if (code == INVALID_CODE_FE) {
*p = 0xfe;
return 1;
}
else if (code == INVALID_CODE_FF) {
*p = 0xff;
return 1;
}
#endif
else {
return ONIGERR_TOO_BIG_WIDE_CHAR_VALUE;
}
*p++ = UTF8_TRAIL0(code);
return (int )(p - buf);
}
}
static int
mbc_case_fold(OnigCaseFoldType flag, const UChar** pp,
const UChar* end, UChar* fold, OnigEncoding enc)
{
const UChar* p = *pp;
if (ONIGENC_IS_MBC_ASCII(p)) {
#if defined(USE_UNICODE_CASE_FOLD_TURKISH_AZERI)
if ((flag & ONIGENC_CASE_FOLD_TURKISH_AZERI) != 0) {
if (*p == 0x49) {
*fold++ = 0xc4;
*fold = 0xb1;
(*pp)++;
return 2;
}
}
#endif
*fold = ONIGENC_ASCII_CODE_TO_LOWER_CASE(*p);
(*pp)++;
return 1; 
}
else {
return onigenc_unicode_mbc_case_fold(enc, flag, pp, end, fold);
}
}
static int
get_ctype_code_range(OnigCtype ctype, OnigCodePoint *sb_out,
const OnigCodePoint* ranges[], OnigEncoding enc ARG_UNUSED)
{
*sb_out = 0x80;
return onigenc_unicode_ctype_code_range(ctype, ranges);
}
static UChar*
left_adjust_char_head(const UChar* start, const UChar* s, const UChar* end, OnigEncoding enc ARG_UNUSED)
{
const UChar *p;
if (s <= start) return (UChar* )s;
p = s;
while (!utf8_islead(*p) && p > start) p--;
return (UChar* )p;
}
static int
get_case_fold_codes_by_str(OnigCaseFoldType flag,
const OnigUChar* p, const OnigUChar* end, OnigCaseFoldCodeItem items[],
OnigEncoding enc)
{
return onigenc_unicode_get_case_fold_codes_by_str(enc, flag, p, end, items);
}
OnigEncodingDefine(utf_8, UTF_8) = {
mbc_enc_len,
"UTF-8", 
4, 
1, 
is_mbc_newline,
mbc_to_code,
code_to_mbclen,
code_to_mbc,
mbc_case_fold,
onigenc_unicode_apply_all_case_fold,
get_case_fold_codes_by_str,
onigenc_unicode_property_name_to_ctype,
onigenc_unicode_is_code_ctype,
get_ctype_code_range,
left_adjust_char_head,
onigenc_always_true_is_allowed_reverse_match,
onigenc_unicode_case_map,
ENCINDEX_UTF_8,
ONIGENC_FLAG_UNICODE,
};
ENC_ALIAS("CP65001", "UTF-8")
ENC_REPLICATE("UTF8-MAC", "UTF-8")
ENC_ALIAS("UTF-8-MAC", "UTF8-MAC")
ENC_ALIAS("UTF-8-HFS", "UTF8-MAC") 
