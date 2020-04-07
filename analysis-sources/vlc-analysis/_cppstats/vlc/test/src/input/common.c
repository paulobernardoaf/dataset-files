#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include "../lib/libvlc_internal.h"
#include "common.h"
static inline int getenv_atoi(const char *name)
{
char *env = getenv(name);
return env ? atoi(env) : 0;
}
void vlc_run_args_init(struct vlc_run_args *args)
{
memset(args, 0, sizeof(struct vlc_run_args));
args->verbose = getenv_atoi("V");
if (args->verbose >= 10)
args->verbose = 9;
args->name = getenv("VLC_TARGET");
args->test_demux_controls = getenv_atoi("VLC_DEMUX_CONTROLS");
}
libvlc_instance_t *libvlc_create(const struct vlc_run_args *args)
{
#if defined(TOP_BUILDDIR)
#if !defined(HAVE_STATIC_MODULES)
setenv("VLC_PLUGIN_PATH", TOP_BUILDDIR"/modules", 1);
#endif
setenv("VLC_DATA_PATH", TOP_SRCDIR"/share", 1);
#endif
const char *argv[2];
char verbose[2];
int argc = args->verbose == 0 ? 1 : 2;
if (args->verbose > 0)
{
argv[0] = "--verbose";
sprintf(verbose, "%u", args->verbose);
argv[1] = verbose;
}
else
argv[0] = "--quiet";
libvlc_instance_t *vlc = libvlc_new(argc, argv);
if (vlc == NULL)
fprintf(stderr, "Error: cannot initialize LibVLC.\n");
return vlc;
}
