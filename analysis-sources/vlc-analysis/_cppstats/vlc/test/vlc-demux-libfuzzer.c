#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "src/input/demux-run.h"
int LLVMFuzzerInitialize(int *argc, char ***argv);
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
static struct vlc_run_args args;
static libvlc_instance_t *vlc;
int LLVMFuzzerInitialize(int *argc, char ***argv)
{
(void) argc; (void) argv;
vlc_run_args_init(&args);
vlc = libvlc_create(&args);
return vlc ? 0 : -1;
}
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
libvlc_demux_process_memory(vlc, &args, data, size);
return 0;
}
