#include "rebase.h"
#include "config.h"
enum rebase_type rebase_parse_value(const char *value)
{
int v = git_parse_maybe_bool(value);
if (!v)
return REBASE_FALSE;
else if (v > 0)
return REBASE_TRUE;
else if (!strcmp(value, "preserve") || !strcmp(value, "p"))
return REBASE_PRESERVE;
else if (!strcmp(value, "merges") || !strcmp(value, "m"))
return REBASE_MERGES;
else if (!strcmp(value, "interactive") || !strcmp(value, "i"))
return REBASE_INTERACTIVE;
return REBASE_INVALID;
}
