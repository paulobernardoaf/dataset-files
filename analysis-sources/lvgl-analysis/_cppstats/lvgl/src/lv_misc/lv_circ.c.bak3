









#include "lv_circ.h"































void lv_circ_init(lv_point_t * c, lv_coord_t * tmp, lv_coord_t radius)
{
c->x = radius;
c->y = 0;
*tmp = 1 - radius;
}






bool lv_circ_cont(lv_point_t * c)
{
return c->y <= c->x ? true : false;
}






void lv_circ_next(lv_point_t * c, lv_coord_t * tmp)
{
c->y++;

if(*tmp <= 0) {
(*tmp) += 2 * c->y + 1; 
} else {
c->x--;
(*tmp) += 2 * (c->y - c->x) + 1; 
}
}




