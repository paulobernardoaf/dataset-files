





































#include "test-tool.h"
#include "git-compat-util.h"
#include <utime.h>

static const char usage_str[] =
"(-v|--verbose|-g|--get) (+|=|=+|=-|-)<seconds> <file>...";

static int timespec_arg(const char *arg, long int *set_time, int *set_eq)
{
char *test;
const char *timespec = arg;
*set_eq = (*timespec == '=') ? 1 : 0;
if (*set_eq) {
timespec++;
if (*timespec == '+') {
*set_eq = 2; 
timespec++;
}
}
*set_time = strtol(timespec, &test, 10);
if (*test) {
return 0;
}
if ((*set_eq && *set_time < 0) || *set_eq == 2) {
time_t now = time(NULL);
*set_time += now;
}
return 1;
}

int cmd__chmtime(int argc, const char **argv)
{
static int verbose;
static int get;

int i = 1;

int set_eq = 0;
long int set_time = 0;

if (argc < 3)
goto usage;

if (strcmp(argv[i], "--get") == 0 || strcmp(argv[i], "-g") == 0) {
get = 1;
++i;
} else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
verbose = 1;
++i;
}

if (i == argc) {
goto usage;
}

if (timespec_arg(argv[i], &set_time, &set_eq)) {
++i;
} else {
if (get == 0) {
fprintf(stderr, "Not a base-10 integer: %s\n", argv[i] + 1);
goto usage;
}
}

if (i == argc)
goto usage;

for (; i < argc; i++) {
struct stat sb;
struct utimbuf utb;
uintmax_t mtime;

if (stat(argv[i], &sb) < 0) {
fprintf(stderr, "Failed to stat %s: %s\n",
argv[i], strerror(errno));
return 1;
}

#if defined(GIT_WINDOWS_NATIVE)
if (!(sb.st_mode & S_IWUSR) &&
chmod(argv[i], sb.st_mode | S_IWUSR)) {
fprintf(stderr, "Could not make user-writable %s: %s",
argv[i], strerror(errno));
return 1;
}
#endif

utb.actime = sb.st_atime;
utb.modtime = set_eq ? set_time : sb.st_mtime + set_time;

mtime = utb.modtime < 0 ? 0: utb.modtime;
if (get) {
printf("%"PRIuMAX"\n", mtime);
} else if (verbose) {
printf("%"PRIuMAX"\t%s\n", mtime, argv[i]);
}

if (utb.modtime != sb.st_mtime && utime(argv[i], &utb) < 0) {
fprintf(stderr, "Failed to modify time on %s: %s\n",
argv[i], strerror(errno));
return 1;
}
}

return 0;

usage:
fprintf(stderr, "usage: %s %s\n", argv[0], usage_str);
return 1;
}
