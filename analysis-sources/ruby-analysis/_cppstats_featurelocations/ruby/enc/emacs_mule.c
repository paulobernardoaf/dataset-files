




























#include "regenc.h"


#define emacsmule_islead(c) ((UChar )(c) < 0x9e)















static const int EncLen_EmacsMule[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

typedef enum { FAILURE = -2, ACCEPT = -1, S0 = 0, S1, S2, S3, S4, S5, S6 } state_t;
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
F, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 5, 6, F, F,
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
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,
A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A
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
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
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
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
2, 2, 2, 2, 2, F, F, F, F, F, F, F, F, F, F, F
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
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,
F, F, F, F, F, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, F
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
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_EmacsMule[firstbyte]-1);
s = trans[s][*p++];
if (s < 0) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(2) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_EmacsMule[firstbyte]-2);
s = trans[s][*p++];
if (s < 0) return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(3) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
if (p == e) return ONIGENC_CONSTRUCT_MBCLEN_NEEDMORE(EncLen_EmacsMule[firstbyte]-3);
s = trans[s][*p++];
return s == ACCEPT ? ONIGENC_CONSTRUCT_MBCLEN_CHARFOUND(4) :
ONIGENC_CONSTRUCT_MBCLEN_INVALID();
}

static OnigCodePoint
mbc_to_code(const UChar* p, const UChar* end, OnigEncoding enc)
{
int c, i, len;
OnigCodePoint n;

len = enclen(enc, p, end);
n = (OnigCodePoint )*p++;
if (len == 1) return n;

for (i = 1; i < len; i++) {
if (p >= end) break;
c = *p++;
n <<= 8; n += c;
}
return n;
}

static int
code_to_mbclen(OnigCodePoint code, OnigEncoding enc ARG_UNUSED)
{
if (ONIGENC_IS_CODE_ASCII(code)) return 1;
else if (code > 0xffffffff) return 0;
else if ((code & 0xff000000) >= 0x80000000) return 4;
else if ((code & 0xff0000) >= 0x800000) return 3;
else if ((code & 0xff00) >= 0x8000) return 2;
else
return ONIGERR_INVALID_CODE_POINT_VALUE;
}

static int
code_to_mbc(OnigCodePoint code, UChar *buf, OnigEncoding enc)
{
UChar *p = buf;

if ((code & 0xff000000) != 0) *p++ = (UChar )(((code >> 24) & 0xff));
if ((code & 0xff0000) != 0) *p++ = (UChar )(((code >> 16) & 0xff));
if ((code & 0xff00) != 0) *p++ = (UChar )(((code >> 8) & 0xff));
*p++ = (UChar )(code & 0xff);

if (enclen(enc, buf, p) != (p - buf))
return ONIGERR_INVALID_CODE_POINT_VALUE;
return (int)(p - buf);
}

static int
mbc_case_fold(OnigCaseFoldType flag,
const UChar** pp, const UChar* end, UChar* lower,
OnigEncoding enc)
{
int len;
const UChar* p = *pp;

if (ONIGENC_IS_MBC_ASCII(p)) {
*lower = ONIGENC_ASCII_CODE_TO_LOWER_CASE(*p);
(*pp)++;
return 1;
}
else {
int i;

len = mbc_enc_len(p, end, enc);
for (i = 0; i < len; i++) {
*lower++ = *p++;
}
(*pp) += len;
return len; 
}
}

static UChar*
left_adjust_char_head(const UChar* start, const UChar* s, const UChar* end, OnigEncoding enc)
{
const UChar *p;

if (s <= start) return (UChar* )s;
p = s;

while (!emacsmule_islead(*p) && p > start) p--;
return (UChar* )p;
}

static int
is_code_ctype(OnigCodePoint code, unsigned int ctype, OnigEncoding enc ARG_UNUSED)
{
if (code < 128)
return ONIGENC_IS_ASCII_CODE_CTYPE(code, ctype);
else
return (code_to_mbclen(code, enc) > 1 ? TRUE : FALSE);
}





OnigEncodingDefine(emacs_mule, Emacs_Mule) = {
mbc_enc_len,
"Emacs-Mule", 
4, 
1, 
onigenc_is_mbc_newline_0x0a,
mbc_to_code,
code_to_mbclen,
code_to_mbc,
mbc_case_fold,
onigenc_ascii_apply_all_case_fold,
onigenc_ascii_get_case_fold_codes_by_str,
onigenc_minimum_property_name_to_ctype,
is_code_ctype,
onigenc_not_support_get_ctype_code_range,
left_adjust_char_head,
onigenc_always_true_is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};

ENC_REPLICATE("stateless-ISO-2022-JP", "Emacs-Mule")
