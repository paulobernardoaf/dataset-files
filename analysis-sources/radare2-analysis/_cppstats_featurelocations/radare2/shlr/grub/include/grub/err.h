


















#if !defined(GRUB_ERR_HEADER)
#define GRUB_ERR_HEADER 1

#include <grub/symbol.h>

typedef enum
{
GRUB_ERR_NONE = 0,
GRUB_ERR_TEST_FAILURE,
GRUB_ERR_BAD_MODULE,
GRUB_ERR_OUT_OF_MEMORY,
GRUB_ERR_BAD_FILE_TYPE,
GRUB_ERR_FILE_NOT_FOUND,
GRUB_ERR_FILE_READ_ERROR,
GRUB_ERR_BAD_FILENAME,
GRUB_ERR_UNKNOWN_FS,
GRUB_ERR_BAD_FS,
GRUB_ERR_BAD_NUMBER,
GRUB_ERR_OUT_OF_RANGE,
GRUB_ERR_UNKNOWN_DEVICE,
GRUB_ERR_BAD_DEVICE,
GRUB_ERR_READ_ERROR,
GRUB_ERR_WRITE_ERROR,
GRUB_ERR_UNKNOWN_COMMAND,
GRUB_ERR_INVALID_COMMAND,
GRUB_ERR_BAD_ARGUMENT,
GRUB_ERR_BAD_PART_TABLE,
GRUB_ERR_UNKNOWN_OS,
GRUB_ERR_BAD_OS,
GRUB_ERR_NO_KERNEL,
GRUB_ERR_BAD_FONT,
GRUB_ERR_NOT_IMPLEMENTED_YET,
GRUB_ERR_SYMLINK_LOOP,
GRUB_ERR_BAD_GZIP_DATA,
GRUB_ERR_MENU,
GRUB_ERR_TIMEOUT,
GRUB_ERR_IO,
GRUB_ERR_ACCESS_DENIED,
GRUB_ERR_MENU_ESCAPE
}
grub_err_t;

extern grub_err_t grub_errno;
extern char grub_errmsg[];

grub_err_t grub_error (grub_err_t n, const char *fmt, ...);
void grub_fatal (const char *fmt, ...);
void grub_error_push (void);
int grub_error_pop (void);
void grub_print_error (void);
#if defined(_MSC_VER)
int grub_err_printf (const char *fmt, ...);
#else
int grub_err_printf (const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
#endif
#endif 
