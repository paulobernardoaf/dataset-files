#include "git-compat-util.h"
#include "trace2.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned short int all_variables;
static void print_variables (const char *string);
static void note_variables (const char *string);
static void subst_from_stdin (void);
int
cmd_main (int argc, const char *argv[])
{
trace2_cmd_name("sh-i18n--envsubst");
switch (argc)
{
case 1:
error ("we won't substitute all variables on stdin for you");
break;
case 2:
all_variables = 0;
note_variables (argv[1]);
subst_from_stdin ();
break;
case 3:
if (strcmp(argv[1], "--variables"))
error ("first argument must be --variables when two are given");
print_variables (argv[2]);
break;
default:
error ("too many arguments");
break;
}
errno = 0;
if (ferror (stderr) || fflush (stderr))
{
fclose (stderr);
exit (EXIT_FAILURE);
}
if (fclose (stderr) && errno != EBADF)
exit (EXIT_FAILURE);
exit (EXIT_SUCCESS);
}
static void
find_variables (const char *string,
void (*callback) (const char *var_ptr, size_t var_len))
{
for (; *string != '\0';)
if (*string++ == '$')
{
const char *variable_start;
const char *variable_end;
unsigned short int valid;
char c;
if (*string == '{')
string++;
variable_start = string;
c = *string;
if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
{
do
c = *++string;
while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
|| (c >= '0' && c <= '9') || c == '_');
variable_end = string;
if (variable_start[-1] == '{')
{
if (*string == '}')
{
string++;
valid = 1;
}
else
valid = 0;
}
else
valid = 1;
if (valid)
callback (variable_start, variable_end - variable_start);
}
}
}
static void
print_variable (const char *var_ptr, size_t var_len)
{
fwrite (var_ptr, var_len, 1, stdout);
putchar ('\n');
}
static void
print_variables (const char *string)
{
find_variables (string, &print_variable);
}
typedef struct string_list_ty string_list_ty;
struct string_list_ty
{
const char **item;
size_t nitems;
size_t nitems_max;
};
static inline void
string_list_init (string_list_ty *slp)
{
slp->item = NULL;
slp->nitems = 0;
slp->nitems_max = 0;
}
static inline void
string_list_append (string_list_ty *slp, const char *s)
{
if (slp->nitems >= slp->nitems_max)
{
slp->nitems_max = slp->nitems_max * 2 + 4;
REALLOC_ARRAY(slp->item, slp->nitems_max);
}
slp->item[slp->nitems++] = s;
}
static int
cmp_string (const void *pstr1, const void *pstr2)
{
const char *str1 = *(const char **)pstr1;
const char *str2 = *(const char **)pstr2;
return strcmp (str1, str2);
}
static inline void
string_list_sort (string_list_ty *slp)
{
QSORT(slp->item, slp->nitems, cmp_string);
}
static int
sorted_string_list_member (const string_list_ty *slp, const char *s)
{
size_t j1, j2;
j1 = 0;
j2 = slp->nitems;
if (j2 > 0)
{
while (j2 - j1 > 1)
{
size_t j = j1 + ((j2 - j1) >> 1);
int result = strcmp (slp->item[j], s);
if (result > 0)
j2 = j;
else if (result == 0)
return 1;
else
j1 = j + 1;
}
if (j2 > j1)
if (strcmp (slp->item[j1], s) == 0)
return 1;
}
return 0;
}
static string_list_ty variables_set;
static void
note_variable (const char *var_ptr, size_t var_len)
{
char *string = xmemdupz (var_ptr, var_len);
string_list_append (&variables_set, string);
}
static void
note_variables (const char *string)
{
string_list_init (&variables_set);
find_variables (string, &note_variable);
string_list_sort (&variables_set);
}
static int
do_getc (void)
{
int c = getc (stdin);
if (c == EOF)
{
if (ferror (stdin))
error ("error while reading standard input");
}
return c;
}
static inline void
do_ungetc (int c)
{
if (c != EOF)
ungetc (c, stdin);
}
static void
subst_from_stdin (void)
{
static char *buffer;
static size_t bufmax;
static size_t buflen;
int c;
for (;;)
{
c = do_getc ();
if (c == EOF)
break;
if (c == '$')
{
unsigned short int opening_brace = 0;
unsigned short int closing_brace = 0;
c = do_getc ();
if (c == '{')
{
opening_brace = 1;
c = do_getc ();
}
if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
{
unsigned short int valid;
buflen = 0;
do
{
if (buflen >= bufmax)
{
bufmax = 2 * bufmax + 10;
buffer = xrealloc (buffer, bufmax);
}
buffer[buflen++] = c;
c = do_getc ();
}
while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
|| (c >= '0' && c <= '9') || c == '_');
if (opening_brace)
{
if (c == '}')
{
closing_brace = 1;
valid = 1;
}
else
{
valid = 0;
do_ungetc (c);
}
}
else
{
valid = 1;
do_ungetc (c);
}
if (valid)
{
if (buflen >= bufmax)
{
bufmax = 2 * bufmax + 10;
buffer = xrealloc (buffer, bufmax);
}
buffer[buflen] = '\0';
if (!all_variables
&& !sorted_string_list_member (&variables_set, buffer))
valid = 0;
}
if (valid)
{
const char *env_value = getenv (buffer);
if (env_value != NULL)
fputs (env_value, stdout);
}
else
{
putchar ('$');
if (opening_brace)
putchar ('{');
fwrite (buffer, buflen, 1, stdout);
if (closing_brace)
putchar ('}');
}
}
else
{
do_ungetc (c);
putchar ('$');
if (opening_brace)
putchar ('{');
}
}
else
putchar (c);
}
}
