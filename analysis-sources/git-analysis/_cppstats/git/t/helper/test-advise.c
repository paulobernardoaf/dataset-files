#include "test-tool.h"
#include "cache.h"
#include "advice.h"
#include "config.h"
int cmd__advise_if_enabled(int argc, const char **argv)
{
if (!argv[1])
die("usage: %s <advice>", argv[0]);
setup_git_directory();
git_config(git_default_config, NULL);
advise_if_enabled(ADVICE_NESTED_TAG, argv[1]);
return 0;
}
