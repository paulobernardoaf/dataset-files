#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include "nvim/lib/kvec.h"
#include "nvim/eval/typval.h"
#include "nvim/eval/encode.h"
#include "nvim/func_attr.h"
#include "nvim/eval/typval_encode.h"
const dict_T *const TYPVAL_ENCODE_NODICT_VAR = NULL;
static inline int _TYPVAL_ENCODE_CHECK_SELF_REFERENCE(
TYPVAL_ENCODE_FIRST_ARG_TYPE TYPVAL_ENCODE_FIRST_ARG_NAME,
void *const val, int *const val_copyID,
const MPConvStack *const mpstack, const int copyID,
const MPConvStackValType conv_type,
const char *const objname)
REAL_FATTR_NONNULL_ARG(2, 3, 4, 7) REAL_FATTR_WARN_UNUSED_RESULT
REAL_FATTR_ALWAYS_INLINE;
static inline int _TYPVAL_ENCODE_CHECK_SELF_REFERENCE(
TYPVAL_ENCODE_FIRST_ARG_TYPE TYPVAL_ENCODE_FIRST_ARG_NAME,
void *const val, int *const val_copyID,
const MPConvStack *const mpstack, const int copyID,
const MPConvStackValType conv_type,
const char *const objname)
{
if (*val_copyID == copyID) {
TYPVAL_ENCODE_CONV_RECURSE(val, conv_type);
return OK;
}
*val_copyID = copyID;
return NOTDONE;
}
static int _TYPVAL_ENCODE_CONVERT_ONE_VALUE(
TYPVAL_ENCODE_FIRST_ARG_TYPE TYPVAL_ENCODE_FIRST_ARG_NAME,
MPConvStack *const mpstack, MPConvStackVal *const cur_mpsv,
typval_T *const tv, const int copyID,
const char *const objname)
REAL_FATTR_NONNULL_ARG(2, 4, 6) REAL_FATTR_WARN_UNUSED_RESULT;
static int _TYPVAL_ENCODE_CONVERT_ONE_VALUE(
TYPVAL_ENCODE_FIRST_ARG_TYPE TYPVAL_ENCODE_FIRST_ARG_NAME,
MPConvStack *const mpstack, MPConvStackVal *const cur_mpsv,
typval_T *const tv, const int copyID,
const char *const objname)
{
switch (tv->v_type) {
case VAR_STRING: {
TYPVAL_ENCODE_CONV_STRING(tv, tv->vval.v_string, tv_strlen(tv));
break;
}
case VAR_NUMBER: {
TYPVAL_ENCODE_CONV_NUMBER(tv, tv->vval.v_number);
break;
}
case VAR_FLOAT: {
TYPVAL_ENCODE_CONV_FLOAT(tv, tv->vval.v_float);
break;
}
case VAR_FUNC: {
TYPVAL_ENCODE_CONV_FUNC_START(tv, tv->vval.v_string);
TYPVAL_ENCODE_CONV_FUNC_BEFORE_ARGS(tv, 0);
TYPVAL_ENCODE_CONV_FUNC_BEFORE_SELF(tv, -1);
TYPVAL_ENCODE_CONV_FUNC_END(tv);
break;
}
case VAR_PARTIAL: {
partial_T *const pt = tv->vval.v_partial;
(void)pt;
TYPVAL_ENCODE_CONV_FUNC_START( 
tv, (pt == NULL ? NULL : partial_name(pt)));
_mp_push(*mpstack, ((MPConvStackVal) { 
.type = kMPConvPartial,
.tv = tv,
.saved_copyID = copyID - 1,
.data = {
.p = {
.stage = kMPConvPartialArgs,
.pt = tv->vval.v_partial,
},
},
}));
break;
}
case VAR_LIST: {
if (tv->vval.v_list == NULL || tv_list_len(tv->vval.v_list) == 0) {
TYPVAL_ENCODE_CONV_EMPTY_LIST(tv);
break;
}
const int saved_copyID = tv_list_copyid(tv->vval.v_list);
_TYPVAL_ENCODE_DO_CHECK_SELF_REFERENCE(tv->vval.v_list, lv_copyID, copyID,
kMPConvList);
TYPVAL_ENCODE_CONV_LIST_START(tv, tv_list_len(tv->vval.v_list));
assert(saved_copyID != copyID && saved_copyID != copyID - 1);
_mp_push(*mpstack, ((MPConvStackVal) {
.type = kMPConvList,
.tv = tv,
.saved_copyID = saved_copyID,
.data = {
.l = {
.list = tv->vval.v_list,
.li = tv_list_first(tv->vval.v_list),
},
},
}));
TYPVAL_ENCODE_CONV_REAL_LIST_AFTER_START(tv, _mp_last(*mpstack));
break;
}
case VAR_SPECIAL: {
switch (tv->vval.v_special) {
case kSpecialVarNull: {
TYPVAL_ENCODE_CONV_NIL(tv); 
break;
}
case kSpecialVarTrue:
case kSpecialVarFalse: {
TYPVAL_ENCODE_CONV_BOOL(tv, tv->vval.v_special == kSpecialVarTrue);
break;
}
}
break;
}
case VAR_DICT: {
if (tv->vval.v_dict == NULL
|| tv->vval.v_dict->dv_hashtab.ht_used == 0) {
TYPVAL_ENCODE_CONV_EMPTY_DICT(tv, tv->vval.v_dict);
break;
}
const dictitem_T *type_di;
const dictitem_T *val_di;
if (TYPVAL_ENCODE_ALLOW_SPECIALS
&& tv->vval.v_dict->dv_hashtab.ht_used == 2
&& (type_di = tv_dict_find((dict_T *)tv->vval.v_dict,
S_LEN("_TYPE"))) != NULL
&& type_di->di_tv.v_type == VAR_LIST
&& (val_di = tv_dict_find((dict_T *)tv->vval.v_dict,
S_LEN("_VAL"))) != NULL) {
size_t i;
for (i = 0; i < ARRAY_SIZE(eval_msgpack_type_lists); i++) {
if (type_di->di_tv.vval.v_list == eval_msgpack_type_lists[i]) {
break;
}
}
if (i == ARRAY_SIZE(eval_msgpack_type_lists)) {
goto _convert_one_value_regular_dict;
}
switch ((MessagePackType)i) {
case kMPNil: {
TYPVAL_ENCODE_CONV_NIL(tv);
break;
}
case kMPBoolean: {
if (val_di->di_tv.v_type != VAR_NUMBER) {
goto _convert_one_value_regular_dict;
}
TYPVAL_ENCODE_CONV_BOOL(tv, val_di->di_tv.vval.v_number);
break;
}
case kMPInteger: {
const list_T *val_list;
varnumber_T sign;
varnumber_T highest_bits;
varnumber_T high_bits;
varnumber_T low_bits;
if (val_di->di_tv.v_type != VAR_LIST
|| tv_list_len(val_list = val_di->di_tv.vval.v_list) != 4) {
goto _convert_one_value_regular_dict;
}
const listitem_T *const sign_li = tv_list_first(val_list);
if (TV_LIST_ITEM_TV(sign_li)->v_type != VAR_NUMBER
|| (sign = TV_LIST_ITEM_TV(sign_li)->vval.v_number) == 0) {
goto _convert_one_value_regular_dict;
}
const listitem_T *const highest_bits_li = (
TV_LIST_ITEM_NEXT(val_list, sign_li));
if (TV_LIST_ITEM_TV(highest_bits_li)->v_type != VAR_NUMBER
|| ((highest_bits
= TV_LIST_ITEM_TV(highest_bits_li)->vval.v_number)
< 0)) {
goto _convert_one_value_regular_dict;
}
const listitem_T *const high_bits_li = (
TV_LIST_ITEM_NEXT(val_list, highest_bits_li));
if (TV_LIST_ITEM_TV(high_bits_li)->v_type != VAR_NUMBER
|| ((high_bits = TV_LIST_ITEM_TV(high_bits_li)->vval.v_number)
< 0)) {
goto _convert_one_value_regular_dict;
}
const listitem_T *const low_bits_li = tv_list_last(val_list);
if (TV_LIST_ITEM_TV(low_bits_li)->v_type != VAR_NUMBER
|| ((low_bits = TV_LIST_ITEM_TV(low_bits_li)->vval.v_number)
< 0)) {
goto _convert_one_value_regular_dict;
}
const uint64_t number = ((uint64_t)(((uint64_t)highest_bits) << 62)
| (uint64_t)(((uint64_t)high_bits) << 31)
| (uint64_t)low_bits);
if (sign > 0) {
TYPVAL_ENCODE_CONV_UNSIGNED_NUMBER(tv, number);
} else {
TYPVAL_ENCODE_CONV_NUMBER(tv, -number);
}
break;
}
case kMPFloat: {
if (val_di->di_tv.v_type != VAR_FLOAT) {
goto _convert_one_value_regular_dict;
}
TYPVAL_ENCODE_CONV_FLOAT(tv, val_di->di_tv.vval.v_float);
break;
}
case kMPString:
case kMPBinary: {
const bool is_string = ((MessagePackType)i == kMPString);
if (val_di->di_tv.v_type != VAR_LIST) {
goto _convert_one_value_regular_dict;
}
size_t len;
char *buf;
if (!encode_vim_list_to_buf(val_di->di_tv.vval.v_list, &len,
&buf)) {
goto _convert_one_value_regular_dict;
}
if (is_string) {
TYPVAL_ENCODE_CONV_STR_STRING(tv, buf, len);
} else { 
TYPVAL_ENCODE_CONV_STRING(tv, buf, len);
}
xfree(buf);
break;
}
case kMPArray: {
if (val_di->di_tv.v_type != VAR_LIST) {
goto _convert_one_value_regular_dict;
}
const int saved_copyID = tv_list_copyid(val_di->di_tv.vval.v_list);
_TYPVAL_ENCODE_DO_CHECK_SELF_REFERENCE(val_di->di_tv.vval.v_list,
lv_copyID, copyID,
kMPConvList);
TYPVAL_ENCODE_CONV_LIST_START(
tv, tv_list_len(val_di->di_tv.vval.v_list));
assert(saved_copyID != copyID && saved_copyID != copyID - 1);
_mp_push(*mpstack, ((MPConvStackVal) {
.tv = tv,
.type = kMPConvList,
.saved_copyID = saved_copyID,
.data = {
.l = {
.list = val_di->di_tv.vval.v_list,
.li = tv_list_first(val_di->di_tv.vval.v_list),
},
},
}));
break;
}
case kMPMap: {
if (val_di->di_tv.v_type != VAR_LIST) {
goto _convert_one_value_regular_dict;
}
list_T *const val_list = val_di->di_tv.vval.v_list;
if (val_list == NULL || tv_list_len(val_list) == 0) {
TYPVAL_ENCODE_CONV_EMPTY_DICT( 
tv, TYPVAL_ENCODE_NODICT_VAR);
break;
}
TV_LIST_ITER_CONST(val_list, li, {
if (TV_LIST_ITEM_TV(li)->v_type != VAR_LIST
|| tv_list_len(TV_LIST_ITEM_TV(li)->vval.v_list) != 2) {
goto _convert_one_value_regular_dict;
}
});
const int saved_copyID = tv_list_copyid(val_di->di_tv.vval.v_list);
_TYPVAL_ENCODE_DO_CHECK_SELF_REFERENCE(val_list, lv_copyID, copyID,
kMPConvPairs);
TYPVAL_ENCODE_CONV_DICT_START(tv, TYPVAL_ENCODE_NODICT_VAR,
tv_list_len(val_list));
assert(saved_copyID != copyID && saved_copyID != copyID - 1);
_mp_push(*mpstack, ((MPConvStackVal) {
.tv = tv,
.type = kMPConvPairs,
.saved_copyID = saved_copyID,
.data = {
.l = {
.list = val_list,
.li = tv_list_first(val_list),
},
},
}));
break;
}
case kMPExt: {
const list_T *val_list;
varnumber_T type;
if (val_di->di_tv.v_type != VAR_LIST
|| tv_list_len((val_list = val_di->di_tv.vval.v_list)) != 2
|| (TV_LIST_ITEM_TV(tv_list_first(val_list))->v_type
!= VAR_NUMBER)
|| ((type
= TV_LIST_ITEM_TV(tv_list_first(val_list))->vval.v_number)
> INT8_MAX)
|| type < INT8_MIN
|| (TV_LIST_ITEM_TV(tv_list_last(val_list))->v_type
!= VAR_LIST)) {
goto _convert_one_value_regular_dict;
}
size_t len;
char *buf;
if (!(
encode_vim_list_to_buf(
TV_LIST_ITEM_TV(tv_list_last(val_list))->vval.v_list, &len,
&buf))) {
goto _convert_one_value_regular_dict;
}
TYPVAL_ENCODE_CONV_EXT_STRING(tv, buf, len, type);
xfree(buf);
break;
}
}
break;
}
_convert_one_value_regular_dict: {}
const int saved_copyID = tv->vval.v_dict->dv_copyID;
_TYPVAL_ENCODE_DO_CHECK_SELF_REFERENCE(tv->vval.v_dict, dv_copyID, copyID,
kMPConvDict);
TYPVAL_ENCODE_CONV_DICT_START(tv, tv->vval.v_dict,
tv->vval.v_dict->dv_hashtab.ht_used);
assert(saved_copyID != copyID && saved_copyID != copyID - 1);
_mp_push(*mpstack, ((MPConvStackVal) {
.tv = tv,
.type = kMPConvDict,
.saved_copyID = saved_copyID,
.data = {
.d = {
.dict = tv->vval.v_dict,
.dictp = &tv->vval.v_dict,
.hi = tv->vval.v_dict->dv_hashtab.ht_array,
.todo = tv->vval.v_dict->dv_hashtab.ht_used,
},
},
}));
TYPVAL_ENCODE_CONV_REAL_DICT_AFTER_START(tv, tv->vval.v_dict,
_mp_last(*mpstack));
break;
}
case VAR_UNKNOWN: {
internal_error(STR(_TYPVAL_ENCODE_CONVERT_ONE_VALUE) "()");
return FAIL;
}
}
typval_encode_stop_converting_one_item:
return OK;
goto typval_encode_stop_converting_one_item; 
}
TYPVAL_ENCODE_SCOPE int _TYPVAL_ENCODE_ENCODE(
TYPVAL_ENCODE_FIRST_ARG_TYPE TYPVAL_ENCODE_FIRST_ARG_NAME,
typval_T *const tv, const char *const objname)
REAL_FATTR_NONNULL_ARG(2, 3) REAL_FATTR_WARN_UNUSED_RESULT;
TYPVAL_ENCODE_SCOPE int _TYPVAL_ENCODE_ENCODE(
TYPVAL_ENCODE_FIRST_ARG_TYPE TYPVAL_ENCODE_FIRST_ARG_NAME,
typval_T *const top_tv, const char *const objname)
{
const int copyID = get_copyID();
MPConvStack mpstack;
_mp_init(mpstack);
if (_TYPVAL_ENCODE_CONVERT_ONE_VALUE(TYPVAL_ENCODE_FIRST_ARG_NAME, &mpstack,
NULL,
top_tv, copyID, objname)
== FAIL) {
goto encode_vim_to__error_ret;
}
typval_encode_stop_converting_one_item:
while (_mp_size(mpstack)) {
MPConvStackVal *cur_mpsv = &_mp_last(mpstack);
typval_T *tv = NULL;
switch (cur_mpsv->type) {
case kMPConvDict: {
if (!cur_mpsv->data.d.todo) {
(void)_mp_pop(mpstack);
cur_mpsv->data.d.dict->dv_copyID = cur_mpsv->saved_copyID;
TYPVAL_ENCODE_CONV_DICT_END(cur_mpsv->tv, *cur_mpsv->data.d.dictp);
continue;
} else if (cur_mpsv->data.d.todo
!= cur_mpsv->data.d.dict->dv_hashtab.ht_used) {
TYPVAL_ENCODE_CONV_DICT_BETWEEN_ITEMS(cur_mpsv->tv,
*cur_mpsv->data.d.dictp);
}
while (HASHITEM_EMPTY(cur_mpsv->data.d.hi)) {
cur_mpsv->data.d.hi++;
}
dictitem_T *const di = TV_DICT_HI2DI(cur_mpsv->data.d.hi);
cur_mpsv->data.d.todo--;
cur_mpsv->data.d.hi++;
TYPVAL_ENCODE_CONV_STR_STRING(NULL, &di->di_key[0],
strlen((char *)&di->di_key[0]));
TYPVAL_ENCODE_CONV_DICT_AFTER_KEY(cur_mpsv->tv,
*cur_mpsv->data.d.dictp);
tv = &di->di_tv;
break;
}
case kMPConvList: {
if (cur_mpsv->data.l.li == NULL) {
(void)_mp_pop(mpstack);
tv_list_set_copyid(cur_mpsv->data.l.list, cur_mpsv->saved_copyID);
TYPVAL_ENCODE_CONV_LIST_END(cur_mpsv->tv);
continue;
} else if (cur_mpsv->data.l.li
!= tv_list_first(cur_mpsv->data.l.list)) {
TYPVAL_ENCODE_CONV_LIST_BETWEEN_ITEMS(cur_mpsv->tv);
}
tv = TV_LIST_ITEM_TV(cur_mpsv->data.l.li);
cur_mpsv->data.l.li = TV_LIST_ITEM_NEXT(cur_mpsv->data.l.list,
cur_mpsv->data.l.li);
break;
}
case kMPConvPairs: {
if (cur_mpsv->data.l.li == NULL) {
(void)_mp_pop(mpstack);
tv_list_set_copyid(cur_mpsv->data.l.list, cur_mpsv->saved_copyID);
TYPVAL_ENCODE_CONV_DICT_END(cur_mpsv->tv, TYPVAL_ENCODE_NODICT_VAR);
continue;
} else if (cur_mpsv->data.l.li
!= tv_list_first(cur_mpsv->data.l.list)) {
TYPVAL_ENCODE_CONV_DICT_BETWEEN_ITEMS(
cur_mpsv->tv, TYPVAL_ENCODE_NODICT_VAR);
}
const list_T *const kv_pair = (
TV_LIST_ITEM_TV(cur_mpsv->data.l.li)->vval.v_list);
TYPVAL_ENCODE_SPECIAL_DICT_KEY_CHECK(
encode_vim_to__error_ret, *TV_LIST_ITEM_TV(tv_list_first(kv_pair)));
if (
_TYPVAL_ENCODE_CONVERT_ONE_VALUE(
TYPVAL_ENCODE_FIRST_ARG_NAME, &mpstack, cur_mpsv,
TV_LIST_ITEM_TV(tv_list_first(kv_pair)), copyID, objname)
== FAIL) {
goto encode_vim_to__error_ret;
}
TYPVAL_ENCODE_CONV_DICT_AFTER_KEY(cur_mpsv->tv,
TYPVAL_ENCODE_NODICT_VAR);
tv = TV_LIST_ITEM_TV(tv_list_last(kv_pair));
cur_mpsv->data.l.li = TV_LIST_ITEM_NEXT(cur_mpsv->data.l.list,
cur_mpsv->data.l.li);
break;
}
case kMPConvPartial: {
partial_T *const pt = cur_mpsv->data.p.pt;
tv = cur_mpsv->tv;
(void)tv;
switch (cur_mpsv->data.p.stage) {
case kMPConvPartialArgs: {
TYPVAL_ENCODE_CONV_FUNC_BEFORE_ARGS(tv,
pt == NULL ? 0 : pt->pt_argc);
cur_mpsv->data.p.stage = kMPConvPartialSelf;
if (pt != NULL && pt->pt_argc > 0) {
TYPVAL_ENCODE_CONV_LIST_START(NULL, pt->pt_argc);
_mp_push(mpstack, ((MPConvStackVal) {
.type = kMPConvPartialList,
.tv = NULL,
.saved_copyID = copyID - 1,
.data = {
.a = {
.arg = pt->pt_argv,
.argv = pt->pt_argv,
.todo = (size_t)pt->pt_argc,
},
},
}));
}
break;
}
case kMPConvPartialSelf: {
cur_mpsv->data.p.stage = kMPConvPartialEnd;
dict_T *const dict = pt == NULL ? NULL : pt->pt_dict;
if (dict != NULL) {
TYPVAL_ENCODE_CONV_FUNC_BEFORE_SELF(tv, dict->dv_hashtab.ht_used);
if (dict->dv_hashtab.ht_used == 0) {
TYPVAL_ENCODE_CONV_EMPTY_DICT(NULL, pt->pt_dict);
continue;
}
const int saved_copyID = dict->dv_copyID;
const int te_csr_ret = _TYPVAL_ENCODE_CHECK_SELF_REFERENCE(
TYPVAL_ENCODE_FIRST_ARG_NAME,
dict, &dict->dv_copyID, &mpstack, copyID, kMPConvDict,
objname);
if (te_csr_ret != NOTDONE) {
if (te_csr_ret == FAIL) {
goto encode_vim_to__error_ret;
} else {
continue;
}
}
TYPVAL_ENCODE_CONV_DICT_START(NULL, pt->pt_dict,
dict->dv_hashtab.ht_used);
assert(saved_copyID != copyID && saved_copyID != copyID - 1);
_mp_push(mpstack, ((MPConvStackVal) {
.type = kMPConvDict,
.tv = NULL,
.saved_copyID = saved_copyID,
.data = {
.d = {
.dict = dict,
.dictp = &pt->pt_dict,
.hi = dict->dv_hashtab.ht_array,
.todo = dict->dv_hashtab.ht_used,
},
},
}));
TYPVAL_ENCODE_CONV_REAL_DICT_AFTER_START(NULL, pt->pt_dict,
_mp_last(mpstack));
} else {
TYPVAL_ENCODE_CONV_FUNC_BEFORE_SELF(tv, -1);
}
break;
}
case kMPConvPartialEnd: {
TYPVAL_ENCODE_CONV_FUNC_END(tv);
(void)_mp_pop(mpstack);
break;
}
}
continue;
}
case kMPConvPartialList: {
if (!cur_mpsv->data.a.todo) {
(void)_mp_pop(mpstack);
TYPVAL_ENCODE_CONV_LIST_END(NULL);
continue;
} else if (cur_mpsv->data.a.argv != cur_mpsv->data.a.arg) {
TYPVAL_ENCODE_CONV_LIST_BETWEEN_ITEMS(NULL);
}
tv = cur_mpsv->data.a.arg++;
cur_mpsv->data.a.todo--;
break;
}
}
assert(tv != NULL);
if (_TYPVAL_ENCODE_CONVERT_ONE_VALUE(TYPVAL_ENCODE_FIRST_ARG_NAME, &mpstack,
cur_mpsv, tv, copyID, objname)
== FAIL) {
goto encode_vim_to__error_ret;
}
}
_mp_destroy(mpstack);
return OK;
encode_vim_to__error_ret:
_mp_destroy(mpstack);
return FAIL;
goto typval_encode_stop_converting_one_item; 
}
