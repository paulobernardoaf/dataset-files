#include "tuklib_common.h"
#include <stdlib.h>
#include <stdio.h>
#include "tuklib_gettext.h"
#include "tuklib_progname.h"
#include "tuklib_exit.h"
extern void
tuklib_exit(int status, int err_status, int show_error)
{
if (status != err_status) {
const int ferror_err = ferror(stdout);
const int fclose_err = fclose(stdout);
if (ferror_err || fclose_err) {
status = err_status;
if (show_error)
fprintf(stderr, "%s: %s: %s\n", progname,
_("Writing to standard "
"output failed"),
fclose_err ? strerror(errno)
: _("Unknown error"));
}
}
if (status != err_status) {
const int ferror_err = ferror(stderr);
const int fclose_err = fclose(stderr);
if (fclose_err || ferror_err)
status = err_status;
}
exit(status);
}
