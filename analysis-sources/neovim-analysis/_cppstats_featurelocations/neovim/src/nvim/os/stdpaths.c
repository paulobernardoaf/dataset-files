


#include <stdbool.h>

#include "nvim/os/stdpaths_defs.h"
#include "nvim/os/os.h"
#include "nvim/path.h"
#include "nvim/memory.h"
#include "nvim/ascii.h"


static const char *xdg_env_vars[] = {
[kXDGConfigHome] = "XDG_CONFIG_HOME",
[kXDGDataHome] = "XDG_DATA_HOME",
[kXDGCacheHome] = "XDG_CACHE_HOME",
[kXDGRuntimeDir] = "XDG_RUNTIME_DIR",
[kXDGConfigDirs] = "XDG_CONFIG_DIRS",
[kXDGDataDirs] = "XDG_DATA_DIRS",
};

#if defined(WIN32)
static const char *const xdg_defaults_env_vars[] = {
[kXDGConfigHome] = "LOCALAPPDATA",
[kXDGDataHome] = "LOCALAPPDATA",
[kXDGCacheHome] = "TEMP",
[kXDGRuntimeDir] = NULL,
[kXDGConfigDirs] = NULL,
[kXDGDataDirs] = NULL,
};
#endif




static const char *const xdg_defaults[] = {
#if defined(WIN32)
[kXDGConfigHome] = "~\\AppData\\Local",
[kXDGDataHome] = "~\\AppData\\Local",
[kXDGCacheHome] = "~\\AppData\\Local\\Temp",
[kXDGRuntimeDir] = NULL,
[kXDGConfigDirs] = NULL,
[kXDGDataDirs] = NULL,
#else
[kXDGConfigHome] = "~/.config",
[kXDGDataHome] = "~/.local/share",
[kXDGCacheHome] = "~/.cache",
[kXDGRuntimeDir] = NULL,
[kXDGConfigDirs] = "/etc/xdg/",
[kXDGDataDirs] = "/usr/local/share/:/usr/share/",
#endif
};






char *stdpaths_get_xdg_var(const XDGVarType idx)
FUNC_ATTR_WARN_UNUSED_RESULT
{
const char *const env = xdg_env_vars[idx];
const char *const fallback = xdg_defaults[idx];

const char *env_val = os_getenv(env);

#if defined(WIN32)
if (env_val == NULL && xdg_defaults_env_vars[idx] != NULL) {
env_val = os_getenv(xdg_defaults_env_vars[idx]);
}
#else
if (env_val == NULL && os_env_exists(env)) {
env_val = "";
}
#endif

char *ret = NULL;
if (env_val != NULL) {
ret = xstrdup(env_val);
} else if (fallback) {
ret = (char *)expand_env_save((char_u *)fallback);
}

return ret;
}









char *get_xdg_home(const XDGVarType idx)
FUNC_ATTR_WARN_UNUSED_RESULT
{
char *dir = stdpaths_get_xdg_var(idx);
if (dir) {
#if defined(WIN32)
dir = concat_fnames_realloc(dir,
(idx == kXDGDataHome ? "nvim-data" : "nvim"),
true);
#else
dir = concat_fnames_realloc(dir, "nvim", true);
#endif
}
return dir;
}






char *stdpaths_user_conf_subpath(const char *fname)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
return concat_fnames_realloc(get_xdg_home(kXDGConfigHome), fname, true);
}








char *stdpaths_user_data_subpath(const char *fname,
const size_t trailing_pathseps,
const bool escape_commas)
FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
char *ret = concat_fnames_realloc(get_xdg_home(kXDGDataHome), fname, true);
const size_t len = strlen(ret);
const size_t numcommas = (escape_commas ? memcnt(ret, ',', len) : 0);
if (numcommas || trailing_pathseps) {
ret = xrealloc(ret, len + trailing_pathseps + numcommas + 1);
for (size_t i = 0 ; i < len + numcommas ; i++) {
if (ret[i] == ',') {
memmove(ret + i + 1, ret + i, len - i + numcommas);
ret[i] = '\\';
i++;
}
}
if (trailing_pathseps) {
memset(ret + len + numcommas, PATHSEP, trailing_pathseps);
}
ret[len + trailing_pathseps + numcommas] = NUL;
}
return ret;
}
