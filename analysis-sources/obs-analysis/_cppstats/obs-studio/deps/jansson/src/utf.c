#include <string.h>
#include "utf.h"
int utf8_encode(int32_t codepoint, char *buffer, size_t *size)
{
if(codepoint < 0)
return -1;
else if(codepoint < 0x80)
{
buffer[0] = (char)codepoint;
*size = 1;
}
else if(codepoint < 0x800)
{
buffer[0] = 0xC0 + ((codepoint & 0x7C0) >> 6);
buffer[1] = 0x80 + ((codepoint & 0x03F));
*size = 2;
}
else if(codepoint < 0x10000)
{
buffer[0] = 0xE0 + ((codepoint & 0xF000) >> 12);
buffer[1] = 0x80 + ((codepoint & 0x0FC0) >> 6);
buffer[2] = 0x80 + ((codepoint & 0x003F));
*size = 3;
}
else if(codepoint <= 0x10FFFF)
{
buffer[0] = 0xF0 + ((codepoint & 0x1C0000) >> 18);
buffer[1] = 0x80 + ((codepoint & 0x03F000) >> 12);
buffer[2] = 0x80 + ((codepoint & 0x000FC0) >> 6);
buffer[3] = 0x80 + ((codepoint & 0x00003F));
*size = 4;
}
else
return -1;
return 0;
}
size_t utf8_check_first(char byte)
{
unsigned char u = (unsigned char)byte;
if(u < 0x80)
return 1;
if(0x80 <= u && u <= 0xBF) {
return 0;
}
else if(u == 0xC0 || u == 0xC1) {
return 0;
}
else if(0xC2 <= u && u <= 0xDF) {
return 2;
}
else if(0xE0 <= u && u <= 0xEF) {
return 3;
}
else if(0xF0 <= u && u <= 0xF4) {
return 4;
}
else { 
return 0;
}
}
size_t utf8_check_full(const char *buffer, size_t size, int32_t *codepoint)
{
size_t i;
int32_t value = 0;
unsigned char u = (unsigned char)buffer[0];
if(size == 2)
{
value = u & 0x1F;
}
else if(size == 3)
{
value = u & 0xF;
}
else if(size == 4)
{
value = u & 0x7;
}
else
return 0;
for(i = 1; i < size; i++)
{
u = (unsigned char)buffer[i];
if(u < 0x80 || u > 0xBF) {
return 0;
}
value = (value << 6) + (u & 0x3F);
}
if(value > 0x10FFFF) {
return 0;
}
else if(0xD800 <= value && value <= 0xDFFF) {
return 0;
}
else if((size == 2 && value < 0x80) ||
(size == 3 && value < 0x800) ||
(size == 4 && value < 0x10000)) {
return 0;
}
if(codepoint)
*codepoint = value;
return 1;
}
const char *utf8_iterate(const char *buffer, size_t bufsize, int32_t *codepoint)
{
size_t count;
int32_t value;
if(!bufsize)
return buffer;
count = utf8_check_first(buffer[0]);
if(count <= 0)
return NULL;
if(count == 1)
value = (unsigned char)buffer[0];
else
{
if(count > bufsize || !utf8_check_full(buffer, count, &value))
return NULL;
}
if(codepoint)
*codepoint = value;
return buffer + count;
}
int utf8_check_string(const char *string, size_t length)
{
size_t i;
for(i = 0; i < length; i++)
{
size_t count = utf8_check_first(string[i]);
if(count == 0)
return 0;
else if(count > 1)
{
if(count > length - i)
return 0;
if(!utf8_check_full(&string[i], count, NULL))
return 0;
i += count - 1;
}
}
return 1;
}
