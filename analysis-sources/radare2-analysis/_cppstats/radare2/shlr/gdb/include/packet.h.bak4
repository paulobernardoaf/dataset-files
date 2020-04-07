
#if !defined(PACKET_H)
#define PACKET_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "libgdbr.h"
#include <stdio.h>
#if __WINDOWS__
#include <windows.h>
#if !defined(_MSC_VER)
#include <winsock.h>
#endif
#endif





int send_packet(libgdbr_t *g);







int read_packet(libgdbr_t *g, bool vcont);

int pack(libgdbr_t *g, const char *msg);

#endif
