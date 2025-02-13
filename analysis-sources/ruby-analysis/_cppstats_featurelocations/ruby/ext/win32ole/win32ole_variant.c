#include "win32ole.h"

struct olevariantdata {
VARIANT realvar;
VARIANT var;
};

static void olevariant_free(void *ptr);
static size_t olevariant_size(const void *ptr);
static void ole_val2olevariantdata(VALUE val, VARTYPE vt, struct olevariantdata *pvar);
static void ole_val2variant_err(VALUE val, VARIANT *var);
static void ole_set_byref(VARIANT *realvar, VARIANT *var, VARTYPE vt);
static VALUE folevariant_s_allocate(VALUE klass);
static VALUE folevariant_s_array(VALUE klass, VALUE dims, VALUE vvt);
static void check_type_val2variant(VALUE val);
static VALUE folevariant_initialize(VALUE self, VALUE args);
static LONG *ary2safe_array_index(int ary_size, VALUE *ary, SAFEARRAY *psa);
static void unlock_safe_array(SAFEARRAY *psa);
static SAFEARRAY *get_locked_safe_array(VALUE val);
static VALUE folevariant_ary_aref(int argc, VALUE *argv, VALUE self);
static VALUE folevariant_ary_aset(int argc, VALUE *argv, VALUE self);
static VALUE folevariant_value(VALUE self);
static VALUE folevariant_vartype(VALUE self);
static VALUE folevariant_set_value(VALUE self, VALUE val);

static const rb_data_type_t olevariant_datatype = {
"win32ole_variant",
{NULL, olevariant_free, olevariant_size,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static void
olevariant_free(void *ptr)
{
struct olevariantdata *pvar = ptr;
VariantClear(&(pvar->realvar));
VariantClear(&(pvar->var));
free(pvar);
}

static size_t
olevariant_size(const void *ptr)
{
return ptr ? sizeof(struct olevariantdata) : 0;
}

static void
ole_val2olevariantdata(VALUE val, VARTYPE vt, struct olevariantdata *pvar)
{
HRESULT hr = S_OK;

if (((vt & ~VT_BYREF) == (VT_ARRAY | VT_UI1)) && RB_TYPE_P(val, T_STRING)) {
long len = RSTRING_LEN(val);
void *pdest = NULL;
SAFEARRAY *p = NULL;
SAFEARRAY *psa = SafeArrayCreateVector(VT_UI1, 0, len);
if (!psa) {
rb_raise(rb_eRuntimeError, "fail to SafeArrayCreateVector");
}
hr = SafeArrayAccessData(psa, &pdest);
if (SUCCEEDED(hr)) {
memcpy(pdest, RSTRING_PTR(val), len);
SafeArrayUnaccessData(psa);
V_VT(&(pvar->realvar)) = (vt & ~VT_BYREF);
p = V_ARRAY(&(pvar->realvar));
if (p != NULL) {
SafeArrayDestroy(p);
}
V_ARRAY(&(pvar->realvar)) = psa;
if (vt & VT_BYREF) {
V_VT(&(pvar->var)) = vt;
V_ARRAYREF(&(pvar->var)) = &(V_ARRAY(&(pvar->realvar)));
} else {
hr = VariantCopy(&(pvar->var), &(pvar->realvar));
}
} else {
if (psa)
SafeArrayDestroy(psa);
}
} else if (vt & VT_ARRAY) {
if (val == Qnil) {
V_VT(&(pvar->var)) = vt;
if (vt & VT_BYREF) {
V_ARRAYREF(&(pvar->var)) = &(V_ARRAY(&(pvar->realvar)));
}
} else {
hr = ole_val_ary2variant_ary(val, &(pvar->realvar), (VARTYPE)(vt & ~VT_BYREF));
if (SUCCEEDED(hr)) {
if (vt & VT_BYREF) {
V_VT(&(pvar->var)) = vt;
V_ARRAYREF(&(pvar->var)) = &(V_ARRAY(&(pvar->realvar)));
} else {
hr = VariantCopy(&(pvar->var), &(pvar->realvar));
}
}
}
#if (_MSC_VER >= 1300) || defined(__CYGWIN__) || defined(__MINGW32__)
} else if ( (vt & ~VT_BYREF) == VT_I8 || (vt & ~VT_BYREF) == VT_UI8) {
ole_val2variant_ex(val, &(pvar->realvar), (vt & ~VT_BYREF));
ole_val2variant_ex(val, &(pvar->var), (vt & ~VT_BYREF));
V_VT(&(pvar->var)) = vt;
if (vt & VT_BYREF) {
ole_set_byref(&(pvar->realvar), &(pvar->var), vt);
}
#endif
} else if ( (vt & ~VT_BYREF) == VT_ERROR) {
ole_val2variant_err(val, &(pvar->realvar));
if (vt & VT_BYREF) {
ole_set_byref(&(pvar->realvar), &(pvar->var), vt);
} else {
hr = VariantCopy(&(pvar->var), &(pvar->realvar));
}
} else {
if (val == Qnil) {
V_VT(&(pvar->var)) = vt;
if (vt == (VT_BYREF | VT_VARIANT)) {
ole_set_byref(&(pvar->realvar), &(pvar->var), vt);
} else {
V_VT(&(pvar->realvar)) = vt & ~VT_BYREF;
if (vt & VT_BYREF) {
ole_set_byref(&(pvar->realvar), &(pvar->var), vt);
}
}
} else {
ole_val2variant_ex(val, &(pvar->realvar), (VARTYPE)(vt & ~VT_BYREF));
if (vt == (VT_BYREF | VT_VARIANT)) {
ole_set_byref(&(pvar->realvar), &(pvar->var), vt);
} else if (vt & VT_BYREF) {
if ( (vt & ~VT_BYREF) != V_VT(&(pvar->realvar))) {
hr = VariantChangeTypeEx(&(pvar->realvar), &(pvar->realvar),
cWIN32OLE_lcid, 0, (VARTYPE)(vt & ~VT_BYREF));
}
if (SUCCEEDED(hr)) {
ole_set_byref(&(pvar->realvar), &(pvar->var), vt);
}
} else {
if (vt == V_VT(&(pvar->realvar))) {
hr = VariantCopy(&(pvar->var), &(pvar->realvar));
} else {
hr = VariantChangeTypeEx(&(pvar->var), &(pvar->realvar),
cWIN32OLE_lcid, 0, vt);
}
}
}
}
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "failed to change type");
}
}

static void
ole_val2variant_err(VALUE val, VARIANT *var)
{
VALUE v = val;
if (rb_obj_is_kind_of(v, cWIN32OLE_VARIANT)) {
v = folevariant_value(v);
}
if (!(FIXNUM_P(v) || RB_TYPE_P(v, T_BIGNUM) || v == Qnil)) {
rb_raise(eWIN32OLERuntimeError, "failed to convert VT_ERROR VARIANT:`%"PRIsVALUE"'", rb_inspect(v));
}
V_VT(var) = VT_ERROR;
if (v != Qnil) {
V_ERROR(var) = RB_NUM2LONG(val);
} else {
V_ERROR(var) = 0;
}
}

static void
ole_set_byref(VARIANT *realvar, VARIANT *var, VARTYPE vt)
{
V_VT(var) = vt;
if (vt == (VT_VARIANT|VT_BYREF)) {
V_VARIANTREF(var) = realvar;
} else {
if (V_VT(realvar) != (vt & ~VT_BYREF)) {
rb_raise(eWIN32OLERuntimeError, "variant type mismatch");
}
switch(vt & ~VT_BYREF) {
case VT_I1:
V_I1REF(var) = &V_I1(realvar);
break;
case VT_UI1:
V_UI1REF(var) = &V_UI1(realvar);
break;
case VT_I2:
V_I2REF(var) = &V_I2(realvar);
break;
case VT_UI2:
V_UI2REF(var) = &V_UI2(realvar);
break;
case VT_I4:
V_I4REF(var) = &V_I4(realvar);
break;
case VT_UI4:
V_UI4REF(var) = &V_UI4(realvar);
break;
case VT_R4:
V_R4REF(var) = &V_R4(realvar);
break;
case VT_R8:
V_R8REF(var) = &V_R8(realvar);
break;

#if (_MSC_VER >= 1300) || defined(__CYGWIN__) || defined(__MINGW32__)
#if defined(V_I8REF)
case VT_I8:
V_I8REF(var) = &V_I8(realvar);
break;
#endif
#if defined(V_UI8REF)
case VT_UI8:
V_UI8REF(var) = &V_UI8(realvar);
break;
#endif
#endif
case VT_INT:
V_INTREF(var) = &V_INT(realvar);
break;

case VT_UINT:
V_UINTREF(var) = &V_UINT(realvar);
break;

case VT_CY:
V_CYREF(var) = &V_CY(realvar);
break;
case VT_DATE:
V_DATEREF(var) = &V_DATE(realvar);
break;
case VT_BSTR:
V_BSTRREF(var) = &V_BSTR(realvar);
break;
case VT_DISPATCH:
V_DISPATCHREF(var) = &V_DISPATCH(realvar);
break;
case VT_ERROR:
V_ERRORREF(var) = &V_ERROR(realvar);
break;
case VT_BOOL:
V_BOOLREF(var) = &V_BOOL(realvar);
break;
case VT_UNKNOWN:
V_UNKNOWNREF(var) = &V_UNKNOWN(realvar);
break;
case VT_ARRAY:
V_ARRAYREF(var) = &V_ARRAY(realvar);
break;
default:
rb_raise(eWIN32OLERuntimeError, "unknown type specified(setting BYREF):%d", vt);
break;
}
}
}

static VALUE
folevariant_s_allocate(VALUE klass)
{
struct olevariantdata *pvar;
VALUE obj;
ole_initialize();
obj = TypedData_Make_Struct(klass, struct olevariantdata, &olevariant_datatype, pvar);
VariantInit(&(pvar->var));
VariantInit(&(pvar->realvar));
return obj;
}

















static VALUE
folevariant_s_array(VALUE klass, VALUE elems, VALUE vvt)
{
VALUE obj = Qnil;
VARTYPE vt;
struct olevariantdata *pvar;
SAFEARRAYBOUND *psab = NULL;
SAFEARRAY *psa = NULL;
UINT dim = 0;
UINT i = 0;

ole_initialize();

vt = RB_NUM2UINT(vvt);
vt = (vt | VT_ARRAY);
Check_Type(elems, T_ARRAY);
obj = folevariant_s_allocate(klass);

TypedData_Get_Struct(obj, struct olevariantdata, &olevariant_datatype, pvar);
dim = RARRAY_LEN(elems);

psab = ALLOC_N(SAFEARRAYBOUND, dim);

if(!psab) {
rb_raise(rb_eRuntimeError, "memory allocation error");
}

for (i = 0; i < dim; i++) {
psab[i].cElements = RB_FIX2INT(rb_ary_entry(elems, i));
psab[i].lLbound = 0;
}

psa = SafeArrayCreate((VARTYPE)(vt & VT_TYPEMASK), dim, psab);
if (psa == NULL) {
if (psab) free(psab);
rb_raise(rb_eRuntimeError, "memory allocation error(SafeArrayCreate)");
}

V_VT(&(pvar->var)) = vt;
if (vt & VT_BYREF) {
V_VT(&(pvar->realvar)) = (vt & ~VT_BYREF);
V_ARRAY(&(pvar->realvar)) = psa;
V_ARRAYREF(&(pvar->var)) = &(V_ARRAY(&(pvar->realvar)));
} else {
V_ARRAY(&(pvar->var)) = psa;
}
if (psab) free(psab);
return obj;
}

static void
check_type_val2variant(VALUE val)
{
VALUE elem;
int len = 0;
int i = 0;
if(!rb_obj_is_kind_of(val, cWIN32OLE) &&
!rb_obj_is_kind_of(val, cWIN32OLE_VARIANT) &&
!rb_obj_is_kind_of(val, rb_cTime)) {
switch (TYPE(val)) {
case T_ARRAY:
len = RARRAY_LEN(val);
for(i = 0; i < len; i++) {
elem = rb_ary_entry(val, i);
check_type_val2variant(elem);
}
break;
case T_STRING:
case T_FIXNUM:
case T_BIGNUM:
case T_FLOAT:
case T_TRUE:
case T_FALSE:
case T_NIL:
break;
default:
rb_raise(rb_eTypeError, "can not convert WIN32OLE_VARIANT from type %s",
rb_obj_classname(val));
}
}
}




































static VALUE
folevariant_initialize(VALUE self, VALUE args)
{
int len = 0;
VARIANT var;
VALUE val;
VALUE vvt;
VARTYPE vt;
struct olevariantdata *pvar;

len = RARRAY_LEN(args);
rb_check_arity(len, 1, 3);
VariantInit(&var);
val = rb_ary_entry(args, 0);

check_type_val2variant(val);

TypedData_Get_Struct(self, struct olevariantdata, &olevariant_datatype, pvar);
if (len == 1) {
ole_val2variant(val, &(pvar->var));
} else {
vvt = rb_ary_entry(args, 1);
vt = RB_NUM2INT(vvt);
if ((vt & VT_TYPEMASK) == VT_RECORD) {
rb_raise(rb_eArgError, "not supported VT_RECORD WIN32OLE_VARIANT object");
}
ole_val2olevariantdata(val, vt, pvar);
}
return self;
}

static SAFEARRAY *
get_locked_safe_array(VALUE val)
{
struct olevariantdata *pvar;
SAFEARRAY *psa = NULL;
HRESULT hr;
TypedData_Get_Struct(val, struct olevariantdata, &olevariant_datatype, pvar);
if (!(V_VT(&(pvar->var)) & VT_ARRAY)) {
rb_raise(rb_eTypeError, "variant type is not VT_ARRAY.");
}
psa = V_ISBYREF(&(pvar->var)) ? *V_ARRAYREF(&(pvar->var)) : V_ARRAY(&(pvar->var));
if (psa == NULL) {
return psa;
}
hr = SafeArrayLock(psa);
if (FAILED(hr)) {
ole_raise(hr, rb_eRuntimeError, "failed to SafeArrayLock");
}
return psa;
}

static LONG *
ary2safe_array_index(int ary_size, VALUE *ary, SAFEARRAY *psa)
{
long dim;
LONG *pid;
long i;
dim = SafeArrayGetDim(psa);
if (dim != ary_size) {
rb_raise(rb_eArgError, "unmatch number of indices");
}
pid = ALLOC_N(LONG, dim);
if (pid == NULL) {
rb_raise(rb_eRuntimeError, "failed to allocate memory for indices");
}
for (i = 0; i < dim; i++) {
pid[i] = RB_NUM2INT(ary[i]);
}
return pid;
}

static void
unlock_safe_array(SAFEARRAY *psa)
{
HRESULT hr;
hr = SafeArrayUnlock(psa);
if (FAILED(hr)) {
ole_raise(hr, rb_eRuntimeError, "failed to SafeArrayUnlock");
}
}





















static VALUE
folevariant_ary_aref(int argc, VALUE *argv, VALUE self)
{
struct olevariantdata *pvar;
SAFEARRAY *psa;
VALUE val = Qnil;
VARIANT variant;
LONG *pid;
HRESULT hr;

TypedData_Get_Struct(self, struct olevariantdata, &olevariant_datatype, pvar);
if (!V_ISARRAY(&(pvar->var))) {
rb_raise(eWIN32OLERuntimeError,
"`[]' is not available for this variant type object");
}
psa = get_locked_safe_array(self);
if (psa == NULL) {
return val;
}

pid = ary2safe_array_index(argc, argv, psa);

VariantInit(&variant);
V_VT(&variant) = (V_VT(&(pvar->var)) & ~VT_ARRAY) | VT_BYREF;
hr = SafeArrayPtrOfIndex(psa, pid, &V_BYREF(&variant));
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "failed to SafeArrayPtrOfIndex");
}
val = ole_variant2val(&variant);

unlock_safe_array(psa);
if (pid) free(pid);
return val;
}






















static VALUE
folevariant_ary_aset(int argc, VALUE *argv, VALUE self)
{
struct olevariantdata *pvar;
SAFEARRAY *psa;
VARIANT var;
VARTYPE vt;
LONG *pid;
HRESULT hr;
VOID *p = NULL;

TypedData_Get_Struct(self, struct olevariantdata, &olevariant_datatype, pvar);
if (!V_ISARRAY(&(pvar->var))) {
rb_raise(eWIN32OLERuntimeError,
"`[]' is not available for this variant type object");
}
psa = get_locked_safe_array(self);
if (psa == NULL) {
rb_raise(rb_eRuntimeError, "failed to get SafeArray pointer");
}

pid = ary2safe_array_index(argc-1, argv, psa);

VariantInit(&var);
vt = (V_VT(&(pvar->var)) & ~VT_ARRAY);
p = val2variant_ptr(argv[argc-1], &var, vt);
if ((V_VT(&var) == VT_DISPATCH && V_DISPATCH(&var) == NULL) ||
(V_VT(&var) == VT_UNKNOWN && V_UNKNOWN(&var) == NULL)) {
rb_raise(eWIN32OLERuntimeError, "argument does not have IDispatch or IUnknown Interface");
}
hr = SafeArrayPutElement(psa, pid, p);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "failed to SafeArrayPutElement");
}

unlock_safe_array(psa);
if (pid) free(pid);
return argv[argc-1];
}










static VALUE
folevariant_value(VALUE self)
{
struct olevariantdata *pvar;
VALUE val = Qnil;
VARTYPE vt;
int dim;
SAFEARRAY *psa;
TypedData_Get_Struct(self, struct olevariantdata, &olevariant_datatype, pvar);

val = ole_variant2val(&(pvar->var));
vt = V_VT(&(pvar->var));

if ((vt & ~VT_BYREF) == (VT_UI1|VT_ARRAY)) {
if (vt & VT_BYREF) {
psa = *V_ARRAYREF(&(pvar->var));
} else {
psa = V_ARRAY(&(pvar->var));
}
if (!psa) {
return val;
}
dim = SafeArrayGetDim(psa);
if (dim == 1) {
val = rb_funcall(val, rb_intern("pack"), 1, rb_str_new2("C*"));
}
}
return val;
}










static VALUE
folevariant_vartype(VALUE self)
{
struct olevariantdata *pvar;
TypedData_Get_Struct(self, struct olevariantdata, &olevariant_datatype, pvar);
return RB_INT2FIX(V_VT(&pvar->var));
}















static VALUE
folevariant_set_value(VALUE self, VALUE val)
{
struct olevariantdata *pvar;
VARTYPE vt;
TypedData_Get_Struct(self, struct olevariantdata, &olevariant_datatype, pvar);
vt = V_VT(&(pvar->var));
if (V_ISARRAY(&(pvar->var)) && ((vt & ~VT_BYREF) != (VT_UI1|VT_ARRAY) || !RB_TYPE_P(val, T_STRING))) {
rb_raise(eWIN32OLERuntimeError,
"`value=' is not available for this variant type object");
}
ole_val2olevariantdata(val, vt, pvar);
return Qnil;
}

void
ole_variant2variant(VALUE val, VARIANT *var)
{
struct olevariantdata *pvar;
TypedData_Get_Struct(val, struct olevariantdata, &olevariant_datatype, pvar);
VariantCopy(var, &(pvar->var));
}

void
Init_win32ole_variant(void)
{
#undef rb_intern
cWIN32OLE_VARIANT = rb_define_class("WIN32OLE_VARIANT", rb_cObject);
rb_define_alloc_func(cWIN32OLE_VARIANT, folevariant_s_allocate);
rb_define_singleton_method(cWIN32OLE_VARIANT, "array", folevariant_s_array, 2);
rb_define_method(cWIN32OLE_VARIANT, "initialize", folevariant_initialize, -2);
rb_define_method(cWIN32OLE_VARIANT, "value", folevariant_value, 0);
rb_define_method(cWIN32OLE_VARIANT, "value=", folevariant_set_value, 1);
rb_define_method(cWIN32OLE_VARIANT, "vartype", folevariant_vartype, 0);
rb_define_method(cWIN32OLE_VARIANT, "[]", folevariant_ary_aref, -1);
rb_define_method(cWIN32OLE_VARIANT, "[]=", folevariant_ary_aset, -1);




rb_define_const(cWIN32OLE_VARIANT, "Empty",
rb_funcall(cWIN32OLE_VARIANT, rb_intern("new"), 2, Qnil, RB_INT2FIX(VT_EMPTY)));




rb_define_const(cWIN32OLE_VARIANT, "Null",
rb_funcall(cWIN32OLE_VARIANT, rb_intern("new"), 2, Qnil, RB_INT2FIX(VT_NULL)));




rb_define_const(cWIN32OLE_VARIANT, "Nothing",
rb_funcall(cWIN32OLE_VARIANT, rb_intern("new"), 2, Qnil, RB_INT2FIX(VT_DISPATCH)));








rb_define_const(cWIN32OLE_VARIANT, "NoParam",
rb_funcall(cWIN32OLE_VARIANT, rb_intern("new"), 2, INT2NUM(DISP_E_PARAMNOTFOUND), RB_INT2FIX(VT_ERROR)));
}
