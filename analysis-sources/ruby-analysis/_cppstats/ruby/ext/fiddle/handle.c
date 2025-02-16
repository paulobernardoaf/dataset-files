#include <ruby.h>
#include <fiddle.h>
VALUE rb_cHandle;
struct dl_handle {
void *ptr;
int open;
int enable_close;
};
#if defined(_WIN32)
#if !defined(_WIN32_WCE)
static void *
w32_coredll(void)
{
MEMORY_BASIC_INFORMATION m;
memset(&m, 0, sizeof(m));
if( !VirtualQuery(_errno, &m, sizeof(m)) ) return NULL;
return m.AllocationBase;
}
#endif
static int
w32_dlclose(void *ptr)
{
#if !defined(_WIN32_WCE)
if( ptr == w32_coredll() ) return 0;
#endif
if( FreeLibrary((HMODULE)ptr) ) return 0;
return errno = rb_w32_map_errno(GetLastError());
}
#define dlclose(ptr) w32_dlclose(ptr)
#endif
static void
fiddle_handle_free(void *ptr)
{
struct dl_handle *fiddle_handle = ptr;
if( fiddle_handle->ptr && fiddle_handle->open && fiddle_handle->enable_close ){
dlclose(fiddle_handle->ptr);
}
xfree(ptr);
}
static size_t
fiddle_handle_memsize(const void *ptr)
{
return sizeof(struct dl_handle);
}
static const rb_data_type_t fiddle_handle_data_type = {
"fiddle/handle",
{0, fiddle_handle_free, fiddle_handle_memsize,},
};
static VALUE
rb_fiddle_handle_close(VALUE self)
{
struct dl_handle *fiddle_handle;
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
if(fiddle_handle->open) {
int ret = dlclose(fiddle_handle->ptr);
fiddle_handle->open = 0;
if(ret) {
#if defined(HAVE_DLERROR)
rb_raise(rb_eFiddleError, "%s", dlerror());
#else
rb_raise(rb_eFiddleError, "could not close handle");
#endif
}
return INT2NUM(ret);
}
rb_raise(rb_eFiddleError, "dlclose() called too many times");
UNREACHABLE;
}
static VALUE
rb_fiddle_handle_s_allocate(VALUE klass)
{
VALUE obj;
struct dl_handle *fiddle_handle;
obj = TypedData_Make_Struct(rb_cHandle, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
fiddle_handle->ptr = 0;
fiddle_handle->open = 0;
fiddle_handle->enable_close = 0;
return obj;
}
static VALUE
predefined_fiddle_handle(void *handle)
{
VALUE obj = rb_fiddle_handle_s_allocate(rb_cHandle);
struct dl_handle *fiddle_handle = DATA_PTR(obj);
fiddle_handle->ptr = handle;
fiddle_handle->open = 1;
OBJ_FREEZE(obj);
return obj;
}
static VALUE
rb_fiddle_handle_initialize(int argc, VALUE argv[], VALUE self)
{
void *ptr;
struct dl_handle *fiddle_handle;
VALUE lib, flag;
char *clib;
int cflag;
const char *err;
switch( rb_scan_args(argc, argv, "02", &lib, &flag) ){
case 0:
clib = NULL;
cflag = RTLD_LAZY | RTLD_GLOBAL;
break;
case 1:
clib = NIL_P(lib) ? NULL : StringValueCStr(lib);
cflag = RTLD_LAZY | RTLD_GLOBAL;
break;
case 2:
clib = NIL_P(lib) ? NULL : StringValueCStr(lib);
cflag = NUM2INT(flag);
break;
default:
rb_bug("rb_fiddle_handle_new");
}
#if defined(_WIN32)
if( !clib ){
HANDLE rb_libruby_handle(void);
ptr = rb_libruby_handle();
}
else if( STRCASECMP(clib, "libc") == 0
#if defined(RUBY_COREDLL)
|| STRCASECMP(clib, RUBY_COREDLL) == 0
|| STRCASECMP(clib, RUBY_COREDLL".dll") == 0
#endif
){
#if defined(_WIN32_WCE)
ptr = dlopen("coredll.dll", cflag);
#else
(void)cflag;
ptr = w32_coredll();
#endif
}
else
#endif
ptr = dlopen(clib, cflag);
#if defined(HAVE_DLERROR)
if( !ptr && (err = dlerror()) ){
rb_raise(rb_eFiddleError, "%s", err);
}
#else
if( !ptr ){
err = dlerror();
rb_raise(rb_eFiddleError, "%s", err);
}
#endif
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
if( fiddle_handle->ptr && fiddle_handle->open && fiddle_handle->enable_close ){
dlclose(fiddle_handle->ptr);
}
fiddle_handle->ptr = ptr;
fiddle_handle->open = 1;
fiddle_handle->enable_close = 0;
if( rb_block_given_p() ){
rb_ensure(rb_yield, self, rb_fiddle_handle_close, self);
}
return Qnil;
}
static VALUE
rb_fiddle_handle_enable_close(VALUE self)
{
struct dl_handle *fiddle_handle;
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
fiddle_handle->enable_close = 1;
return Qnil;
}
static VALUE
rb_fiddle_handle_disable_close(VALUE self)
{
struct dl_handle *fiddle_handle;
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
fiddle_handle->enable_close = 0;
return Qnil;
}
static VALUE
rb_fiddle_handle_close_enabled_p(VALUE self)
{
struct dl_handle *fiddle_handle;
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
if(fiddle_handle->enable_close) return Qtrue;
return Qfalse;
}
static VALUE
rb_fiddle_handle_to_i(VALUE self)
{
struct dl_handle *fiddle_handle;
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
return PTR2NUM(fiddle_handle);
}
static VALUE fiddle_handle_sym(void *handle, VALUE symbol);
static VALUE
rb_fiddle_handle_sym(VALUE self, VALUE sym)
{
struct dl_handle *fiddle_handle;
TypedData_Get_Struct(self, struct dl_handle, &fiddle_handle_data_type, fiddle_handle);
if( ! fiddle_handle->open ){
rb_raise(rb_eFiddleError, "closed handle");
}
return fiddle_handle_sym(fiddle_handle->ptr, sym);
}
#if !defined(RTLD_NEXT)
#define RTLD_NEXT NULL
#endif
#if !defined(RTLD_DEFAULT)
#define RTLD_DEFAULT NULL
#endif
static VALUE
rb_fiddle_handle_s_sym(VALUE self, VALUE sym)
{
return fiddle_handle_sym(RTLD_NEXT, sym);
}
static VALUE
fiddle_handle_sym(void *handle, VALUE symbol)
{
#if defined(HAVE_DLERROR)
const char *err;
#define CHECK_DLERROR if ((err = dlerror()) != 0) { func = 0; }
#else
#define CHECK_DLERROR
#endif
void (*func)();
const char *name = StringValueCStr(symbol);
#if defined(HAVE_DLERROR)
dlerror();
#endif
func = (void (*)())(VALUE)dlsym(handle, name);
CHECK_DLERROR;
#if defined(FUNC_STDCALL)
if( !func ){
int i;
int len = (int)strlen(name);
char *name_n;
#if defined(__CYGWIN__) || defined(_WIN32) || defined(__MINGW32__)
{
char *name_a = (char*)xmalloc(len+2);
strcpy(name_a, name);
name_n = name_a;
name_a[len] = 'A';
name_a[len+1] = '\0';
func = dlsym(handle, name_a);
CHECK_DLERROR;
if( func ) goto found;
name_n = xrealloc(name_a, len+6);
}
#else
name_n = (char*)xmalloc(len+6);
#endif
memcpy(name_n, name, len);
name_n[len++] = '@';
for( i = 0; i < 256; i += 4 ){
sprintf(name_n + len, "%d", i);
func = dlsym(handle, name_n);
CHECK_DLERROR;
if( func ) break;
}
if( func ) goto found;
name_n[len-1] = 'A';
name_n[len++] = '@';
for( i = 0; i < 256; i += 4 ){
sprintf(name_n + len, "%d", i);
func = dlsym(handle, name_n);
CHECK_DLERROR;
if( func ) break;
}
found:
xfree(name_n);
}
#endif
if( !func ){
rb_raise(rb_eFiddleError, "unknown symbol \"%"PRIsVALUE"\"", symbol);
}
return PTR2NUM(func);
}
void
Init_fiddle_handle(void)
{
rb_cHandle = rb_define_class_under(mFiddle, "Handle", rb_cObject);
rb_define_alloc_func(rb_cHandle, rb_fiddle_handle_s_allocate);
rb_define_singleton_method(rb_cHandle, "sym", rb_fiddle_handle_s_sym, 1);
rb_define_singleton_method(rb_cHandle, "[]", rb_fiddle_handle_s_sym, 1);
rb_define_const(rb_cHandle, "NEXT", predefined_fiddle_handle(RTLD_NEXT));
rb_define_const(rb_cHandle, "DEFAULT", predefined_fiddle_handle(RTLD_DEFAULT));
rb_define_const(rb_cHandle, "RTLD_GLOBAL", INT2NUM(RTLD_GLOBAL));
rb_define_const(rb_cHandle, "RTLD_LAZY", INT2NUM(RTLD_LAZY));
rb_define_const(rb_cHandle, "RTLD_NOW", INT2NUM(RTLD_NOW));
rb_define_method(rb_cHandle, "initialize", rb_fiddle_handle_initialize, -1);
rb_define_method(rb_cHandle, "to_i", rb_fiddle_handle_to_i, 0);
rb_define_method(rb_cHandle, "close", rb_fiddle_handle_close, 0);
rb_define_method(rb_cHandle, "sym", rb_fiddle_handle_sym, 1);
rb_define_method(rb_cHandle, "[]", rb_fiddle_handle_sym, 1);
rb_define_method(rb_cHandle, "disable_close", rb_fiddle_handle_disable_close, 0);
rb_define_method(rb_cHandle, "enable_close", rb_fiddle_handle_enable_close, 0);
rb_define_method(rb_cHandle, "close_enabled?", rb_fiddle_handle_close_enabled_p, 0);
}
