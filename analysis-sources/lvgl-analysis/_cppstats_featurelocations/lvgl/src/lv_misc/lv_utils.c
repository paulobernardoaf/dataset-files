







#include <stdbool.h>

#include "lv_utils.h"
#include "lv_math.h"































char * lv_utils_num_to_str(int32_t num, char * buf)
{
if(num == 0) {
buf[0] = '0';
buf[1] = '\0';
return buf;
}
int8_t digitCount = 0;
int8_t i = 0;
if(num < 0) {
buf[digitCount++] = '-';
num = LV_MATH_ABS(num);
++i;
}
while(num) {
char digit = num % 10;
buf[digitCount++] = digit + 48;
num /= 10;
}
buf[digitCount] = '\0';
digitCount--;
while(digitCount > i) {
char temp = buf[i];
buf[i] = buf[digitCount];
buf[digitCount] = temp;
digitCount--;
i++;
}
return buf;
}


















void * lv_utils_bsearch(const void * key, const void * base, uint32_t n, uint32_t size,
int32_t (*cmp)(const void * pRef, const void * pElement))
{
const char * middle;
int32_t c;

for(middle = base; n != 0;) {
middle += (n / 2) * size;
if((c = (*cmp)(key, middle)) > 0) {
n = (n / 2) - ((n & 1) == 0);
base = (middle += size);
} else if(c < 0) {
n /= 2;
middle = base;
} else {
return (char *)middle;
}
}
return NULL;
}




