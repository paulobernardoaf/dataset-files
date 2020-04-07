#include "cache.h"
#include "config.h"
#include "dir.h"
#include "tr2_sysenv.h"
struct tr2_sysenv_entry {
const char *env_var_name;
const char *git_config_name;
char *value;
unsigned int getenv_called : 1;
};
static struct tr2_sysenv_entry tr2_sysenv_settings[] = {
[TR2_SYSENV_CFG_PARAM] = { "GIT_TRACE2_CONFIG_PARAMS",
"trace2.configparams" },
[TR2_SYSENV_DST_DEBUG] = { "GIT_TRACE2_DST_DEBUG",
"trace2.destinationdebug" },
[TR2_SYSENV_NORMAL] = { "GIT_TRACE2",
"trace2.normaltarget" },
[TR2_SYSENV_NORMAL_BRIEF] = { "GIT_TRACE2_BRIEF",
"trace2.normalbrief" },
[TR2_SYSENV_EVENT] = { "GIT_TRACE2_EVENT",
"trace2.eventtarget" },
[TR2_SYSENV_EVENT_BRIEF] = { "GIT_TRACE2_EVENT_BRIEF",
"trace2.eventbrief" },
[TR2_SYSENV_EVENT_NESTING] = { "GIT_TRACE2_EVENT_NESTING",
"trace2.eventnesting" },
[TR2_SYSENV_PERF] = { "GIT_TRACE2_PERF",
"trace2.perftarget" },
[TR2_SYSENV_PERF_BRIEF] = { "GIT_TRACE2_PERF_BRIEF",
"trace2.perfbrief" },
[TR2_SYSENV_MAX_FILES] = { "GIT_TRACE2_MAX_FILES",
"trace2.maxfiles" },
};
static int tr2_sysenv_cb(const char *key, const char *value, void *d)
{
int k;
if (!starts_with(key, "trace2."))
return 0;
for (k = 0; k < ARRAY_SIZE(tr2_sysenv_settings); k++) {
if (!strcmp(key, tr2_sysenv_settings[k].git_config_name)) {
free(tr2_sysenv_settings[k].value);
tr2_sysenv_settings[k].value = xstrdup(value);
return 0;
}
}
return 0;
}
void tr2_sysenv_load(void)
{
if (ARRAY_SIZE(tr2_sysenv_settings) != TR2_SYSENV_MUST_BE_LAST)
BUG("tr2_sysenv_settings size is wrong");
read_very_early_config(tr2_sysenv_cb, NULL);
}
const char *tr2_sysenv_get(enum tr2_sysenv_variable var)
{
if (var >= TR2_SYSENV_MUST_BE_LAST)
BUG("tr2_sysenv_get invalid var '%d'", var);
if (!tr2_sysenv_settings[var].getenv_called) {
const char *v = getenv(tr2_sysenv_settings[var].env_var_name);
if (v && *v) {
free(tr2_sysenv_settings[var].value);
tr2_sysenv_settings[var].value = xstrdup(v);
}
tr2_sysenv_settings[var].getenv_called = 1;
}
return tr2_sysenv_settings[var].value;
}
const char *tr2_sysenv_display_name(enum tr2_sysenv_variable var)
{
if (var >= TR2_SYSENV_MUST_BE_LAST)
BUG("tr2_sysenv_get invalid var '%d'", var);
return tr2_sysenv_settings[var].env_var_name;
}
void tr2_sysenv_release(void)
{
int k;
for (k = 0; k < ARRAY_SIZE(tr2_sysenv_settings); k++)
free(tr2_sysenv_settings[k].value);
}
