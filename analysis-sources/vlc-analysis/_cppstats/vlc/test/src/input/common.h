#include <vlc/vlc.h>
#if 0
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...) (void)0
#endif
struct vlc_run_args
{
const char *name;
unsigned verbose;
bool test_demux_controls;
};
void vlc_run_args_init(struct vlc_run_args *args);
libvlc_instance_t *libvlc_create(const struct vlc_run_args *args);
