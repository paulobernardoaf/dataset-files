#include "win32ole.h"

struct olerecorddata {
IRecordInfo *pri;
void *pdata;
};

static HRESULT recordinfo_from_itypelib(ITypeLib *pTypeLib, VALUE name, IRecordInfo **ppri);
static int hash2olerec(VALUE key, VALUE val, VALUE rec);
static void olerecord_free(void *pvar);
static size_t olerecord_size(const void *ptr);
static VALUE folerecord_s_allocate(VALUE klass);
static VALUE folerecord_initialize(VALUE self, VALUE typename, VALUE oleobj);
static VALUE folerecord_to_h(VALUE self);
static VALUE folerecord_typename(VALUE self);
static VALUE olerecord_ivar_get(VALUE self, VALUE name);
static VALUE olerecord_ivar_set(VALUE self, VALUE name, VALUE val);
static VALUE folerecord_method_missing(int argc, VALUE *argv, VALUE self);
static VALUE folerecord_ole_instance_variable_get(VALUE self, VALUE name);
static VALUE folerecord_ole_instance_variable_set(VALUE self, VALUE name, VALUE val);
static VALUE folerecord_inspect(VALUE self);

static const rb_data_type_t olerecord_datatype = {
"win32ole_record",
{NULL, olerecord_free, olerecord_size,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static HRESULT
recordinfo_from_itypelib(ITypeLib *pTypeLib, VALUE name, IRecordInfo **ppri)
{

unsigned int count;
unsigned int i;
ITypeInfo *pTypeInfo;
HRESULT hr = OLE_E_LAST;
BSTR bstr;

count = pTypeLib->lpVtbl->GetTypeInfoCount(pTypeLib);
for (i = 0; i < count; i++) {
hr = pTypeLib->lpVtbl->GetDocumentation(pTypeLib, i,
&bstr, NULL, NULL, NULL);
if (FAILED(hr))
continue;

hr = pTypeLib->lpVtbl->GetTypeInfo(pTypeLib, i, &pTypeInfo);
if (FAILED(hr))
continue;

if (rb_str_cmp(WC2VSTR(bstr), name) == 0) {
hr = GetRecordInfoFromTypeInfo(pTypeInfo, ppri);
OLE_RELEASE(pTypeInfo);
return hr;
}
OLE_RELEASE(pTypeInfo);
}
hr = OLE_E_LAST;
return hr;
}

static int
hash2olerec(VALUE key, VALUE val, VALUE rec)
{
VARIANT var;
OLECHAR *pbuf;
struct olerecorddata *prec;
IRecordInfo *pri;
HRESULT hr;

if (val != Qnil) {
TypedData_Get_Struct(rec, struct olerecorddata, &olerecord_datatype, prec);
pri = prec->pri;
VariantInit(&var);
ole_val2variant(val, &var);
pbuf = ole_vstr2wc(key);
hr = pri->lpVtbl->PutField(pri, INVOKE_PROPERTYPUT, prec->pdata, pbuf, &var);
SysFreeString(pbuf);
VariantClear(&var);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "failed to putfield of `%s`", StringValuePtr(key));
}
}
return ST_CONTINUE;
}

void
ole_rec2variant(VALUE rec, VARIANT *var)
{
struct olerecorddata *prec;
ULONG size = 0;
IRecordInfo *pri;
HRESULT hr;
VALUE fields;
TypedData_Get_Struct(rec, struct olerecorddata, &olerecord_datatype, prec);
pri = prec->pri;
if (pri) {
hr = pri->lpVtbl->GetSize(pri, &size);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "failed to get size for allocation of VT_RECORD object");
}
if (prec->pdata) {
free(prec->pdata);
}
prec->pdata = ALLOC_N(char, size);
if (!prec->pdata) {
rb_raise(rb_eRuntimeError, "failed to memory allocation of %lu bytes", (unsigned long)size);
}
hr = pri->lpVtbl->RecordInit(pri, prec->pdata);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "failed to initialize VT_RECORD object");
}
fields = folerecord_to_h(rec);
rb_hash_foreach(fields, hash2olerec, rec);
V_RECORDINFO(var) = pri;
V_RECORD(var) = prec->pdata;
V_VT(var) = VT_RECORD;
} else {
rb_raise(eWIN32OLERuntimeError, "failed to retrieve IRecordInfo interface");
}
}

void
olerecord_set_ivar(VALUE obj, IRecordInfo *pri, void *prec)
{
HRESULT hr;
BSTR bstr;
BSTR *bstrs;
ULONG count = 0;
ULONG i;
VALUE fields;
VALUE val;
VARIANT var;
void *pdata = NULL;
struct olerecorddata *pvar;

TypedData_Get_Struct(obj, struct olerecorddata, &olerecord_datatype, pvar);
OLE_ADDREF(pri);
OLE_RELEASE(pvar->pri);
pvar->pri = pri;

hr = pri->lpVtbl->GetName(pri, &bstr);
if (SUCCEEDED(hr)) {
rb_ivar_set(obj, rb_intern("typename"), WC2VSTR(bstr));
}

hr = pri->lpVtbl->GetFieldNames(pri, &count, NULL);
if (FAILED(hr) || count == 0)
return;
bstrs = ALLOCA_N(BSTR, count);
hr = pri->lpVtbl->GetFieldNames(pri, &count, bstrs);
if (FAILED(hr)) {
return;
}

fields = rb_hash_new();
rb_ivar_set(obj, rb_intern("fields"), fields);
for (i = 0; i < count; i++) {
pdata = NULL;
VariantInit(&var);
val = Qnil;
if (prec) {
hr = pri->lpVtbl->GetFieldNoCopy(pri, prec, bstrs[i], &var, &pdata);
if (SUCCEEDED(hr)) {
val = ole_variant2val(&var);
}
}
rb_hash_aset(fields, WC2VSTR(bstrs[i]), val);
}
}

VALUE
create_win32ole_record(IRecordInfo *pri, void *prec)
{
VALUE obj = folerecord_s_allocate(cWIN32OLE_RECORD);
olerecord_set_ivar(obj, pri, prec);
return obj;
}





































static void
olerecord_free(void *ptr) {
struct olerecorddata *pvar = ptr;
OLE_FREE(pvar->pri);
if (pvar->pdata) {
free(pvar->pdata);
}
free(pvar);
}

static size_t
olerecord_size(const void *ptr)
{
const struct olerecorddata *pvar = ptr;
size_t s = 0;
ULONG size = 0;
HRESULT hr;
if (ptr) {
s += sizeof(struct olerecorddata);
if (pvar->pri) {
hr = pvar->pri->lpVtbl->GetSize(pvar->pri, &size);
if (SUCCEEDED(hr)) {
s += size;
}
}
}
return s;
}

static VALUE
folerecord_s_allocate(VALUE klass) {
VALUE obj = Qnil;
struct olerecorddata *pvar;
obj = TypedData_Make_Struct(klass, struct olerecorddata, &olerecord_datatype, pvar);
pvar->pri = NULL;
pvar->pdata = NULL;
return obj;
}




























static VALUE
folerecord_initialize(VALUE self, VALUE typename, VALUE oleobj) {
HRESULT hr;
ITypeLib *pTypeLib = NULL;
IRecordInfo *pri = NULL;

if (!RB_TYPE_P(typename, T_STRING) && !RB_TYPE_P(typename, T_SYMBOL)) {
rb_raise(rb_eArgError, "1st argument should be String or Symbol");
}
if (RB_TYPE_P(typename, T_SYMBOL)) {
typename = rb_sym2str(typename);
}

hr = S_OK;
if(rb_obj_is_kind_of(oleobj, cWIN32OLE)) {
hr = typelib_from_val(oleobj, &pTypeLib);
} else if (rb_obj_is_kind_of(oleobj, cWIN32OLE_TYPELIB)) {
pTypeLib = itypelib(oleobj);
OLE_ADDREF(pTypeLib);
if (pTypeLib) {
hr = S_OK;
} else {
hr = E_FAIL;
}
} else {
rb_raise(rb_eArgError, "2nd argument should be WIN32OLE object or WIN32OLE_TYPELIB object");
}

if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "fail to query ITypeLib interface");
}

hr = recordinfo_from_itypelib(pTypeLib, typename, &pri);
OLE_RELEASE(pTypeLib);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLERuntimeError, "fail to query IRecordInfo interface for `%s'", StringValuePtr(typename));
}

olerecord_set_ivar(self, pri, NULL);

return self;
}


































static VALUE
folerecord_to_h(VALUE self)
{
return rb_ivar_get(self, rb_intern("fields"));
}
































static VALUE
folerecord_typename(VALUE self)
{
return rb_ivar_get(self, rb_intern("typename"));
}

static VALUE
olerecord_ivar_get(VALUE self, VALUE name)
{
VALUE fields;
fields = rb_ivar_get(self, rb_intern("fields"));
return rb_hash_fetch(fields, name);
}

static VALUE
olerecord_ivar_set(VALUE self, VALUE name, VALUE val)
{
long len;
char *p;
VALUE fields;
len = RSTRING_LEN(name);
p = RSTRING_PTR(name);
if (p[len-1] == '=') {
name = rb_str_subseq(name, 0, len-1);
}
fields = rb_ivar_get(self, rb_intern("fields"));
rb_hash_fetch(fields, name);
return rb_hash_aset(fields, name, val);
}



























static VALUE
folerecord_method_missing(int argc, VALUE *argv, VALUE self)
{
VALUE name;
rb_check_arity(argc, 1, 2);
name = rb_sym2str(argv[0]);

#if SIZEOF_SIZE_T > SIZEOF_LONG
{
size_t n = strlen(StringValueCStr(name));
if (n >= LONG_MAX) {
rb_raise(rb_eRuntimeError, "too long member name");
}
}
#endif

if (argc == 1) {
return olerecord_ivar_get(self, name);
} else if (argc == 2) {
return olerecord_ivar_set(self, name, argv[1]);
}
return Qnil;
}





























static VALUE
folerecord_ole_instance_variable_get(VALUE self, VALUE name)
{
VALUE sname;
if(!RB_TYPE_P(name, T_STRING) && !RB_TYPE_P(name, T_SYMBOL)) {
rb_raise(rb_eTypeError, "wrong argument type (expected String or Symbol)");
}
sname = name;
if (RB_TYPE_P(name, T_SYMBOL)) {
sname = rb_sym2str(name);
}
return olerecord_ivar_get(self, sname);
}


























static VALUE
folerecord_ole_instance_variable_set(VALUE self, VALUE name, VALUE val)
{
VALUE sname;
if(!RB_TYPE_P(name, T_STRING) && !RB_TYPE_P(name, T_SYMBOL)) {
rb_raise(rb_eTypeError, "wrong argument type (expected String or Symbol)");
}
sname = name;
if (RB_TYPE_P(name, T_SYMBOL)) {
sname = rb_sym2str(name);
}
return olerecord_ivar_set(self, sname, val);
}























static VALUE
folerecord_inspect(VALUE self)
{
VALUE tname;
VALUE field;
tname = folerecord_typename(self);
if (tname == Qnil) {
tname = rb_inspect(tname);
}
field = rb_inspect(folerecord_to_h(self));
return rb_sprintf("#<WIN32OLE_RECORD(%"PRIsVALUE") %"PRIsVALUE">",
tname,
field);
}

void
Init_win32ole_record(void)
{
cWIN32OLE_RECORD = rb_define_class("WIN32OLE_RECORD", rb_cObject);
rb_define_alloc_func(cWIN32OLE_RECORD, folerecord_s_allocate);
rb_define_method(cWIN32OLE_RECORD, "initialize", folerecord_initialize, 2);
rb_define_method(cWIN32OLE_RECORD, "to_h", folerecord_to_h, 0);
rb_define_method(cWIN32OLE_RECORD, "typename", folerecord_typename, 0);
rb_define_method(cWIN32OLE_RECORD, "method_missing", folerecord_method_missing, -1);
rb_define_method(cWIN32OLE_RECORD, "ole_instance_variable_get", folerecord_ole_instance_variable_get, 1);
rb_define_method(cWIN32OLE_RECORD, "ole_instance_variable_set", folerecord_ole_instance_variable_set, 2);
rb_define_method(cWIN32OLE_RECORD, "inspect", folerecord_inspect, 0);
}
