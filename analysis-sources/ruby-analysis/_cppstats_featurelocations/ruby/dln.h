










#if !defined(DLN_H)
#define DLN_H
#include "ruby/defines.h" 

#if defined(__cplusplus)
#if !defined(HAVE_PROTOTYPES)
#define HAVE_PROTOTYPES 1
#endif
#if !defined(HAVE_STDARG_PROTOTYPES)
#define HAVE_STDARG_PROTOTYPES 1
#endif
#endif

#undef _
#if defined(HAVE_PROTOTYPES)
#define _(args) args
#else
#define _(args) ()
#endif

RUBY_SYMBOL_EXPORT_BEGIN

#if !defined(DLN_FIND_EXTRA_ARG)
#define DLN_FIND_EXTRA_ARG
#endif
#if !defined(DLN_FIND_EXTRA_ARG_DECL)
#define DLN_FIND_EXTRA_ARG_DECL
#endif

char *dln_find_exe_r(const char*,const char*,char*,size_t DLN_FIND_EXTRA_ARG_DECL);
char *dln_find_file_r(const char*,const char*,char*,size_t DLN_FIND_EXTRA_ARG_DECL);

#if defined(USE_DLN_A_OUT)
extern char *dln_argv0;
#endif

void *dln_load(const char*);

RUBY_SYMBOL_EXPORT_END

#endif
