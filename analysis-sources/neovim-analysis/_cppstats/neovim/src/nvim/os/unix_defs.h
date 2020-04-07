#include <unistd.h>
#include <sys/param.h>
#include <limits.h>
#define TEMP_DIR_NAMES { "$TMPDIR", "/tmp", ".", "~" }
#define TEMP_FILE_PATH_MAXLEN 256
#define HAVE_ACL (HAVE_POSIX_ACL || HAVE_SOLARIS_ACL)
#define SPECIAL_WILDCHAR "`'{"
#define ENV_SEPCHAR ':'
#define ENV_SEPSTR ":"
