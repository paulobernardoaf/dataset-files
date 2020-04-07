#include "libgdbr.h"
int handle_qSupported(libgdbr_t *g);
int send_msg(libgdbr_t* g, const char* msg);
int send_ack(libgdbr_t* g);
