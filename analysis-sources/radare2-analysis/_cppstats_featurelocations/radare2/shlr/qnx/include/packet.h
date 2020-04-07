
#if !defined(PACKET_H)
#define PACKET_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "libqnxr.h"
#include <stdio.h>
#if __WINDOWS__
#include <windows.h>
#if !__CYGWIN__ && !defined(MSC_VER)
#include <winsock.h>
#endif
#else
#include <unistd.h>
#endif

int qnxr_send_nak (libqnxr_t *instance);
int qnxr_send_ch_reset (libqnxr_t *instance);
int qnxr_send_ch_debug (libqnxr_t *instance);
int qnxr_send_ch_text (libqnxr_t *instance);






int qnxr_send_packet (libqnxr_t *instance);






int qnxr_read_packet (libqnxr_t *instance);

#endif
