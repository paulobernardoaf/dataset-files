#include "win32ole.h"
struct olevariabledata {
ITypeInfo *pTypeInfo;
UINT index;
};
static void olevariable_free(void *ptr);
static size_t olevariable_size(const void *ptr);
static VALUE folevariable_name(VALUE self);
static VALUE ole_variable_ole_type(ITypeInfo *pTypeInfo, UINT var_index);
static VALUE folevariable_ole_type(VALUE self);
static VALUE ole_variable_ole_type_detail(ITypeInfo *pTypeInfo, UINT var_index);
static VALUE folevariable_ole_type_detail(VALUE self);
static VALUE ole_variable_value(ITypeInfo *pTypeInfo, UINT var_index);
static VALUE folevariable_value(VALUE self);
static VALUE ole_variable_visible(ITypeInfo *pTypeInfo, UINT var_index);
static VALUE folevariable_visible(VALUE self);
static VALUE ole_variable_kind(ITypeInfo *pTypeInfo, UINT var_index);
static VALUE folevariable_variable_kind(VALUE self);
static VALUE ole_variable_varkind(ITypeInfo *pTypeInfo, UINT var_index);
static VALUE folevariable_varkind(VALUE self);
static VALUE folevariable_inspect(VALUE self);
static const rb_data_type_t olevariable_datatype = {
"win32ole_variable",
{NULL, olevariable_free, olevariable_size,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};
static void
olevariable_free(void *ptr)
{
struct olevariabledata *polevar = ptr;
OLE_FREE(polevar->pTypeInfo);
free(polevar);
}
static size_t
olevariable_size(const void *ptr)
{
return ptr ? sizeof(struct olevariabledata) : 0;
}
VALUE
create_win32ole_variable(ITypeInfo *pTypeInfo, UINT index, VALUE name)
{
struct olevariabledata *pvar;
VALUE obj = TypedData_Make_Struct(cWIN32OLE_VARIABLE, struct olevariabledata,
&olevariable_datatype, pvar);
pvar->pTypeInfo = pTypeInfo;
OLE_ADDREF(pTypeInfo);
pvar->index = index;
rb_ivar_set(obj, rb_intern("name"), name);
return obj;
}
static VALUE
folevariable_name(VALUE self)
{
return rb_ivar_get(self, rb_intern("name"));
}
static VALUE
ole_variable_ole_type(ITypeInfo *pTypeInfo, UINT var_index)
{
VARDESC *pVarDesc;
HRESULT hr;
VALUE type;
hr = pTypeInfo->lpVtbl->GetVarDesc(pTypeInfo, var_index, &pVarDesc);
if (FAILED(hr))
ole_raise(hr, eWIN32OLERuntimeError, "failed to GetVarDesc");
type = ole_typedesc2val(pTypeInfo, &(pVarDesc->elemdescVar.tdesc), Qnil);
pTypeInfo->lpVtbl->ReleaseVarDesc(pTypeInfo, pVarDesc);
return type;
}
static VALUE
folevariable_ole_type(VALUE self)
{
struct olevariabledata *pvar;
TypedData_Get_Struct(self, struct olevariabledata, &olevariable_datatype, pvar);
return ole_variable_ole_type(pvar->pTypeInfo, pvar->index);
}
static VALUE
ole_variable_ole_type_detail(ITypeInfo *pTypeInfo, UINT var_index)
{
VARDESC *pVarDesc;
HRESULT hr;
VALUE type = rb_ary_new();
hr = pTypeInfo->lpVtbl->GetVarDesc(pTypeInfo, var_index, &pVarDesc);
if (FAILED(hr))
ole_raise(hr, eWIN32OLERuntimeError, "failed to GetVarDesc");
ole_typedesc2val(pTypeInfo, &(pVarDesc->elemdescVar.tdesc), type);
pTypeInfo->lpVtbl->ReleaseVarDesc(pTypeInfo, pVarDesc);
return type;
}
static VALUE
folevariable_ole_type_detail(VALUE self)
{
struct olevariabledata *pvar;
TypedData_Get_Struct(self, struct olevariabledata, &olevariable_datatype, pvar);
return ole_variable_ole_type_detail(pvar->pTypeInfo, pvar->index);
}
static VALUE
ole_variable_value(ITypeInfo *pTypeInfo, UINT var_index)
{
VARDESC *pVarDesc;
HRESULT hr;
VALUE val = Qnil;
hr = pTypeInfo->lpVtbl->GetVarDesc(pTypeInfo, var_index, &pVarDesc);
if (FAILED(hr))
return Qnil;
if(pVarDesc->varkind == VAR_CONST)
val = ole_variant2val(V_UNION1(pVarDesc, lpvarValue));
pTypeInfo->lpVtbl->ReleaseVarDesc(pTypeInfo, pVarDesc);
return val;
}
static VALUE
folevariable_value(VALUE self)
{
struct olevariabledata *pvar;
TypedData_Get_Struct(self, struct olevariabledata, &olevariable_datatype, pvar);
return ole_variable_value(pvar->pTypeInfo, pvar->index);
}
static VALUE
ole_variable_visible(ITypeInfo *pTypeInfo, UINT var_index)
{
VARDESC *pVarDesc;
HRESULT hr;
VALUE visible = Qfalse;
hr = pTypeInfo->lpVtbl->GetVarDesc(pTypeInfo, var_index, &pVarDesc);
if (FAILED(hr))
return visible;
if (!(pVarDesc->wVarFlags & (VARFLAG_FHIDDEN |
VARFLAG_FRESTRICTED |
VARFLAG_FNONBROWSABLE))) {
visible = Qtrue;
}
pTypeInfo->lpVtbl->ReleaseVarDesc(pTypeInfo, pVarDesc);
return visible;
}
static VALUE
folevariable_visible(VALUE self)
{
struct olevariabledata *pvar;
TypedData_Get_Struct(self, struct olevariabledata, &olevariable_datatype, pvar);
return ole_variable_visible(pvar->pTypeInfo, pvar->index);
}
static VALUE
ole_variable_kind(ITypeInfo *pTypeInfo, UINT var_index)
{
VARDESC *pVarDesc;
HRESULT hr;
VALUE kind = rb_str_new2("UNKNOWN");
hr = pTypeInfo->lpVtbl->GetVarDesc(pTypeInfo, var_index, &pVarDesc);
if (FAILED(hr))
return kind;
switch(pVarDesc->varkind) {
case VAR_PERINSTANCE:
kind = rb_str_new2("PERINSTANCE");
break;
case VAR_STATIC:
kind = rb_str_new2("STATIC");
break;
case VAR_CONST:
kind = rb_str_new2("CONSTANT");
break;
case VAR_DISPATCH:
kind = rb_str_new2("DISPATCH");
break;
default:
break;
}
pTypeInfo->lpVtbl->ReleaseVarDesc(pTypeInfo, pVarDesc);
return kind;
}
static VALUE
folevariable_variable_kind(VALUE self)
{
struct olevariabledata *pvar;
TypedData_Get_Struct(self, struct olevariabledata, &olevariable_datatype, pvar);
return ole_variable_kind(pvar->pTypeInfo, pvar->index);
}
static VALUE
ole_variable_varkind(ITypeInfo *pTypeInfo, UINT var_index)
{
VARDESC *pVarDesc;
HRESULT hr;
VALUE kind = Qnil;
hr = pTypeInfo->lpVtbl->GetVarDesc(pTypeInfo, var_index, &pVarDesc);
if (FAILED(hr))
return kind;
pTypeInfo->lpVtbl->ReleaseVarDesc(pTypeInfo, pVarDesc);
kind = RB_INT2FIX(pVarDesc->varkind);
return kind;
}
static VALUE
folevariable_varkind(VALUE self)
{
struct olevariabledata *pvar;
TypedData_Get_Struct(self, struct olevariabledata, &olevariable_datatype, pvar);
return ole_variable_varkind(pvar->pTypeInfo, pvar->index);
}
static VALUE
folevariable_inspect(VALUE self)
{
VALUE v = rb_inspect(folevariable_value(self));
VALUE n = folevariable_name(self);
VALUE detail = rb_sprintf("%"PRIsVALUE"=%"PRIsVALUE, n, v);
return make_inspect("WIN32OLE_VARIABLE", detail);
}
void Init_win32ole_variable(void)
{
cWIN32OLE_VARIABLE = rb_define_class("WIN32OLE_VARIABLE", rb_cObject);
rb_define_method(cWIN32OLE_VARIABLE, "name", folevariable_name, 0);
rb_define_method(cWIN32OLE_VARIABLE, "ole_type", folevariable_ole_type, 0);
rb_define_method(cWIN32OLE_VARIABLE, "ole_type_detail", folevariable_ole_type_detail, 0);
rb_define_method(cWIN32OLE_VARIABLE, "value", folevariable_value, 0);
rb_define_method(cWIN32OLE_VARIABLE, "visible?", folevariable_visible, 0);
rb_define_method(cWIN32OLE_VARIABLE, "variable_kind", folevariable_variable_kind, 0);
rb_define_method(cWIN32OLE_VARIABLE, "varkind", folevariable_varkind, 0);
rb_define_method(cWIN32OLE_VARIABLE, "inspect", folevariable_inspect, 0);
rb_define_alias(cWIN32OLE_VARIABLE, "to_s", "name");
}
