


































#if !defined(LIBIBERTY_H)
#define LIBIBERTY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ansidecl.h"


#include <stddef.h>

#include <stdarg.h>

#include <stdio.h>





extern void unlock_stream (FILE *);





extern void unlock_std_streams (void);





extern FILE *fopen_unlocked (const char *, const char *);
extern FILE *fdopen_unlocked (int, const char *);
extern FILE *freopen_unlocked (const char *, const char *, FILE *);




extern char **buildargv (const char *) ATTRIBUTE_MALLOC;



extern void freeargv (char **);




extern char **dupargv (char **) ATTRIBUTE_MALLOC;



extern void expandargv PARAMS ((int *, char ***));



extern int writeargv PARAMS ((char **, FILE *));











#if !HAVE_DECL_BASENAME
#if defined (__GNU_LIBRARY__ ) || defined (__linux__) || defined (__FreeBSD__) || defined (__OpenBSD__) || defined(__NetBSD__) || defined (__CYGWIN__) || defined (__CYGWIN32__) || defined (__MINGW32__) || defined (HAVE_DECL_BASENAME)
extern char *basename (const char *);
#else



#define basename basename_cannot_be_used_without_a_prototype
#endif
#endif



extern const char *lbasename (const char *);




extern const char *dos_lbasename (const char *);





extern const char *unix_lbasename (const char *);



extern char *lrealpath (const char *);





extern char *concat (const char *, ...) ATTRIBUTE_MALLOC ATTRIBUTE_SENTINEL;








extern char *reconcat (char *, const char *, ...) ATTRIBUTE_MALLOC ATTRIBUTE_SENTINEL;





extern unsigned long concat_length (const char *, ...) ATTRIBUTE_SENTINEL;






extern char *concat_copy (char *, const char *, ...) ATTRIBUTE_SENTINEL;






extern char *concat_copy2 (const char *, ...) ATTRIBUTE_SENTINEL;



extern char *libiberty_concat_ptr;





#define ACONCAT(ACONCAT_PARAMS) (libiberty_concat_ptr = (char *) alloca (concat_length ACONCAT_PARAMS + 1), concat_copy2 ACONCAT_PARAMS)





extern int fdmatch (int fd1, int fd2);




#if defined (HAVE_DECL_FFS) && !HAVE_DECL_FFS
extern int ffs(int);
#endif




extern char * getpwd (void);




#if defined(__MINGW32__)

struct timeval;
extern int gettimeofday (struct timeval *, void *); 
#endif



extern long get_run_time (void);




extern char *make_relative_prefix (const char *, const char *,
const char *) ATTRIBUTE_MALLOC;





extern char *make_relative_prefix_ignore_links (const char *, const char *,
const char *) ATTRIBUTE_MALLOC;



extern char *choose_temp_base (void) ATTRIBUTE_MALLOC;



extern char *make_temp_file (const char *) ATTRIBUTE_MALLOC;



extern int unlink_if_ordinary (const char *);



extern const char *spaces (int count);




extern int errno_max (void);




extern const char *strerrno (int);



extern int strtoerrno (const char *);



extern char *xstrerror (int);




extern int signo_max (void);











extern const char *strsigno (int);



extern int strtosigno (const char *);



extern int xatexit (void (*fn) (void));



extern void xexit (int status) ATTRIBUTE_NORETURN;



extern void xmalloc_set_program_name (const char *);


extern void xmalloc_failed (size_t) ATTRIBUTE_NORETURN;





extern void *xmalloc (size_t) ATTRIBUTE_MALLOC;





extern void *xrealloc (void *, size_t);




extern void *xcalloc (size_t, size_t) ATTRIBUTE_MALLOC;



extern char *xstrdup (const char *) ATTRIBUTE_MALLOC;



extern char *xstrndup (const char *, size_t) ATTRIBUTE_MALLOC;



extern void *xmemdup (const void *, size_t, size_t) ATTRIBUTE_MALLOC;


extern double physmem_total (void);
extern double physmem_available (void);


extern unsigned int xcrc32 (const unsigned char *, int, unsigned int);








#define XALLOCA(T) ((T *) alloca (sizeof (T)))
#define XNEW(T) ((T *) xmalloc (sizeof (T)))
#define XCNEW(T) ((T *) xcalloc (1, sizeof (T)))

#if defined(_MSC_VER)
#include <stdlib.h>
#define alloca(x) _alloca(x)
#define xmalloc malloc
#define xcalloc calloc
#define xrealloc realloc
#endif
#define XALLOCA(T) ((T *) alloca (sizeof (T)))
#define XNEW(T) ((T *) xmalloc (sizeof (T)))
#define XCNEW(T) ((T *) xcalloc (1, sizeof (T)))
#define XDUP(T, P) ((T *) xmemdup ((P), sizeof (T), sizeof (T)))
#define XDELETE(P) free ((void*) (P))



#define XALLOCAVEC(T, N) ((T *) alloca (sizeof (T) * (N)))
#define XNEWVEC(T, N) ((T *) xmalloc (sizeof (T) * (N)))
#define XCNEWVEC(T, N) ((T *) xcalloc ((N), sizeof (T)))
#define XDUPVEC(T, P, N) ((T *) xmemdup ((P), sizeof (T) * (N), sizeof (T) * (N)))
#define XRESIZEVEC(T, P, N) ((T *) xrealloc ((void *) (P), sizeof (T) * (N)))
#define XDELETEVEC(P) free ((void*) (P))



#define XALLOCAVAR(T, S) ((T *) alloca ((S)))
#define XNEWVAR(T, S) ((T *) xmalloc ((S)))
#define XCNEWVAR(T, S) ((T *) xcalloc (1, (S)))
#define XDUPVAR(T, P, S1, S2) ((T *) xmemdup ((P), (S1), (S2)))
#define XRESIZEVAR(T, P, S) ((T *) xrealloc ((P), (S)))



#define XOBNEW(O, T) ((T *) obstack_alloc ((O), sizeof (T)))
#define XOBNEWVEC(O, T, N) ((T *) obstack_alloc ((O), sizeof (T) * (N)))
#define XOBNEWVAR(O, T, S) ((T *) obstack_alloc ((O), (S)))
#define XOBFINISH(O, T) ((T) obstack_finish ((O)))



#define _hex_array_size 256
#define _hex_bad 99
extern const unsigned char _hex_value[_hex_array_size];
extern void hex_init (void);
#define hex_p(c) (hex_value (c) != _hex_bad)


#define hex_value(c) ((unsigned int) _hex_value[(unsigned char) (c)])




#define PEX_RECORD_TIMES 0x1


#define PEX_USE_PIPES 0x2


#define PEX_SAVE_TEMPS 0x4









extern struct pex_obj *pex_init (int flags, const char *pname,
const char *tempbase);








#define PEX_LAST 0x1


#define PEX_SEARCH 0x2


#define PEX_SUFFIX 0x4


#define PEX_STDERR_TO_STDOUT 0x8



#define PEX_BINARY_INPUT 0x10






#define PEX_BINARY_OUTPUT 0x20







#define PEX_STDERR_TO_PIPE 0x40



#define PEX_BINARY_ERROR 0x80




















































extern const char *pex_run (struct pex_obj *obj, int flags,
const char *executable, char * const *argv,
const char *outname, const char *errname,
int *err);











extern const char *pex_run_in_environment (struct pex_obj *obj, int flags,
const char *executable,
char * const *argv,
char * const *env,
const char *outname,
const char *errname, int *err);





extern FILE *pex_input_file (struct pex_obj *obj, int flags,
const char *in_name);






extern FILE *pex_input_pipe (struct pex_obj *obj, int binary);







extern FILE *pex_read_output (struct pex_obj *, int binary);







extern FILE *pex_read_err (struct pex_obj *, int binary);





extern int pex_get_status (struct pex_obj *, int count, int *vector);






struct pex_time
{
unsigned long user_seconds;
unsigned long user_microseconds;
unsigned long system_seconds;
unsigned long system_microseconds;
};

extern int pex_get_times (struct pex_obj *, int count,
struct pex_time *vector);




extern void pex_free (struct pex_obj *);












extern const char *pex_one (int flags, const char *executable,
char * const *argv, const char *pname,
const char *outname, const char *errname,
int *status, int *err);







#define PEXECUTE_FIRST 1
#define PEXECUTE_LAST 2
#define PEXECUTE_ONE (PEXECUTE_FIRST + PEXECUTE_LAST)
#define PEXECUTE_SEARCH 4
#define PEXECUTE_VERBOSE 8



extern int pexecute (const char *, char * const *, const char *,
const char *, char **, char **, int);



extern int pwait (int, int *, int);

#if !HAVE_DECL_ASPRINTF



extern int asprintf (char **, const char *, ...) ATTRIBUTE_PRINTF_2;
#endif

#if !HAVE_DECL_VASPRINTF



extern int vasprintf (char **, const char *, va_list) ATTRIBUTE_PRINTF(2,0);
#endif

#if defined(HAVE_DECL_SNPRINTF) && !HAVE_DECL_SNPRINTF

extern int snprintf (char *, size_t, const char *, ...) ATTRIBUTE_PRINTF_3;
#endif

#if defined(HAVE_DECL_VSNPRINTF) && !HAVE_DECL_VSNPRINTF

extern int vsnprintf (char *, size_t, const char *, va_list) ATTRIBUTE_PRINTF(3,0);
#endif

#if defined(HAVE_DECL_STRVERSCMP) && !HAVE_DECL_STRVERSCMP

extern int strverscmp (const char *, const char *);
#endif


extern void setproctitle (const char *name, ...);

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))







#if !defined(_MSC_VER)
extern void *_alloca (size_t) ATTRIBUTE_MALLOC;
#endif
#undef alloca
#if GCC_VERSION >= 2000 && !defined USE_C_ALLOCA
#define alloca(x) __builtin_alloca(x)
#undef C_ALLOCA
#define ASTRDUP(X) (__extension__ ({ const char *const libiberty_optr = (X); const unsigned long libiberty_len = strlen (libiberty_optr) + 1; char *const libiberty_nptr = (char *const) alloca (libiberty_len); (char *) memcpy (libiberty_nptr, libiberty_optr, libiberty_len); }))




#else
#define alloca(x) _alloca(x)
#undef USE_C_ALLOCA
#define USE_C_ALLOCA 1
#undef C_ALLOCA
#define C_ALLOCA 1
extern const char *libiberty_optr;
extern char *libiberty_nptr;
extern unsigned long libiberty_len;
#define ASTRDUP(X) (libiberty_optr = (X), libiberty_len = strlen (libiberty_optr) + 1, libiberty_nptr = (char *) alloca (libiberty_len), (char *) memcpy (libiberty_nptr, libiberty_optr, libiberty_len))




#endif

#if defined(__cplusplus)
}
#endif


#endif 
