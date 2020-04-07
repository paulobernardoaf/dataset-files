#include "shift_jis.h"
OnigEncodingDefine(shift_jis, Shift_JIS) = {
mbc_enc_len,
"Shift_JIS", 
2, 
1, 
onigenc_is_mbc_newline_0x0a,
mbc_to_code,
code_to_mbclen,
code_to_mbc,
mbc_case_fold,
apply_all_case_fold,
get_case_fold_codes_by_str,
property_name_to_ctype,
is_code_ctype,
get_ctype_code_range,
left_adjust_char_head,
is_allowed_reverse_match,
onigenc_ascii_only_case_map,
0,
ONIGENC_FLAG_NONE,
};
ENC_REPLICATE("MacJapanese", "Shift_JIS")
ENC_ALIAS("MacJapan", "MacJapanese")
