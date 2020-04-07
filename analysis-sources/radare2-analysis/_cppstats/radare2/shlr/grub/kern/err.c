#include <grub/err.h>
#include <grub/misc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <grub/i18n.h>
GRUB_EXPORT(grub_errno);
GRUB_EXPORT(grub_errmsg);
GRUB_EXPORT(grub_error);
GRUB_EXPORT(grub_fatal);
GRUB_EXPORT(grub_error_push);
GRUB_EXPORT(grub_error_pop);
GRUB_EXPORT(grub_print_error);
GRUB_EXPORT(grub_err_printf);
#define GRUB_MAX_ERRMSG 256
#define GRUB_ERROR_STACK_SIZE 10
grub_err_t grub_errno;
char grub_errmsg[GRUB_MAX_ERRMSG];
static struct
{
grub_err_t no;
char errmsg[GRUB_MAX_ERRMSG];
} grub_error_stack_items[GRUB_ERROR_STACK_SIZE];
static int grub_error_stack_pos;
static int grub_error_stack_assert;
grub_err_t
grub_error (grub_err_t n, const char *fmt, ...)
{
va_list ap;
grub_errno = n;
va_start (ap, fmt);
grub_vsnprintf (grub_errmsg, sizeof (grub_errmsg), _(fmt), ap);
va_end (ap);
return n;
}
void
grub_fatal (const char *fmt, ...)
{
va_list ap;
va_start (ap, fmt);
grub_vprintf (_(fmt), ap);
va_end (ap);
abort ();
}
void
grub_error_push (void)
{
if (grub_error_stack_pos < GRUB_ERROR_STACK_SIZE)
{
grub_error_stack_items[grub_error_stack_pos].no = grub_errno;
grub_memcpy (grub_error_stack_items[grub_error_stack_pos].errmsg,
grub_errmsg,
sizeof (grub_errmsg));
grub_error_stack_pos++;
}
else
{
grub_error_stack_assert = 1;
}
grub_errno = GRUB_ERR_NONE;
}
int
grub_error_pop (void)
{
if (grub_error_stack_pos > 0)
{
grub_error_stack_pos--;
grub_errno = grub_error_stack_items[grub_error_stack_pos].no;
grub_memcpy (grub_errmsg,
grub_error_stack_items[grub_error_stack_pos].errmsg,
sizeof (grub_errmsg));
return 1;
}
else
{
grub_errno = GRUB_ERR_NONE;
return 0;
}
}
void
grub_print_error (void)
{
do
{
if (grub_errno != GRUB_ERR_NONE)
grub_err_printf (_("error: %s.\n"), grub_errmsg);
}
while (grub_error_pop ());
if (grub_error_stack_assert)
{
grub_err_printf ("assert: error stack overflow detected!\n");
grub_error_stack_assert = 0;
}
}
