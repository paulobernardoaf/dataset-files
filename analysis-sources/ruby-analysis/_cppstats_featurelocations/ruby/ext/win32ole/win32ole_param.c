#include "win32ole.h"

VALUE cWIN32OLE_PARAM;

struct oleparamdata {
ITypeInfo *pTypeInfo;
UINT method_index;
UINT index;
};

static void oleparam_free(void *ptr);
static size_t oleparam_size(const void *ptr);
static VALUE foleparam_s_allocate(VALUE klass);
static VALUE oleparam_ole_param_from_index(VALUE self, ITypeInfo *pTypeInfo, UINT method_index, int param_index);
static VALUE oleparam_ole_param(VALUE self, VALUE olemethod, int n);
static VALUE foleparam_initialize(VALUE self, VALUE olemethod, VALUE n);
static VALUE foleparam_name(VALUE self);
static VALUE ole_param_ole_type(ITypeInfo *pTypeInfo, UINT method_index, UINT index);
static VALUE foleparam_ole_type(VALUE self);
static VALUE ole_param_ole_type_detail(ITypeInfo *pTypeInfo, UINT method_index, UINT index);
static VALUE foleparam_ole_type_detail(VALUE self);
static VALUE ole_param_flag_mask(ITypeInfo *pTypeInfo, UINT method_index, UINT index, USHORT mask);
static VALUE foleparam_input(VALUE self);
static VALUE foleparam_output(VALUE self);
static VALUE foleparam_optional(VALUE self);
static VALUE foleparam_retval(VALUE self);
static VALUE ole_param_default(ITypeInfo *pTypeInfo, UINT method_index, UINT index);
static VALUE foleparam_default(VALUE self);
static VALUE foleparam_inspect(VALUE self);

static const rb_data_type_t oleparam_datatype = {
"win32ole_param",
{NULL, oleparam_free, oleparam_size,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static void
oleparam_free(void *ptr)
{
struct oleparamdata *pole = ptr;
OLE_FREE(pole->pTypeInfo);
free(pole);
}

static size_t
oleparam_size(const void *ptr)
{
return ptr ? sizeof(struct oleparamdata) : 0;
}

VALUE
create_win32ole_param(ITypeInfo *pTypeInfo, UINT method_index, UINT index, VALUE name)
{
struct oleparamdata *pparam;
VALUE obj = foleparam_s_allocate(cWIN32OLE_PARAM);
TypedData_Get_Struct(obj, struct oleparamdata, &oleparam_datatype, pparam);

pparam->pTypeInfo = pTypeInfo;
OLE_ADDREF(pTypeInfo);
pparam->method_index = method_index;
pparam->index = index;
rb_ivar_set(obj, rb_intern("name"), name);
return obj;
}







static VALUE
foleparam_s_allocate(VALUE klass)
{
struct oleparamdata *pparam;
VALUE obj;
obj = TypedData_Make_Struct(klass,
struct oleparamdata,
&oleparam_datatype, pparam);
pparam->pTypeInfo = NULL;
pparam->method_index = 0;
pparam->index = 0;
return obj;
}

static VALUE
oleparam_ole_param_from_index(VALUE self, ITypeInfo *pTypeInfo, UINT method_index, int param_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
BSTR *bstrs;
UINT len;
struct oleparamdata *pparam;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
ole_raise(hr, rb_eRuntimeError, "fail to ITypeInfo::GetFuncDesc");

len = 0;
bstrs = ALLOCA_N(BSTR, pFuncDesc->cParams + 1);
hr = pTypeInfo->lpVtbl->GetNames(pTypeInfo, pFuncDesc->memid,
bstrs, pFuncDesc->cParams + 1,
&len);
if (FAILED(hr)) {
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
ole_raise(hr, rb_eRuntimeError, "fail to ITypeInfo::GetNames");
}
SysFreeString(bstrs[0]);
if (param_index < 1 || len <= (UINT)param_index)
{
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
rb_raise(rb_eIndexError, "index of param must be in 1..%d", len);
}

TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
pparam->pTypeInfo = pTypeInfo;
OLE_ADDREF(pTypeInfo);
pparam->method_index = method_index;
pparam->index = param_index - 1;
rb_ivar_set(self, rb_intern("name"), WC2VSTR(bstrs[param_index]));

pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return self;
}

static VALUE
oleparam_ole_param(VALUE self, VALUE olemethod, int n)
{
struct olemethoddata *pmethod = olemethod_data_get_struct(olemethod);
return oleparam_ole_param_from_index(self, pmethod->pTypeInfo, pmethod->index, n);
}














static VALUE
foleparam_initialize(VALUE self, VALUE olemethod, VALUE n)
{
int idx;
if (!rb_obj_is_kind_of(olemethod, cWIN32OLE_METHOD)) {
rb_raise(rb_eTypeError, "1st parameter must be WIN32OLE_METHOD object");
}
idx = RB_FIX2INT(n);
return oleparam_ole_param(self, olemethod, idx);
}











static VALUE
foleparam_name(VALUE self)
{
return rb_ivar_get(self, rb_intern("name"));
}

static VALUE
ole_param_ole_type(ITypeInfo *pTypeInfo, UINT method_index, UINT index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE type = rb_str_new2("unknown type");
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return type;
type = ole_typedesc2val(pTypeInfo,
&(pFuncDesc->lprgelemdescParam[index].tdesc), Qnil);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return type;
}











static VALUE
foleparam_ole_type(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_ole_type(pparam->pTypeInfo, pparam->method_index,
pparam->index);
}

static VALUE
ole_param_ole_type_detail(ITypeInfo *pTypeInfo, UINT method_index, UINT index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE typedetail = rb_ary_new();
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return typedetail;
ole_typedesc2val(pTypeInfo,
&(pFuncDesc->lprgelemdescParam[index].tdesc), typedetail);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return typedetail;
}











static VALUE
foleparam_ole_type_detail(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_ole_type_detail(pparam->pTypeInfo, pparam->method_index,
pparam->index);
}

static VALUE
ole_param_flag_mask(ITypeInfo *pTypeInfo, UINT method_index, UINT index, USHORT mask)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE ret = Qfalse;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if(FAILED(hr))
return ret;
if (V_UNION1((&(pFuncDesc->lprgelemdescParam[index])), paramdesc).wParamFlags &mask)
ret = Qtrue;
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return ret;
}











static VALUE
foleparam_input(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_flag_mask(pparam->pTypeInfo, pparam->method_index,
pparam->index, PARAMFLAG_FIN);
}




















static VALUE
foleparam_output(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_flag_mask(pparam->pTypeInfo, pparam->method_index,
pparam->index, PARAMFLAG_FOUT);
}











static VALUE
foleparam_optional(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_flag_mask(pparam->pTypeInfo, pparam->method_index,
pparam->index, PARAMFLAG_FOPT);
}












static VALUE
foleparam_retval(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_flag_mask(pparam->pTypeInfo, pparam->method_index,
pparam->index, PARAMFLAG_FRETVAL);
}

static VALUE
ole_param_default(ITypeInfo *pTypeInfo, UINT method_index, UINT index)
{
FUNCDESC *pFuncDesc;
ELEMDESC *pElemDesc;
PARAMDESCEX * pParamDescEx;
HRESULT hr;
USHORT wParamFlags;
USHORT mask = PARAMFLAG_FOPT|PARAMFLAG_FHASDEFAULT;
VALUE defval = Qnil;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return defval;
pElemDesc = &pFuncDesc->lprgelemdescParam[index];
wParamFlags = V_UNION1(pElemDesc, paramdesc).wParamFlags;
if ((wParamFlags & mask) == mask) {
pParamDescEx = V_UNION1(pElemDesc, paramdesc).pparamdescex;
defval = ole_variant2val(&pParamDescEx->varDefaultValue);
}
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return defval;
}






























static VALUE
foleparam_default(VALUE self)
{
struct oleparamdata *pparam;
TypedData_Get_Struct(self, struct oleparamdata, &oleparam_datatype, pparam);
return ole_param_default(pparam->pTypeInfo, pparam->method_index,
pparam->index);
}









static VALUE
foleparam_inspect(VALUE self)
{
VALUE detail = foleparam_name(self);
VALUE defval = foleparam_default(self);
if (defval != Qnil) {
rb_str_cat2(detail, "=");
rb_str_concat(detail, rb_inspect(defval));
}
return make_inspect("WIN32OLE_PARAM", detail);
}

void
Init_win32ole_param(void)
{
cWIN32OLE_PARAM = rb_define_class("WIN32OLE_PARAM", rb_cObject);
rb_define_alloc_func(cWIN32OLE_PARAM, foleparam_s_allocate);
rb_define_method(cWIN32OLE_PARAM, "initialize", foleparam_initialize, 2);
rb_define_method(cWIN32OLE_PARAM, "name", foleparam_name, 0);
rb_define_method(cWIN32OLE_PARAM, "ole_type", foleparam_ole_type, 0);
rb_define_method(cWIN32OLE_PARAM, "ole_type_detail", foleparam_ole_type_detail, 0);
rb_define_method(cWIN32OLE_PARAM, "input?", foleparam_input, 0);
rb_define_method(cWIN32OLE_PARAM, "output?", foleparam_output, 0);
rb_define_method(cWIN32OLE_PARAM, "optional?", foleparam_optional, 0);
rb_define_method(cWIN32OLE_PARAM, "retval?", foleparam_retval, 0);
rb_define_method(cWIN32OLE_PARAM, "default", foleparam_default, 0);
rb_define_alias(cWIN32OLE_PARAM, "to_s", "name");
rb_define_method(cWIN32OLE_PARAM, "inspect", foleparam_inspect, 0);
}
