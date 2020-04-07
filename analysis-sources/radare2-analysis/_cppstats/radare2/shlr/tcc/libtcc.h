#if !defined(LIBTCCAPI)
#define LIBTCCAPI
#endif
#if defined(__cplusplus)
extern "C" {
#endif
struct TCCState;
typedef struct TCCState TCCState;
LIBTCCAPI TCCState *tcc_new(const char* arch, int bits, const char *os);
LIBTCCAPI void tcc_delete(TCCState *s);
LIBTCCAPI void tcc_set_lib_path(TCCState *s, const char *path);
LIBTCCAPI void tcc_set_error_func(TCCState *s, void *error_opaque,
void (*error_func)(void *opaque, const char *msg));
LIBTCCAPI int tcc_set_options(TCCState *s, const char *str);
LIBTCCAPI int tcc_add_include_path(TCCState *s, const char *pathname);
LIBTCCAPI int tcc_add_sysinclude_path(TCCState *s, const char *pathname);
LIBTCCAPI void tcc_define_symbol(TCCState *s, const char *sym, const char *value);
LIBTCCAPI void tcc_undefine_symbol(TCCState *s, const char *sym);
LIBTCCAPI int tcc_add_file(TCCState *s, const char *filename, const char *dir);
LIBTCCAPI int tcc_compile_string(TCCState *s, const char *buf);
LIBTCCAPI int tcc_set_output_type(TCCState *s, int output_type);
#define TCC_OUTPUT_MEMORY 0 
#define TCC_OUTPUT_EXE 1 
#define TCC_OUTPUT_DLL 2 
#define TCC_OUTPUT_OBJ 3 
#define TCC_OUTPUT_PREPROCESS 4 
LIBTCCAPI int tcc_add_library_path(TCCState *s, const char *pathname);
LIBTCCAPI int tcc_add_library(TCCState *s, const char *libraryname);
LIBTCCAPI int tcc_add_symbol(TCCState *s, const char *name, const void *val);
LIBTCCAPI int tcc_output_file(TCCState *s, const char *filename);
LIBTCCAPI int tcc_run(TCCState *s, int argc, char **argv);
LIBTCCAPI int tcc_relocate(TCCState *s1, void *ptr);
#define TCC_RELOCATE_AUTO (void*)1
LIBTCCAPI void *tcc_get_symbol(TCCState *s, const char *name);
extern char **tcc_cb_ptr;
void tcc_set_callback (TCCState *s, void (*cb)(const char *,char**), char **p);
#if defined(__cplusplus)
}
#endif
