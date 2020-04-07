







#if defined(LV_CONF_INCLUDE_SIMPLE)
#include "lv_conf.h"
#else
#include "../../../lv_conf.h"
#endif

#include "lv_area.h"
#include "lv_math.h"

































void lv_area_set(lv_area_t * area_p, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2)
{
area_p->x1 = x1;
area_p->y1 = y1;
area_p->x2 = x2;
area_p->y2 = y2;
}






void lv_area_set_width(lv_area_t * area_p, lv_coord_t w)
{
area_p->x2 = area_p->x1 + w - 1;
}






void lv_area_set_height(lv_area_t * area_p, lv_coord_t h)
{
area_p->y2 = area_p->y1 + h - 1;
}







void lv_area_set_pos(lv_area_t * area_p, lv_coord_t x, lv_coord_t y)
{
lv_coord_t w = lv_area_get_width(area_p);
lv_coord_t h = lv_area_get_height(area_p);
area_p->x1 = x;
area_p->y1 = y;
lv_area_set_width(area_p, w);
lv_area_set_height(area_p, h);
}






uint32_t lv_area_get_size(const lv_area_t * area_p)
{
uint32_t size;

size = (uint32_t)(area_p->x2 - area_p->x1 + 1) * (area_p->y2 - area_p->y1 + 1);

return size;
}








bool lv_area_intersect(lv_area_t * res_p, const lv_area_t * a1_p, const lv_area_t * a2_p)
{

res_p->x1 = LV_MATH_MAX(a1_p->x1, a2_p->x1);
res_p->y1 = LV_MATH_MAX(a1_p->y1, a2_p->y1);
res_p->x2 = LV_MATH_MIN(a1_p->x2, a2_p->x2);
res_p->y2 = LV_MATH_MIN(a1_p->y2, a2_p->y2);


bool union_ok = true;
if((res_p->x1 > res_p->x2) || (res_p->y1 > res_p->y2)) {
union_ok = false;
}

return union_ok;
}






void lv_area_join(lv_area_t * a_res_p, const lv_area_t * a1_p, const lv_area_t * a2_p)
{
a_res_p->x1 = LV_MATH_MIN(a1_p->x1, a2_p->x1);
a_res_p->y1 = LV_MATH_MIN(a1_p->y1, a2_p->y1);
a_res_p->x2 = LV_MATH_MAX(a1_p->x2, a2_p->x2);
a_res_p->y2 = LV_MATH_MAX(a1_p->y2, a2_p->y2);
}







bool lv_area_is_point_on(const lv_area_t * a_p, const lv_point_t * p_p)
{
bool is_on = false;

if((p_p->x >= a_p->x1 && p_p->x <= a_p->x2) && ((p_p->y >= a_p->y1 && p_p->y <= a_p->y2))) {
is_on = true;
}

return is_on;
}







bool lv_area_is_on(const lv_area_t * a1_p, const lv_area_t * a2_p)
{
if((a1_p->x1 <= a2_p->x2) && (a1_p->x2 >= a2_p->x1) && (a1_p->y1 <= a2_p->y2) && (a1_p->y2 >= a2_p->y1)) {
return true;
} else {
return false;
}
}







bool lv_area_is_in(const lv_area_t * ain_p, const lv_area_t * aholder_p)
{
bool is_in = false;

if(ain_p->x1 >= aholder_p->x1 && ain_p->y1 >= aholder_p->y1 && ain_p->x2 <= aholder_p->x2 &&
ain_p->y2 <= aholder_p->y2) {
is_in = true;
}

return is_in;
}






void lv_area_increment(lv_area_t * a_p, const lv_coord_t amount)
{
a_p->x1 -= amount;
a_p->y1 -= amount;
a_p->x2 += amount;
a_p->y2 += amount;
}




