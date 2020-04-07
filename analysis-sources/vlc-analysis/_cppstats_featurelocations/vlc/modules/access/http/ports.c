



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
#include "transport.h"
#include <vlc_common.h>


static const unsigned short blocked_ports[] = {
1, 
7, 
9, 
11, 
13, 
15, 
17, 
19, 
20, 
21, 
22, 
23, 
25, 
37, 
42, 
43, 
53, 
77, 
79, 
87, 
95, 
101, 
102, 
103, 
104, 
109, 
110, 
111, 
113, 
115, 
117, 
119, 
123, 
135, 
139, 
143, 
179, 
389, 
465, 
512, 
513, 
514, 
515, 
526, 
530, 
531, 
532, 
540, 
556, 
563, 
587, 
601, 
636, 
993, 
995, 
2049, 
3659, 
4045, 
6000, 
6665, 
6666, 
6667, 
6668, 
6669, 
};

static int portcmp(const void *key, const void *entry)
{
const unsigned *port = key;
const unsigned short *blocked_port = entry;

return ((int)*port) - ((int)*blocked_port);
}

bool vlc_http_port_blocked(unsigned port)
{
if (port > 0xffff)
return true;

return bsearch(&port, blocked_ports, ARRAY_SIZE(blocked_ports),
sizeof (unsigned short), portcmp) != NULL;
}
