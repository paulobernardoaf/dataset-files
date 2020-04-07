#include "common.h"
int vlc_demux_process_url(const struct vlc_run_args *, const char *url);
int vlc_demux_process_path(const struct vlc_run_args *, const char *path);
int vlc_demux_process_memory(const struct vlc_run_args *,
const unsigned char *buf, size_t length);
int libvlc_demux_process_memory(libvlc_instance_t *vlc,
const struct vlc_run_args *args,
const unsigned char *buf, size_t length);
