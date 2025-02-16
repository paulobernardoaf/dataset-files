

















#if !defined(GRUB_UTIL_MISC_HEADER)
#define GRUB_UTIL_MISC_HEADER 1

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <unistd.h>

#include <config.h>
#include <grub/types.h>
#include <grub/symbol.h>
#include <grub/list.h>

#if defined(__NetBSD__)

#define DEFAULT_DIRECTORY "/burg"
#else
#define DEFAULT_DIRECTORY "/boot/burg"
#endif

#define DEFAULT_DEVICE_MAP DEFAULT_DIRECTORY "/device.map"

extern char *progname;
extern int verbosity;

void EXPORT_FUNC(grub_util_warn) (const char *fmt, ...);
void EXPORT_FUNC(grub_util_info) (const char *fmt, ...);
void EXPORT_FUNC(grub_util_error) (const char *fmt, ...) __attribute__ ((noreturn));

void *xmalloc (size_t size);
void *xrealloc (void *ptr, size_t size);
char *xstrdup (const char *str);
void *xmalloc_zero (size_t size);

void * grub_list_reverse (grub_list_t head);

char *grub_util_get_path (const char *dir, const char *file);
size_t grub_util_get_fp_size (FILE *fp);
size_t grub_util_get_image_size (const char *path);
void grub_util_read_at (void *img, size_t len, off_t offset, FILE *fp);
char *grub_util_read_image (const char *path);
void grub_util_load_image (const char *path, char *buf);
void grub_util_write_image (const char *img, size_t size, FILE *out);
void grub_util_write_image_at (const void *img, size_t size, off_t offset,
FILE *out);
char * grub_util_get_module_name (const char *str);
char * grub_util_get_module_path (const char *prefix, const char *str);

#if !defined(HAVE_VASPRINTF)

int vasprintf (char **buf, const char *fmt, va_list ap);

#endif

#if !defined(HAVE_ASPRINTF)

int asprintf (char **buf, const char *fmt, ...);

#endif

char *xasprintf (const char *fmt, ...);

#if defined(__MINGW32__)

#define fseeko fseeko64
#define ftello ftello64

void sync (void);
int fsync (int fno);
void sleep(int s);

grub_int64_t grub_util_get_disk_size (const char *name);

#define realpath(a, b) (char *) a

#endif


char *make_system_path_relative_to_its_root (const char *path);

char *canonicalize_file_name (const char *path);

void grub_util_init_nls (void);

#endif 
