#include "win32ole.h"
static void olemethod_free(void *ptr);
static size_t olemethod_size(const void *ptr);
static VALUE ole_method_sub(VALUE self, ITypeInfo *pOwnerTypeInfo, ITypeInfo *pTypeInfo, VALUE name);
static VALUE olemethod_from_typeinfo(VALUE self, ITypeInfo *pTypeInfo, VALUE name);
static VALUE ole_methods_sub(ITypeInfo *pOwnerTypeInfo, ITypeInfo *pTypeInfo, VALUE methods, int mask);
static VALUE olemethod_set_member(VALUE self, ITypeInfo *pTypeInfo, ITypeInfo *pOwnerTypeInfo, int index, VALUE name);
static VALUE folemethod_initialize(VALUE self, VALUE oletype, VALUE method);
static VALUE folemethod_name(VALUE self);
static VALUE ole_method_return_type(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_return_type(VALUE self);
static VALUE ole_method_return_vtype(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_return_vtype(VALUE self);
static VALUE ole_method_return_type_detail(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_return_type_detail(VALUE self);
static VALUE ole_method_invkind(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE ole_method_invoke_kind(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_invkind(VALUE self);
static VALUE folemethod_invoke_kind(VALUE self);
static VALUE ole_method_visible(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_visible(VALUE self);
static VALUE ole_method_event(ITypeInfo *pTypeInfo, UINT method_index, VALUE method_name);
static VALUE folemethod_event(VALUE self);
static VALUE folemethod_event_interface(VALUE self);
static HRESULT ole_method_docinfo_from_type(ITypeInfo *pTypeInfo, UINT method_index, BSTR *name, BSTR *helpstr, DWORD *helpcontext, BSTR *helpfile);
static VALUE ole_method_helpstring(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_helpstring(VALUE self);
static VALUE ole_method_helpfile(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_helpfile(VALUE self);
static VALUE ole_method_helpcontext(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_helpcontext(VALUE self);
static VALUE ole_method_dispid(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_dispid(VALUE self);
static VALUE ole_method_offset_vtbl(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_offset_vtbl(VALUE self);
static VALUE ole_method_size_params(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_size_params(VALUE self);
static VALUE ole_method_size_opt_params(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_size_opt_params(VALUE self);
static VALUE ole_method_params(ITypeInfo *pTypeInfo, UINT method_index);
static VALUE folemethod_params(VALUE self);
static VALUE folemethod_inspect(VALUE self);
static const rb_data_type_t olemethod_datatype = {
"win32ole_method",
{NULL, olemethod_free, olemethod_size,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};
static void
olemethod_free(void *ptr)
{
struct olemethoddata *polemethod = ptr;
OLE_FREE(polemethod->pTypeInfo);
OLE_FREE(polemethod->pOwnerTypeInfo);
free(polemethod);
}
static size_t
olemethod_size(const void *ptr)
{
return ptr ? sizeof(struct olemethoddata) : 0;
}
struct olemethoddata *
olemethod_data_get_struct(VALUE obj)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(obj, struct olemethoddata, &olemethod_datatype, pmethod);
return pmethod;
}
static VALUE
ole_method_sub(VALUE self, ITypeInfo *pOwnerTypeInfo, ITypeInfo *pTypeInfo, VALUE name)
{
HRESULT hr;
TYPEATTR *pTypeAttr;
BSTR bstr;
FUNCDESC *pFuncDesc;
WORD i;
VALUE fname;
VALUE method = Qnil;
hr = OLE_GET_TYPEATTR(pTypeInfo, &pTypeAttr);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLEQueryInterfaceError, "failed to GetTypeAttr");
}
for(i = 0; i < pTypeAttr->cFuncs && method == Qnil; i++) {
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, i, &pFuncDesc);
if (FAILED(hr))
continue;
hr = pTypeInfo->lpVtbl->GetDocumentation(pTypeInfo, pFuncDesc->memid,
&bstr, NULL, NULL, NULL);
if (FAILED(hr)) {
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
continue;
}
fname = WC2VSTR(bstr);
if (strcasecmp(StringValuePtr(name), StringValuePtr(fname)) == 0) {
olemethod_set_member(self, pTypeInfo, pOwnerTypeInfo, i, fname);
method = self;
}
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
pFuncDesc=NULL;
}
OLE_RELEASE_TYPEATTR(pTypeInfo, pTypeAttr);
return method;
}
VALUE
ole_methods_from_typeinfo(ITypeInfo *pTypeInfo, int mask)
{
HRESULT hr;
TYPEATTR *pTypeAttr;
WORD i;
HREFTYPE href;
ITypeInfo *pRefTypeInfo;
VALUE methods = rb_ary_new();
hr = OLE_GET_TYPEATTR(pTypeInfo, &pTypeAttr);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLEQueryInterfaceError, "failed to GetTypeAttr");
}
ole_methods_sub(0, pTypeInfo, methods, mask);
for(i=0; i < pTypeAttr->cImplTypes; i++){
hr = pTypeInfo->lpVtbl->GetRefTypeOfImplType(pTypeInfo, i, &href);
if(FAILED(hr))
continue;
hr = pTypeInfo->lpVtbl->GetRefTypeInfo(pTypeInfo, href, &pRefTypeInfo);
if (FAILED(hr))
continue;
ole_methods_sub(pTypeInfo, pRefTypeInfo, methods, mask);
OLE_RELEASE(pRefTypeInfo);
}
OLE_RELEASE_TYPEATTR(pTypeInfo, pTypeAttr);
return methods;
}
static VALUE
olemethod_from_typeinfo(VALUE self, ITypeInfo *pTypeInfo, VALUE name)
{
HRESULT hr;
TYPEATTR *pTypeAttr;
WORD i;
HREFTYPE href;
ITypeInfo *pRefTypeInfo;
VALUE method = Qnil;
hr = OLE_GET_TYPEATTR(pTypeInfo, &pTypeAttr);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLEQueryInterfaceError, "failed to GetTypeAttr");
}
method = ole_method_sub(self, 0, pTypeInfo, name);
if (method != Qnil) {
return method;
}
for(i=0; i < pTypeAttr->cImplTypes && method == Qnil; i++){
hr = pTypeInfo->lpVtbl->GetRefTypeOfImplType(pTypeInfo, i, &href);
if(FAILED(hr))
continue;
hr = pTypeInfo->lpVtbl->GetRefTypeInfo(pTypeInfo, href, &pRefTypeInfo);
if (FAILED(hr))
continue;
method = ole_method_sub(self, pTypeInfo, pRefTypeInfo, name);
OLE_RELEASE(pRefTypeInfo);
}
OLE_RELEASE_TYPEATTR(pTypeInfo, pTypeAttr);
return method;
}
static VALUE
ole_methods_sub(ITypeInfo *pOwnerTypeInfo, ITypeInfo *pTypeInfo, VALUE methods, int mask)
{
HRESULT hr;
TYPEATTR *pTypeAttr;
BSTR bstr;
FUNCDESC *pFuncDesc;
VALUE method;
WORD i;
hr = OLE_GET_TYPEATTR(pTypeInfo, &pTypeAttr);
if (FAILED(hr)) {
ole_raise(hr, eWIN32OLEQueryInterfaceError, "failed to GetTypeAttr");
}
for(i = 0; i < pTypeAttr->cFuncs; i++) {
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, i, &pFuncDesc);
if (FAILED(hr))
continue;
hr = pTypeInfo->lpVtbl->GetDocumentation(pTypeInfo, pFuncDesc->memid,
&bstr, NULL, NULL, NULL);
if (FAILED(hr)) {
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
continue;
}
if(pFuncDesc->invkind & mask) {
method = folemethod_s_allocate(cWIN32OLE_METHOD);
olemethod_set_member(method, pTypeInfo, pOwnerTypeInfo,
i, WC2VSTR(bstr));
rb_ary_push(methods, method);
}
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
pFuncDesc=NULL;
}
OLE_RELEASE_TYPEATTR(pTypeInfo, pTypeAttr);
return methods;
}
VALUE
create_win32ole_method(ITypeInfo *pTypeInfo, VALUE name)
{
VALUE method = folemethod_s_allocate(cWIN32OLE_METHOD);
VALUE obj = olemethod_from_typeinfo(method, pTypeInfo, name);
return obj;
}
static VALUE
olemethod_set_member(VALUE self, ITypeInfo *pTypeInfo, ITypeInfo *pOwnerTypeInfo, int index, VALUE name)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
pmethod->pTypeInfo = pTypeInfo;
OLE_ADDREF(pTypeInfo);
pmethod->pOwnerTypeInfo = pOwnerTypeInfo;
OLE_ADDREF(pOwnerTypeInfo);
pmethod->index = index;
rb_ivar_set(self, rb_intern("name"), name);
return self;
}
VALUE
folemethod_s_allocate(VALUE klass)
{
struct olemethoddata *pmethod;
VALUE obj;
obj = TypedData_Make_Struct(klass,
struct olemethoddata,
&olemethod_datatype, pmethod);
pmethod->pTypeInfo = NULL;
pmethod->pOwnerTypeInfo = NULL;
pmethod->index = 0;
return obj;
}
static VALUE
folemethod_initialize(VALUE self, VALUE oletype, VALUE method)
{
VALUE obj = Qnil;
ITypeInfo *pTypeInfo;
if (rb_obj_is_kind_of(oletype, cWIN32OLE_TYPE)) {
SafeStringValue(method);
pTypeInfo = itypeinfo(oletype);
obj = olemethod_from_typeinfo(self, pTypeInfo, method);
if (obj == Qnil) {
rb_raise(eWIN32OLERuntimeError, "not found %s",
StringValuePtr(method));
}
}
else {
rb_raise(rb_eTypeError, "1st argument should be WIN32OLE_TYPE object");
}
return obj;
}
static VALUE
folemethod_name(VALUE self)
{
return rb_ivar_get(self, rb_intern("name"));
}
static VALUE
ole_method_return_type(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE type;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
ole_raise(hr, eWIN32OLEQueryInterfaceError, "failed to GetFuncDesc");
type = ole_typedesc2val(pTypeInfo, &(pFuncDesc->elemdescFunc.tdesc), Qnil);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return type;
}
static VALUE
folemethod_return_type(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_return_type(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_return_vtype(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE vvt;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
ole_raise(hr, eWIN32OLERuntimeError, "failed to GetFuncDesc");
vvt = RB_INT2FIX(pFuncDesc->elemdescFunc.tdesc.vt);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return vvt;
}
static VALUE
folemethod_return_vtype(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_return_vtype(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_return_type_detail(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE type = rb_ary_new();
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return type;
ole_typedesc2val(pTypeInfo, &(pFuncDesc->elemdescFunc.tdesc), type);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return type;
}
static VALUE
folemethod_return_type_detail(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_return_type_detail(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_invkind(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE invkind;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if(FAILED(hr))
ole_raise(hr, eWIN32OLERuntimeError, "failed to GetFuncDesc");
invkind = RB_INT2FIX(pFuncDesc->invkind);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return invkind;
}
static VALUE
ole_method_invoke_kind(ITypeInfo *pTypeInfo, UINT method_index)
{
VALUE type = rb_str_new2("UNKNOWN");
VALUE invkind = ole_method_invkind(pTypeInfo, method_index);
if((RB_FIX2INT(invkind) & INVOKE_PROPERTYGET) &&
(RB_FIX2INT(invkind) & INVOKE_PROPERTYPUT) ) {
type = rb_str_new2("PROPERTY");
} else if(RB_FIX2INT(invkind) & INVOKE_PROPERTYGET) {
type = rb_str_new2("PROPERTYGET");
} else if(RB_FIX2INT(invkind) & INVOKE_PROPERTYPUT) {
type = rb_str_new2("PROPERTYPUT");
} else if(RB_FIX2INT(invkind) & INVOKE_PROPERTYPUTREF) {
type = rb_str_new2("PROPERTYPUTREF");
} else if(RB_FIX2INT(invkind) & INVOKE_FUNC) {
type = rb_str_new2("FUNC");
}
return type;
}
static VALUE
folemethod_invkind(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_invkind(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
folemethod_invoke_kind(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_invoke_kind(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_visible(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE visible;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if(FAILED(hr))
return Qfalse;
if (pFuncDesc->wFuncFlags & (FUNCFLAG_FRESTRICTED |
FUNCFLAG_FHIDDEN |
FUNCFLAG_FNONBROWSABLE)) {
visible = Qfalse;
} else {
visible = Qtrue;
}
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return visible;
}
static VALUE
folemethod_visible(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_visible(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_event(ITypeInfo *pTypeInfo, UINT method_index, VALUE method_name)
{
TYPEATTR *pTypeAttr;
HRESULT hr;
WORD i;
int flags;
HREFTYPE href;
ITypeInfo *pRefTypeInfo;
FUNCDESC *pFuncDesc;
BSTR bstr;
VALUE name;
VALUE event = Qfalse;
hr = OLE_GET_TYPEATTR(pTypeInfo, &pTypeAttr);
if (FAILED(hr))
return event;
if(pTypeAttr->typekind != TKIND_COCLASS) {
pTypeInfo->lpVtbl->ReleaseTypeAttr(pTypeInfo, pTypeAttr);
return event;
}
for (i = 0; i < pTypeAttr->cImplTypes; i++) {
hr = pTypeInfo->lpVtbl->GetImplTypeFlags(pTypeInfo, i, &flags);
if (FAILED(hr))
continue;
if (flags & IMPLTYPEFLAG_FSOURCE) {
hr = pTypeInfo->lpVtbl->GetRefTypeOfImplType(pTypeInfo,
i, &href);
if (FAILED(hr))
continue;
hr = pTypeInfo->lpVtbl->GetRefTypeInfo(pTypeInfo,
href, &pRefTypeInfo);
if (FAILED(hr))
continue;
hr = pRefTypeInfo->lpVtbl->GetFuncDesc(pRefTypeInfo, method_index,
&pFuncDesc);
if (FAILED(hr)) {
OLE_RELEASE(pRefTypeInfo);
continue;
}
hr = pRefTypeInfo->lpVtbl->GetDocumentation(pRefTypeInfo,
pFuncDesc->memid,
&bstr, NULL, NULL, NULL);
if (FAILED(hr)) {
pRefTypeInfo->lpVtbl->ReleaseFuncDesc(pRefTypeInfo, pFuncDesc);
OLE_RELEASE(pRefTypeInfo);
continue;
}
name = WC2VSTR(bstr);
pRefTypeInfo->lpVtbl->ReleaseFuncDesc(pRefTypeInfo, pFuncDesc);
OLE_RELEASE(pRefTypeInfo);
if (rb_str_cmp(method_name, name) == 0) {
event = Qtrue;
break;
}
}
}
OLE_RELEASE_TYPEATTR(pTypeInfo, pTypeAttr);
return event;
}
static VALUE
folemethod_event(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
if (!pmethod->pOwnerTypeInfo)
return Qfalse;
return ole_method_event(pmethod->pOwnerTypeInfo,
pmethod->index,
rb_ivar_get(self, rb_intern("name")));
}
static VALUE
folemethod_event_interface(VALUE self)
{
BSTR name;
struct olemethoddata *pmethod;
HRESULT hr;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
if(folemethod_event(self) == Qtrue) {
hr = ole_docinfo_from_type(pmethod->pTypeInfo, &name, NULL, NULL, NULL);
if(SUCCEEDED(hr))
return WC2VSTR(name);
}
return Qnil;
}
static HRESULT
ole_method_docinfo_from_type(
ITypeInfo *pTypeInfo,
UINT method_index,
BSTR *name,
BSTR *helpstr,
DWORD *helpcontext,
BSTR *helpfile
)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return hr;
hr = pTypeInfo->lpVtbl->GetDocumentation(pTypeInfo, pFuncDesc->memid,
name, helpstr,
helpcontext, helpfile);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return hr;
}
static VALUE
ole_method_helpstring(ITypeInfo *pTypeInfo, UINT method_index)
{
HRESULT hr;
BSTR bhelpstring;
hr = ole_method_docinfo_from_type(pTypeInfo, method_index, NULL, &bhelpstring,
NULL, NULL);
if (FAILED(hr))
return Qnil;
return WC2VSTR(bhelpstring);
}
static VALUE
folemethod_helpstring(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_helpstring(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_helpfile(ITypeInfo *pTypeInfo, UINT method_index)
{
HRESULT hr;
BSTR bhelpfile;
hr = ole_method_docinfo_from_type(pTypeInfo, method_index, NULL, NULL,
NULL, &bhelpfile);
if (FAILED(hr))
return Qnil;
return WC2VSTR(bhelpfile);
}
static VALUE
folemethod_helpfile(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_helpfile(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_helpcontext(ITypeInfo *pTypeInfo, UINT method_index)
{
HRESULT hr;
DWORD helpcontext = 0;
hr = ole_method_docinfo_from_type(pTypeInfo, method_index, NULL, NULL,
&helpcontext, NULL);
if (FAILED(hr))
return Qnil;
return RB_INT2FIX(helpcontext);
}
static VALUE
folemethod_helpcontext(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_helpcontext(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_dispid(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE dispid = Qnil;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return dispid;
dispid = RB_INT2NUM(pFuncDesc->memid);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return dispid;
}
static VALUE
folemethod_dispid(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_dispid(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_offset_vtbl(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE offset_vtbl = Qnil;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return offset_vtbl;
offset_vtbl = RB_INT2FIX(pFuncDesc->oVft);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return offset_vtbl;
}
static VALUE
folemethod_offset_vtbl(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_offset_vtbl(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_size_params(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE size_params = Qnil;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return size_params;
size_params = RB_INT2FIX(pFuncDesc->cParams);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return size_params;
}
static VALUE
folemethod_size_params(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_size_params(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_size_opt_params(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
VALUE size_opt_params = Qnil;
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return size_opt_params;
size_opt_params = RB_INT2FIX(pFuncDesc->cParamsOpt);
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return size_opt_params;
}
static VALUE
folemethod_size_opt_params(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_size_opt_params(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
ole_method_params(ITypeInfo *pTypeInfo, UINT method_index)
{
FUNCDESC *pFuncDesc;
HRESULT hr;
BSTR *bstrs;
UINT len, i;
VALUE param;
VALUE params = rb_ary_new();
hr = pTypeInfo->lpVtbl->GetFuncDesc(pTypeInfo, method_index, &pFuncDesc);
if (FAILED(hr))
return params;
len = 0;
bstrs = ALLOCA_N(BSTR, pFuncDesc->cParams + 1);
hr = pTypeInfo->lpVtbl->GetNames(pTypeInfo, pFuncDesc->memid,
bstrs, pFuncDesc->cParams + 1,
&len);
if (FAILED(hr)) {
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return params;
}
SysFreeString(bstrs[0]);
if (pFuncDesc->cParams > 0) {
for(i = 1; i < len; i++) {
param = create_win32ole_param(pTypeInfo, method_index, i-1, WC2VSTR(bstrs[i]));
rb_ary_push(params, param);
}
}
pTypeInfo->lpVtbl->ReleaseFuncDesc(pTypeInfo, pFuncDesc);
return params;
}
static VALUE
folemethod_params(VALUE self)
{
struct olemethoddata *pmethod;
TypedData_Get_Struct(self, struct olemethoddata, &olemethod_datatype, pmethod);
return ole_method_params(pmethod->pTypeInfo, pmethod->index);
}
static VALUE
folemethod_inspect(VALUE self)
{
return default_inspect(self, "WIN32OLE_METHOD");
}
void Init_win32ole_method(void)
{
cWIN32OLE_METHOD = rb_define_class("WIN32OLE_METHOD", rb_cObject);
rb_define_alloc_func(cWIN32OLE_METHOD, folemethod_s_allocate);
rb_define_method(cWIN32OLE_METHOD, "initialize", folemethod_initialize, 2);
rb_define_method(cWIN32OLE_METHOD, "name", folemethod_name, 0);
rb_define_method(cWIN32OLE_METHOD, "return_type", folemethod_return_type, 0);
rb_define_method(cWIN32OLE_METHOD, "return_vtype", folemethod_return_vtype, 0);
rb_define_method(cWIN32OLE_METHOD, "return_type_detail", folemethod_return_type_detail, 0);
rb_define_method(cWIN32OLE_METHOD, "invoke_kind", folemethod_invoke_kind, 0);
rb_define_method(cWIN32OLE_METHOD, "invkind", folemethod_invkind, 0);
rb_define_method(cWIN32OLE_METHOD, "visible?", folemethod_visible, 0);
rb_define_method(cWIN32OLE_METHOD, "event?", folemethod_event, 0);
rb_define_method(cWIN32OLE_METHOD, "event_interface", folemethod_event_interface, 0);
rb_define_method(cWIN32OLE_METHOD, "helpstring", folemethod_helpstring, 0);
rb_define_method(cWIN32OLE_METHOD, "helpfile", folemethod_helpfile, 0);
rb_define_method(cWIN32OLE_METHOD, "helpcontext", folemethod_helpcontext, 0);
rb_define_method(cWIN32OLE_METHOD, "dispid", folemethod_dispid, 0);
rb_define_method(cWIN32OLE_METHOD, "offset_vtbl", folemethod_offset_vtbl, 0);
rb_define_method(cWIN32OLE_METHOD, "size_params", folemethod_size_params, 0);
rb_define_method(cWIN32OLE_METHOD, "size_opt_params", folemethod_size_opt_params, 0);
rb_define_method(cWIN32OLE_METHOD, "params", folemethod_params, 0);
rb_define_alias(cWIN32OLE_METHOD, "to_s", "name");
rb_define_method(cWIN32OLE_METHOD, "inspect", folemethod_inspect, 0);
}
