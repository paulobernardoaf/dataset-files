







#include "lv_color.h"
#include "lv_math.h"




































lv_color_t lv_color_hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
h = (uint32_t)((uint32_t)h * 255) / 360;
s = (uint16_t)((uint16_t)s * 255) / 100;
v = (uint16_t)((uint16_t)v * 255) / 100;

uint8_t r, g, b;

uint8_t region, remainder, p, q, t;

if(s == 0) {
r = v;
g = v;
b = v;
return lv_color_make(v, v, v);
}

region = h / 43;
remainder = (h - (region * 43)) * 6;

p = (v * (255 - s)) >> 8;
q = (v * (255 - ((s * remainder) >> 8))) >> 8;
t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

switch(region) {
case 0:
r = v;
g = t;
b = p;
break;
case 1:
r = q;
g = v;
b = p;
break;
case 2:
r = p;
g = v;
b = t;
break;
case 3:
r = p;
g = q;
b = v;
break;
case 4:
r = t;
g = p;
b = v;
break;
default:
r = v;
g = p;
b = q;
break;
}

lv_color_t result = lv_color_make(r, g, b);
return result;
}








lv_color_hsv_t lv_color_rgb_to_hsv(uint8_t r8, uint8_t g8, uint8_t b8)
{
uint16_t r = ((uint32_t)r8 << 10) / 255;
uint16_t g = ((uint32_t)g8 << 10) / 255;
uint16_t b = ((uint32_t)b8 << 10) / 255;

uint16_t rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
uint16_t rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

lv_color_hsv_t hsv;


hsv.v = (100 * rgbMax) >> 10;

int32_t delta = rgbMax - rgbMin;
if (LV_MATH_ABS(delta) < 3) {
hsv.h = 0;
hsv.s = 0;
return hsv;
}


hsv.s = 100 * delta / rgbMax;
if(hsv.s < 3) {
hsv.h = 0;
return hsv;
}


int32_t h;
if(rgbMax == r)
h = (((g - b) << 10) / delta) + (g < b ? (6 << 10) : 0); 
else if(rgbMax == g)
h = (((b - r) << 10) / delta) + (2 << 10); 
else if(rgbMax == b)
h = (((r - g) << 10) / delta) + (4 << 10); 
else
h = 0;
h *= 60;
h >>= 10;
if (h < 0) h += 360;

hsv.h = h;
return hsv;
}






lv_color_hsv_t lv_color_to_hsv(lv_color_t color)
{
lv_color32_t color32;
color32.full = lv_color_to32(color);
return lv_color_rgb_to_hsv(color32.ch.red, color32.ch.green, color32.ch.blue);
}
