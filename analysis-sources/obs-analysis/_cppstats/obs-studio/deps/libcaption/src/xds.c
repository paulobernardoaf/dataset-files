#include "xds.h"
#include "caption.h"
#include <string.h>
void xds_init(xds_t* xds)
{
memset(xds, 0, sizeof(xds_t));
}
int xds_decode(xds_t* xds, uint16_t cc)
{
switch (xds->state) {
default:
case 0:
xds_init(xds);
xds->class_code = (cc & 0x0F00) >> 8;
xds->type = (cc & 0x000F);
xds->state = 1;
return LIBCAPTION_OK;
case 1:
if (0x8F00 == (cc & 0xFF00)) {
xds->checksum = (cc & 0x007F);
xds->state = 0;
return LIBCAPTION_READY;
}
if (xds->size < 32) {
xds->content[xds->size + 0] = (cc & 0x7F00) >> 8;
xds->content[xds->size + 1] = (cc & 0x007F);
xds->size += 2;
return LIBCAPTION_OK;
}
}
xds->state = 0;
return LIBCAPTION_ERROR;
}
