

























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdio.h>
#include "src/input/demux-run.h"

int main(int argc, char *argv[])
{
const char *filename;
struct vlc_run_args args;
vlc_run_args_init(&args);

switch (argc)
{
case 2:
filename = argv[argc - 1];
break;
default:
fprintf(stderr, "Usage: [VLC_TARGET=demux] %s <filename>\n", argv[0]);
return 1;
}

return -vlc_demux_process_path(&args, filename);
}
