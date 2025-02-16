







#include "lv_math.h"
#include <stdbool.h>
#include <stdlib.h>
















static const int16_t sin0_90_table[] = {
0, 572, 1144, 1715, 2286, 2856, 3425, 3993, 4560, 5126, 5690, 6252, 6813, 7371, 7927, 8481,
9032, 9580, 10126, 10668, 11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762, 32767};














int16_t lv_trigo_sin(int16_t angle)
{
int16_t ret = 0;
angle = angle % 360;

if(angle < 0) angle = 360 + angle;

if(angle < 90) {
ret = sin0_90_table[angle];
} else if(angle >= 90 && angle < 180) {
angle = 180 - angle;
ret = sin0_90_table[angle];
} else if(angle >= 180 && angle < 270) {
angle = angle - 180;
ret = -sin0_90_table[angle];
} else { 
angle = 360 - angle;
ret = -sin0_90_table[angle];
}

return ret;
}










int32_t lv_bezier3(uint32_t t, int32_t u0, int32_t u1, int32_t u2, int32_t u3)
{
uint32_t t_rem = 1024 - t;
uint32_t t_rem2 = (t_rem * t_rem) >> 10;
uint32_t t_rem3 = (t_rem2 * t_rem) >> 10;
uint32_t t2 = (t * t) >> 10;
uint32_t t3 = (t2 * t) >> 10;

uint32_t v1 = ((uint32_t)t_rem3 * u0) >> 10;
uint32_t v2 = ((uint32_t)3 * t_rem2 * t * u1) >> 20;
uint32_t v3 = ((uint32_t)3 * t_rem * t2 * u2) >> 20;
uint32_t v4 = ((uint32_t)t3 * u3) >> 10;

return v1 + v2 + v3 + v4;
}







uint16_t lv_atan2(int x, int y)
{










unsigned char negflag;
unsigned char tempdegree;
unsigned char comp;
unsigned int degree; 


unsigned int ux;
unsigned int uy;


negflag = 0;
if(x < 0) {
negflag += 0x01; 
x = (0 - x); 
}
ux = x; 
if(y < 0) {
negflag += 0x02; 
y = (0 - y); 
}
uy = y; 


if(ux > uy) {
degree = (uy * 45) / ux; 
negflag += 0x10; 
} else {
degree = (ux * 45) / uy; 
}


comp = 0;
tempdegree = degree; 
if(tempdegree > 22) { 
if(tempdegree <= 44) comp++;
if(tempdegree <= 41) comp++;
if(tempdegree <= 37) comp++;
if(tempdegree <= 32) comp++; 
} else { 
if(tempdegree >= 2) comp++;
if(tempdegree >= 6) comp++;
if(tempdegree >= 10) comp++;
if(tempdegree >= 15) comp++; 
}
degree += comp; 


if(negflag & 0x10) degree = (90 - degree);



if(negflag & 0x02) { 
if(negflag & 0x01) 
degree = (180 + degree);
else 
degree = (180 - degree);
} else { 
if(negflag & 0x01) 
degree = (360 - degree);
}
return degree;
}






uint32_t lv_sqrt(uint32_t num)
{

uint32_t root = 0;
uint32_t place = 0x40000000;

while(place > num) place >>= 2;
while(place) {
if(num >= root + place) {
num -= root + place;
root += (place << 1);
}
root >>= 1;
place >>= 2;
}
return root;
}




