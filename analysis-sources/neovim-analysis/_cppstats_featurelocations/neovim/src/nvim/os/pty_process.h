#if !defined(NVIM_OS_PTY_PROCESS_H)
#define NVIM_OS_PTY_PROCESS_H

#if defined(WIN32)
#include "nvim/os/pty_process_win.h"
#else
#include "nvim/os/pty_process_unix.h"
#endif
#endif 
