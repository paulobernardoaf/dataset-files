#if !defined(R2_LIB_H)
#define R2_LIB_H

#include "r_types.h"
#include "r_list.h"

#if __UNIX__
#include <dlfcn.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

R_LIB_VERSION_HEADER (r_lib);


#define R_LIB_SEPARATOR "."
#define R_LIB_SYMNAME "radare_plugin"
#define R_LIB_SYMFUNC "radare_plugin_function"

#define R_LIB_ENV "R2_LIBR_PLUGINS"


#if __WINDOWS__
#include <windows.h>
#define R_LIB_EXT "dll"
#elif __APPLE__
#define R_LIB_EXT "dylib"
#else
#define R_LIB_EXT "so"
#endif


typedef struct r_lib_plugin_t {
int type;
char *file;
void *data; 
struct r_lib_handler_t *handler;
void *dl_handler; 
char *author;
char *version;
void (*free)(void *data);
} RLibPlugin;


typedef struct r_lib_handler_t {
int type;
char desc[128]; 
void *user; 
int (*constructor)(RLibPlugin *, void *user, void *data);
int (*destructor)(RLibPlugin *, void *user, void *data);
} RLibHandler;



typedef struct r_lib_struct_t {
int type;
void *data; 
const char *version; 
void (*free)(void *data);
const char *pkgname; 
} RLibStruct;

typedef RLibStruct* (*RLibStructFunc) (void);


enum {
R_LIB_TYPE_IO, 
R_LIB_TYPE_DBG, 
R_LIB_TYPE_LANG, 
R_LIB_TYPE_ASM, 
R_LIB_TYPE_ANAL, 
R_LIB_TYPE_PARSE, 
R_LIB_TYPE_BIN, 
R_LIB_TYPE_BIN_XTR, 
R_LIB_TYPE_BIN_LDR, 
R_LIB_TYPE_BP, 
R_LIB_TYPE_SYSCALL, 
R_LIB_TYPE_FASTCALL,
R_LIB_TYPE_CRYPTO, 
R_LIB_TYPE_CORE, 
R_LIB_TYPE_EGG, 
R_LIB_TYPE_FS, 
R_LIB_TYPE_LAST
};

typedef struct r_lib_t {



char *symname;
char *symnamefunc;
RList *plugins;
RList *handlers;
} RLib;

#if defined(R_API)

R_API void *r_lib_dl_open(const char *libname);

R_API void *r_lib_dl_sym(void *handler, const char *name);
R_API int r_lib_dl_close(void *handler);


typedef int (*RLibCallback)(RLibPlugin *, void *, void *);
R_API RLib *r_lib_new(const char *symname, const char *symnamefunc);
R_API void r_lib_free(RLib *lib);
R_API int r_lib_run_handler(RLib *lib, RLibPlugin *plugin, RLibStruct *symbol);
R_API RLibHandler *r_lib_get_handler(RLib *lib, int type);
R_API int r_lib_open(RLib *lib, const char *file);
R_API bool r_lib_opendir(RLib *lib, const char *path);
R_API int r_lib_open_ptr (RLib *lib, const char *file, void *handler, RLibStruct *stru);
R_API char *r_lib_path(const char *libname);
R_API void r_lib_list(RLib *lib);
R_API bool r_lib_add_handler(RLib *lib, int type, const char *desc, RLibCallback ct, RLibCallback dt, void *user);
R_API bool r_lib_del_handler(RLib *lib, int type);
R_API int r_lib_close(RLib *lib, const char *file);

R_API const char *r_lib_types_get(int idx);
R_API int r_lib_types_get_i(const char *str);
#endif

#if defined(__cplusplus)
}
#endif

#endif
