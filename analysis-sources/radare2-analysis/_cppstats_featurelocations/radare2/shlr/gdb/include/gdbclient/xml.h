

#if !defined(GDBCLIENT_XML_H)
#define GDBCLIENT_XML_H

#include "libgdbr.h"

int gdbr_read_target_xml(libgdbr_t *g);
int gdbr_read_processes_xml(libgdbr_t *g, int pid, RList* list);

#endif 
