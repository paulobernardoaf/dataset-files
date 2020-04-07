












#include "vim.h"

#if defined(FEAT_EVAL) || defined(PROTO)




static dict_T *first_dict = NULL;




dict_T *
dict_alloc(void)
{
dict_T *d;

d = ALLOC_CLEAR_ONE(dict_T);
if (d != NULL)
{

if (first_dict != NULL)
first_dict->dv_used_prev = d;
d->dv_used_next = first_dict;
d->dv_used_prev = NULL;
first_dict = d;

hash_init(&d->dv_hashtab);
d->dv_lock = 0;
d->dv_scope = 0;
d->dv_refcount = 0;
d->dv_copyID = 0;
}
return d;
}




dict_T *
dict_alloc_id(alloc_id_T id UNUSED)
{
#if defined(FEAT_EVAL)
if (alloc_fail_id == id && alloc_does_fail(sizeof(list_T)))
return NULL;
#endif
return (dict_alloc());
}

dict_T *
dict_alloc_lock(int lock)
{
dict_T *d = dict_alloc();

if (d != NULL)
d->dv_lock = lock;
return d;
}





int
rettv_dict_alloc(typval_T *rettv)
{
dict_T *d = dict_alloc_lock(0);

if (d == NULL)
return FAIL;

rettv_dict_set(rettv, d);
return OK;
}




void
rettv_dict_set(typval_T *rettv, dict_T *d)
{
rettv->v_type = VAR_DICT;
rettv->vval.v_dict = d;
if (d != NULL)
++d->dv_refcount;
}





void
dict_free_contents(dict_T *d)
{
int todo;
hashitem_T *hi;
dictitem_T *di;


hash_lock(&d->dv_hashtab);
todo = (int)d->dv_hashtab.ht_used;
for (hi = d->dv_hashtab.ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{


di = HI2DI(hi);
hash_remove(&d->dv_hashtab, hi);
dictitem_free(di);
--todo;
}
}


hash_clear(&d->dv_hashtab);
}

static void
dict_free_dict(dict_T *d)
{

if (d->dv_used_prev == NULL)
first_dict = d->dv_used_next;
else
d->dv_used_prev->dv_used_next = d->dv_used_next;
if (d->dv_used_next != NULL)
d->dv_used_next->dv_used_prev = d->dv_used_prev;
vim_free(d);
}

static void
dict_free(dict_T *d)
{
if (!in_free_unref_items)
{
dict_free_contents(d);
dict_free_dict(d);
}
}





void
dict_unref(dict_T *d)
{
if (d != NULL && --d->dv_refcount <= 0)
dict_free(d);
}





int
dict_free_nonref(int copyID)
{
dict_T *dd;
int did_free = FALSE;

for (dd = first_dict; dd != NULL; dd = dd->dv_used_next)
if ((dd->dv_copyID & COPYID_MASK) != (copyID & COPYID_MASK))
{



dict_free_contents(dd);
did_free = TRUE;
}
return did_free;
}

void
dict_free_items(int copyID)
{
dict_T *dd, *dd_next;

for (dd = first_dict; dd != NULL; dd = dd_next)
{
dd_next = dd->dv_used_next;
if ((dd->dv_copyID & COPYID_MASK) != (copyID & COPYID_MASK))
dict_free_dict(dd);
}
}








dictitem_T *
dictitem_alloc(char_u *key)
{
dictitem_T *di;

di = alloc(offsetof(dictitem_T, di_key) + STRLEN(key) + 1);
if (di != NULL)
{
STRCPY(di->di_key, key);
di->di_flags = DI_FLAGS_ALLOC;
di->di_tv.v_lock = 0;
}
return di;
}




static dictitem_T *
dictitem_copy(dictitem_T *org)
{
dictitem_T *di;

di = alloc(offsetof(dictitem_T, di_key) + STRLEN(org->di_key) + 1);
if (di != NULL)
{
STRCPY(di->di_key, org->di_key);
di->di_flags = DI_FLAGS_ALLOC;
copy_tv(&org->di_tv, &di->di_tv);
}
return di;
}




void
dictitem_remove(dict_T *dict, dictitem_T *item)
{
hashitem_T *hi;

hi = hash_find(&dict->dv_hashtab, item->di_key);
if (HASHITEM_EMPTY(hi))
internal_error("dictitem_remove()");
else
hash_remove(&dict->dv_hashtab, hi);
dictitem_free(item);
}




void
dictitem_free(dictitem_T *item)
{
clear_tv(&item->di_tv);
if (item->di_flags & DI_FLAGS_ALLOC)
vim_free(item);
}







dict_T *
dict_copy(dict_T *orig, int deep, int copyID)
{
dict_T *copy;
dictitem_T *di;
int todo;
hashitem_T *hi;

if (orig == NULL)
return NULL;

copy = dict_alloc();
if (copy != NULL)
{
if (copyID != 0)
{
orig->dv_copyID = copyID;
orig->dv_copydict = copy;
}
todo = (int)orig->dv_hashtab.ht_used;
for (hi = orig->dv_hashtab.ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;

di = dictitem_alloc(hi->hi_key);
if (di == NULL)
break;
if (deep)
{
if (item_copy(&HI2DI(hi)->di_tv, &di->di_tv, deep,
copyID) == FAIL)
{
vim_free(di);
break;
}
}
else
copy_tv(&HI2DI(hi)->di_tv, &di->di_tv);
if (dict_add(copy, di) == FAIL)
{
dictitem_free(di);
break;
}
}
}

++copy->dv_refcount;
if (todo > 0)
{
dict_unref(copy);
copy = NULL;
}
}

return copy;
}





int
dict_add(dict_T *d, dictitem_T *item)
{
return hash_add(&d->dv_hashtab, item->di_key);
}





static int
dict_add_number_special(dict_T *d, char *key, varnumber_T nr, vartype_T vartype)
{
dictitem_T *item;

item = dictitem_alloc((char_u *)key);
if (item == NULL)
return FAIL;
item->di_tv.v_type = vartype;
item->di_tv.vval.v_number = nr;
if (dict_add(d, item) == FAIL)
{
dictitem_free(item);
return FAIL;
}
return OK;
}





int
dict_add_number(dict_T *d, char *key, varnumber_T nr)
{
return dict_add_number_special(d, key, nr, VAR_NUMBER);
}





int
dict_add_bool(dict_T *d, char *key, varnumber_T nr)
{
return dict_add_number_special(d, key, nr, VAR_BOOL);
}





int
dict_add_string(dict_T *d, char *key, char_u *str)
{
return dict_add_string_len(d, key, str, -1);
}







int
dict_add_string_len(dict_T *d, char *key, char_u *str, int len)
{
dictitem_T *item;
char_u *val = NULL;

item = dictitem_alloc((char_u *)key);
if (item == NULL)
return FAIL;
item->di_tv.v_type = VAR_STRING;
if (str != NULL)
{
if (len == -1)
val = vim_strsave(str);
else
val = vim_strnsave(str, len);
}
item->di_tv.vval.v_string = val;
if (dict_add(d, item) == FAIL)
{
dictitem_free(item);
return FAIL;
}
return OK;
}





int
dict_add_list(dict_T *d, char *key, list_T *list)
{
dictitem_T *item;

item = dictitem_alloc((char_u *)key);
if (item == NULL)
return FAIL;
item->di_tv.v_type = VAR_LIST;
item->di_tv.vval.v_list = list;
++list->lv_refcount;
if (dict_add(d, item) == FAIL)
{
dictitem_free(item);
return FAIL;
}
return OK;
}





int
dict_add_tv(dict_T *d, char *key, typval_T *tv)
{
dictitem_T *item;

item = dictitem_alloc((char_u *)key);
if (item == NULL)
return FAIL;
copy_tv(tv, &item->di_tv);
if (dict_add(d, item) == FAIL)
{
dictitem_free(item);
return FAIL;
}
return OK;
}





int
dict_add_callback(dict_T *d, char *key, callback_T *cb)
{
dictitem_T *item;

item = dictitem_alloc((char_u *)key);
if (item == NULL)
return FAIL;
put_callback(cb, &item->di_tv);
if (dict_add(d, item) == FAIL)
{
dictitem_free(item);
return FAIL;
}
return OK;
}








void
dict_iterate_start(typval_T *var, dict_iterator_T *iter)
{
if (var->v_type != VAR_DICT || var->vval.v_dict == NULL)
iter->dit_todo = 0;
else
{
dict_T *d = var->vval.v_dict;

iter->dit_todo = d->dv_hashtab.ht_used;
iter->dit_hi = d->dv_hashtab.ht_array;
}
}








char_u *
dict_iterate_next(dict_iterator_T *iter, typval_T **tv_result)
{
dictitem_T *di;
char_u *result;

if (iter->dit_todo == 0)
return NULL;

while (HASHITEM_EMPTY(iter->dit_hi))
++iter->dit_hi;

di = HI2DI(iter->dit_hi);
result = di->di_key;
*tv_result = &di->di_tv;

--iter->dit_todo;
++iter->dit_hi;
return result;
}





int
dict_add_dict(dict_T *d, char *key, dict_T *dict)
{
dictitem_T *item;

item = dictitem_alloc((char_u *)key);
if (item == NULL)
return FAIL;
item->di_tv.v_type = VAR_DICT;
item->di_tv.vval.v_dict = dict;
++dict->dv_refcount;
if (dict_add(d, item) == FAIL)
{
dictitem_free(item);
return FAIL;
}
return OK;
}




long
dict_len(dict_T *d)
{
if (d == NULL)
return 0L;
return (long)d->dv_hashtab.ht_used;
}






dictitem_T *
dict_find(dict_T *d, char_u *key, int len)
{
#define AKEYLEN 200
char_u buf[AKEYLEN];
char_u *akey;
char_u *tofree = NULL;
hashitem_T *hi;

if (d == NULL)
return NULL;
if (len < 0)
akey = key;
else if (len >= AKEYLEN)
{
tofree = akey = vim_strnsave(key, len);
if (akey == NULL)
return NULL;
}
else
{

vim_strncpy(buf, key, len);
akey = buf;
}

hi = hash_find(&d->dv_hashtab, akey);
vim_free(tofree);
if (HASHITEM_EMPTY(hi))
return NULL;
return HI2DI(hi);
}





int
dict_get_tv(dict_T *d, char_u *key, typval_T *rettv)
{
dictitem_T *di;

di = dict_find(d, key, -1);
if (di == NULL)
return FAIL;
copy_tv(&di->di_tv, rettv);
return OK;
}







char_u *
dict_get_string(dict_T *d, char_u *key, int save)
{
dictitem_T *di;
char_u *s;

di = dict_find(d, key, -1);
if (di == NULL)
return NULL;
s = tv_get_string(&di->di_tv);
if (save && s != NULL)
s = vim_strsave(s);
return s;
}





varnumber_T
dict_get_number(dict_T *d, char_u *key)
{
return dict_get_number_def(d, key, 0);
}





varnumber_T
dict_get_number_def(dict_T *d, char_u *key, int def)
{
dictitem_T *di;

di = dict_find(d, key, -1);
if (di == NULL)
return def;
return tv_get_number(&di->di_tv);
}






varnumber_T
dict_get_number_check(dict_T *d, char_u *key)
{
dictitem_T *di;

di = dict_find(d, key, -1);
if (di == NULL)
return 0;
if (di->di_tv.v_type != VAR_NUMBER)
{
semsg(_(e_invarg2), tv_get_string(&di->di_tv));
return 0;
}
return tv_get_number(&di->di_tv);
}





char_u *
dict2string(typval_T *tv, int copyID, int restore_copyID)
{
garray_T ga;
int first = TRUE;
char_u *tofree;
char_u numbuf[NUMBUFLEN];
hashitem_T *hi;
char_u *s;
dict_T *d;
int todo;

if ((d = tv->vval.v_dict) == NULL)
return NULL;
ga_init2(&ga, (int)sizeof(char), 80);
ga_append(&ga, '{');

todo = (int)d->dv_hashtab.ht_used;
for (hi = d->dv_hashtab.ht_array; todo > 0 && !got_int; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;

if (first)
first = FALSE;
else
ga_concat(&ga, (char_u *)", ");

tofree = string_quote(hi->hi_key, FALSE);
if (tofree != NULL)
{
ga_concat(&ga, tofree);
vim_free(tofree);
}
ga_concat(&ga, (char_u *)": ");
s = echo_string_core(&HI2DI(hi)->di_tv, &tofree, numbuf, copyID,
FALSE, restore_copyID, TRUE);
if (s != NULL)
ga_concat(&ga, s);
vim_free(tofree);
if (s == NULL || did_echo_string_emsg)
break;
line_breakcheck();

}
}
if (todo > 0)
{
vim_free(ga.ga_data);
return NULL;
}

ga_append(&ga, '}');
ga_append(&ga, NUL);
return (char_u *)ga.ga_data;
}





static int
get_literal_key(char_u **arg, typval_T *tv)
{
char_u *p;

if (!ASCII_ISALNUM(**arg) && **arg != '_' && **arg != '-')
return FAIL;

for (p = *arg; ASCII_ISALNUM(*p) || *p == '_' || *p == '-'; ++p)
;
tv->v_type = VAR_STRING;
tv->vval.v_string = vim_strnsave(*arg, (int)(p - *arg));

*arg = skipwhite(p);
return OK;
}






int
eval_dict(char_u **arg, typval_T *rettv, int evaluate, int literal)
{
dict_T *d = NULL;
typval_T tvkey;
typval_T tv;
char_u *key = NULL;
dictitem_T *item;
char_u *start = skipwhite(*arg + 1);
char_u buf[NUMBUFLEN];








if (*start != '}')
{
if (eval1(&start, &tv, FALSE) == FAIL) 
return FAIL;
if (*start == '}')
return NOTDONE;
}

if (evaluate)
{
d = dict_alloc();
if (d == NULL)
return FAIL;
}
tvkey.v_type = VAR_UNKNOWN;
tv.v_type = VAR_UNKNOWN;

*arg = skipwhite(*arg + 1);
while (**arg != '}' && **arg != NUL)
{
if ((literal
? get_literal_key(arg, &tvkey)
: eval1(arg, &tvkey, evaluate)) == FAIL) 
goto failret;

if (**arg != ':')
{
if (evaluate)
semsg(_(e_missing_dict_colon), *arg);
clear_tv(&tvkey);
goto failret;
}
if (evaluate)
{
key = tv_get_string_buf_chk(&tvkey, buf);
if (key == NULL)
{

clear_tv(&tvkey);
goto failret;
}
}

*arg = skipwhite(*arg + 1);
if (eval1(arg, &tv, evaluate) == FAIL) 
{
if (evaluate)
clear_tv(&tvkey);
goto failret;
}
if (evaluate)
{
item = dict_find(d, key, -1);
if (item != NULL)
{
if (evaluate)
semsg(_(e_duplicate_key), key);
clear_tv(&tvkey);
clear_tv(&tv);
goto failret;
}
item = dictitem_alloc(key);
if (item != NULL)
{
item->di_tv = tv;
item->di_tv.v_lock = 0;
if (dict_add(d, item) == FAIL)
dictitem_free(item);
}
}
clear_tv(&tvkey);

if (**arg == '}')
break;
if (**arg != ',')
{
if (evaluate)
semsg(_(e_missing_dict_comma), *arg);
goto failret;
}
*arg = skipwhite(*arg + 1);
}

if (**arg != '}')
{
if (evaluate)
semsg(_(e_missing_dict_end), *arg);
failret:
if (d != NULL)
dict_free(d);
return FAIL;
}

*arg = skipwhite(*arg + 1);
if (evaluate)
rettv_dict_set(rettv, d);

return OK;
}







void
dict_extend(dict_T *d1, dict_T *d2, char_u *action)
{
dictitem_T *di1;
hashitem_T *hi2;
int todo;
char_u *arg_errmsg = (char_u *)N_("extend() argument");

todo = (int)d2->dv_hashtab.ht_used;
for (hi2 = d2->dv_hashtab.ht_array; todo > 0; ++hi2)
{
if (!HASHITEM_EMPTY(hi2))
{
--todo;
di1 = dict_find(d1, hi2->hi_key, -1);
if (d1->dv_scope != 0)
{


if (d1->dv_scope == VAR_DEF_SCOPE
&& HI2DI(hi2)->di_tv.v_type == VAR_FUNC
&& var_check_func_name(hi2->hi_key, di1 == NULL))
break;
if (!valid_varname(hi2->hi_key))
break;
}
if (di1 == NULL)
{
di1 = dictitem_copy(HI2DI(hi2));
if (di1 != NULL && dict_add(d1, di1) == FAIL)
dictitem_free(di1);
}
else if (*action == 'e')
{
semsg(_("E737: Key already exists: %s"), hi2->hi_key);
break;
}
else if (*action == 'f' && HI2DI(hi2) != di1)
{
if (var_check_lock(di1->di_tv.v_lock, arg_errmsg, TRUE)
|| var_check_ro(di1->di_flags, arg_errmsg, TRUE))
break;
clear_tv(&di1->di_tv);
copy_tv(&HI2DI(hi2)->di_tv, &di1->di_tv);
}
}
}
}




dictitem_T *
dict_lookup(hashitem_T *hi)
{
return HI2DI(hi);
}




int
dict_equal(
dict_T *d1,
dict_T *d2,
int ic, 
int recursive) 
{
hashitem_T *hi;
dictitem_T *item2;
int todo;

if (d1 == NULL && d2 == NULL)
return TRUE;
if (d1 == NULL || d2 == NULL)
return FALSE;
if (d1 == d2)
return TRUE;
if (dict_len(d1) != dict_len(d2))
return FALSE;

todo = (int)d1->dv_hashtab.ht_used;
for (hi = d1->dv_hashtab.ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
item2 = dict_find(d2, hi->hi_key, -1);
if (item2 == NULL)
return FALSE;
if (!tv_equal(&HI2DI(hi)->di_tv, &item2->di_tv, ic, recursive))
return FALSE;
--todo;
}
}
return TRUE;
}







static void
dict_list(typval_T *argvars, typval_T *rettv, int what)
{
list_T *l2;
dictitem_T *di;
hashitem_T *hi;
listitem_T *li;
listitem_T *li2;
dict_T *d;
int todo;

if (argvars[0].v_type != VAR_DICT)
{
emsg(_(e_dictreq));
return;
}
if ((d = argvars[0].vval.v_dict) == NULL)
return;

if (rettv_list_alloc(rettv) == FAIL)
return;

todo = (int)d->dv_hashtab.ht_used;
for (hi = d->dv_hashtab.ht_array; todo > 0; ++hi)
{
if (!HASHITEM_EMPTY(hi))
{
--todo;
di = HI2DI(hi);

li = listitem_alloc();
if (li == NULL)
break;
list_append(rettv->vval.v_list, li);

if (what == 0)
{

li->li_tv.v_type = VAR_STRING;
li->li_tv.v_lock = 0;
li->li_tv.vval.v_string = vim_strsave(di->di_key);
}
else if (what == 1)
{

copy_tv(&di->di_tv, &li->li_tv);
}
else
{

l2 = list_alloc();
li->li_tv.v_type = VAR_LIST;
li->li_tv.v_lock = 0;
li->li_tv.vval.v_list = l2;
if (l2 == NULL)
break;
++l2->lv_refcount;

li2 = listitem_alloc();
if (li2 == NULL)
break;
list_append(l2, li2);
li2->li_tv.v_type = VAR_STRING;
li2->li_tv.v_lock = 0;
li2->li_tv.vval.v_string = vim_strsave(di->di_key);

li2 = listitem_alloc();
if (li2 == NULL)
break;
list_append(l2, li2);
copy_tv(&di->di_tv, &li2->li_tv);
}
}
}
}




void
f_items(typval_T *argvars, typval_T *rettv)
{
dict_list(argvars, rettv, 2);
}




void
f_keys(typval_T *argvars, typval_T *rettv)
{
dict_list(argvars, rettv, 0);
}




void
f_values(typval_T *argvars, typval_T *rettv)
{
dict_list(argvars, rettv, 1);
}




void
dict_set_items_ro(dict_T *di)
{
int todo = (int)di->dv_hashtab.ht_used;
hashitem_T *hi;


for (hi = di->dv_hashtab.ht_array; todo > 0 ; ++hi)
{
if (HASHITEM_EMPTY(hi))
continue;
--todo;
HI2DI(hi)->di_flags |= DI_FLAGS_RO | DI_FLAGS_FIX;
}
}




void
f_has_key(typval_T *argvars, typval_T *rettv)
{
if (argvars[0].v_type != VAR_DICT)
{
emsg(_(e_dictreq));
return;
}
if (argvars[0].vval.v_dict == NULL)
return;

rettv->vval.v_number = dict_find(argvars[0].vval.v_dict,
tv_get_string(&argvars[1]), -1) != NULL;
}




void
dict_remove(typval_T *argvars, typval_T *rettv, char_u *arg_errmsg)
{
dict_T *d;
char_u *key;
dictitem_T *di;

if (argvars[2].v_type != VAR_UNKNOWN)
semsg(_(e_toomanyarg), "remove()");
else if ((d = argvars[0].vval.v_dict) != NULL
&& !var_check_lock(d->dv_lock, arg_errmsg, TRUE))
{
key = tv_get_string_chk(&argvars[1]);
if (key != NULL)
{
di = dict_find(d, key, -1);
if (di == NULL)
semsg(_(e_dictkey), key);
else if (!var_check_fixed(di->di_flags, arg_errmsg, TRUE)
&& !var_check_ro(di->di_flags, arg_errmsg, TRUE))
{
*rettv = di->di_tv;
init_tv(&di->di_tv);
dictitem_remove(d, di);
}
}
}
}

#endif 
